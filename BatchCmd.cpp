#include "BatchCmd.h"
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

BatchProcessor::BatchProcessor(std::string_view batchCmd, std::initializer_list<EnvArgView> envArglist)
{
    setCommand(batchCmd);

    for (auto&& arg : envArglist)
    {
        addEnvArgs(arg);
    }
}

BatchProcessor::BatchProcessor(const std::filesystem::path& batchCmdSourceFile, std::initializer_list<EnvArgView> envArglist)
{
    setCommandSourceFile(batchCmdSourceFile);

    for (auto&& arg : envArglist)
    {
        addEnvArgs(arg);
    }
}

void BatchProcessor::setCommand(std::string_view batchCmd)
{
    _batchCommand = batchCmd;
}

void BatchProcessor::setCommandSourceFile(const std::filesystem::path& batchCmdSourceFile)
{
    namespace fs = std::filesystem;
    std::string batchCmd;
    std::ifstream sourceFile(batchCmdSourceFile, std::ios::in);

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

    setCommand(batchCmd);
}

bool BatchProcessor::addEnvArgs(const EnvArgView& evnArg)
{
    auto iter = _envArgs.emplace(evnArg);
    if (iter.second == false)
    {
        return false;
    }
    return true;
}

bool BatchProcessor::execute(std::string_view workAbsPath, std::string_view genBatchPrefix)
{
    if (_batchCommand.empty())
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
    _generatedBatchFilePath = workPath;
    _generatedBatchFilePath.append(batFileName);

    std::ofstream batchFile;
    batchFile.open(_generatedBatchFilePath, std::ios::trunc);
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

    std::wstring redirectCommand = _generatedBatchFilePath;
    redirectCommand.append(L" 2>&1");
    _pipeFile = _wpopen(redirectCommand.c_str(), L"rt");
    if (_pipeFile == NULL)
    {
        terminate();
        return false;
    }
    return true;
}

bool BatchProcessor::polling(char pipeBuf[BatchProcessor::PIPE_SIZE])
{
    if (fgets(pipeBuf, PIPE_SIZE, _pipeFile) == NULL)
    {
        return false;
    }

    return true;
}

bool BatchProcessor::terminate()
{
    if (_pipeFile != NULL)
    {
        _endOfFile = std::feof(_pipeFile);
        _errorCode = std::ferror(_pipeFile);
        _returnCode = _pclose(_pipeFile);
    }

    try
    {
        std::filesystem::remove(_generatedBatchFilePath);
        _removeFile = 1;
    }
    catch (...)
    {
        _removeFile = 0;
    }

    return true;
}