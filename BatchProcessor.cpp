#include "BatchProcessor.h"
#include <cstdio>
#include <iostream>
#include <random>
#include <chrono>
#include <sstream>
#include <fstream>

template<int64_t MinTy, int64_t MaxTy>
std::string BatchProcessor::MakeRandHexStr(std::string_view prefix)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> dis(MinTy, MaxTy);
    int64_t randValue = dis(gen);

    std::stringstream stream;
    stream << prefix << std::hex << randValue;

    return stream.str();
}

BatchProcessor::BatchProcessor()
{
}

BatchProcessor::~BatchProcessor()
{
    terminate();
}

BatchProcessor::BatchProcessor(std::string_view batchCmd, std::initializer_list<EnvArgView> envArglist)
{
    setCommand(batchCmd);
    setEnvArgs(envArglist);
}

BatchProcessor::BatchProcessor(const std::filesystem::path& batchSourcePath, std::initializer_list<EnvArgView> envArglist)
{
    setCommandSourceFile(batchSourcePath);
    setEnvArgs(envArglist);
}

void BatchProcessor::setCommand(std::string_view batchCmd)
{
    _batchCommand = batchCmd;
}

void BatchProcessor::setCommand(std::string&& batchCmd)
{
    _batchCommand = std::move(batchCmd);
}

void BatchProcessor::setCommandSourceFile(const std::filesystem::path& batchSourcePath)
{
    std::string batchCmd;
    std::ifstream sourceFile(batchSourcePath, std::ios::in);

    try
    {
        std::string line;
        while (sourceFile && !sourceFile.eof())
        {
            getline(sourceFile, line);
            batchCmd.append(line);
            batchCmd.append("\r\n");
        }
    }
    catch (...)
    {
        sourceFile.close();
        return;
    }
    sourceFile.close();

    _batchSourcePath = batchSourcePath;
    setCommand(std::move(batchCmd));
}

void BatchProcessor::setEnvArgs(std::initializer_list<EnvArgView> envArglist)
{
    _envArgs.insert(envArglist.begin(), envArglist.end());
}

bool BatchProcessor::execute(std::string_view workAbsPath, std::string_view genBatchPrefix)
{
    if (isExecuted() == true)
    {
        return false;
    }
    else if (_batchCommand.empty() == true)
    {
        return false;
    }

    std::string batFileName(BatchProcessor::MakeRandHexStr(genBatchPrefix) + ".bat");

    std::filesystem::path workPath;
    if (workAbsPath.empty() == true)
    {
        workPath = std::filesystem::current_path().c_str();
    }
    else
    {
        workPath = workAbsPath;
    }
    _batchGeneratedPath = workPath;
    _batchGeneratedPath.append(batFileName);

    std::ofstream batchFile;
    batchFile.open(_batchGeneratedPath, std::ios::trunc);
    if (workAbsPath.empty() == false)
    {
        batchFile << workPath.root_name().string() << std::endl;
        batchFile << "cd " << workPath << std::endl;
    }

    for (auto& envArg : _envArgs)
    {
        batchFile << "SET " << envArg.first << "=" << envArg.second << std::endl;
    }
    batchFile << _batchCommand << std::endl;
    batchFile.close();

    std::wstring redirectCommand = _batchGeneratedPath;
    redirectCommand.append(L" 2>&1");
    _pipeFile = _wpopen(redirectCommand.c_str(), L"rt");
    _isExecuted = true;

    if (_pipeFile == NULL)
    {
        terminate();
        return false;
    }
    return true;
}

bool BatchProcessor::polling(char pipeBuf[BatchProcessor::PIPE_SIZE])
{
    if (isExecuted() == false || isTerminated() == true)
    {
        return false;
    }

    if (fgets(pipeBuf, PIPE_SIZE, _pipeFile) == NULL)
    {
        return false;
    }

    return true;
}

bool BatchProcessor::terminate()
{
    if (isExecuted() == false || isTerminated() == true)
    {
        return false;
    }

    if (_pipeFile != NULL)
    {
        _execResult.endOfFile = std::feof(_pipeFile);
        _execResult.errorCode = std::ferror(_pipeFile);
        _execResult.returnCode = _pclose(_pipeFile);
        _pipeFile = NULL;
    }

    try
    {
        std::filesystem::remove(_batchGeneratedPath);
        _execResult.removeFile = 1;
    }
    catch (...)
    {
        _execResult.removeFile = 0;
    }

    _isTerminated = true;
    return true;
}