#include "BatchCmd.h"
#include <cstdio>
#include <iostream>
#include <random>
#include <chrono>
#include <sstream>
#include <fstream>

template<int64_t MinTy, int64_t MaxTy>
std::string BatchFileCmd::MakeRandHexStr(const std::string& prefix)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> dis(MinTy, MaxTy);
    int64_t randValue = dis(gen);

    std::stringstream stream;
    stream << prefix << std::hex << randValue;

    return stream.str();
}

BatchFileCmd::BatchFileCmd(const std::string& cmd, std::initializer_list<std::pair<std::string, std::string>> envArgs,
    const PollingCallbackFunc& callbackFunc)
{
    setCommand(cmd);

    for (auto&& arg : envArgs)
    {
        addEnvArgs(arg.first, arg.second);
    }

    setPollingCallback(callbackFunc);
}

BatchFileCmd::BatchFileCmd(const std::filesystem::path& batachFilePath, std::initializer_list<std::pair<std::string, std::string>> envArgs,
    const PollingCallbackFunc& callbackFunc)
{
    setCommandBatchFile(batachFilePath);

    for (auto&& arg : envArgs)
    {
        addEnvArgs(arg.first, arg.second);
    }

    setPollingCallback(callbackFunc);
}

void BatchFileCmd::setCommand(const std::string& cmd)
{
    _command = cmd;
}

void BatchFileCmd::setCommandBatchFile(const std::filesystem::path& batachFilePath)
{
    namespace fs = std::filesystem;
    std::string batchFileContents;
    std::ifstream batchFile(batachFilePath, std::ios::in);

    try
    {
        std::string line;
        while (batchFile && !batchFile.eof())
        {
            getline(batchFile, line);
            batchFileContents.append(line);
            batchFileContents.append("\r\n");
        }
    }
    catch (...)
    {
        batchFile.close();
        return;
    }
    batchFile.close();

    _command = batchFileContents;

}

bool BatchFileCmd::addEnvArgs(const std::string& name, const std::string& value)
{
    auto iter = _envArgs.emplace(name, value);
    if (iter.second == false)
    {
        return false;
    }
    return true;
}

void BatchFileCmd::setPollingCallback(const PollingCallbackFunc& func)
{
    _pollingCallback = func;
}

bool BatchFileCmd::execute(const std::string& workAbsPath, const std::string& tmpFilePrefix)
{
    if (_command.empty())
    {
        return false;
    }

    std::string batFileName(BatchFileCmd::MakeRandHexStr(tmpFilePrefix) + ".bat");

    std::filesystem::path workPath;
    if (workAbsPath.empty() == true)
    {
        workPath = std::filesystem::current_path().c_str();
    }
    else
    {
        workPath = workAbsPath;
    }
    _batchFilePath = workPath;
    _batchFilePath.append(batFileName);

    std::ofstream batchFile;
    batchFile.open(_batchFilePath, std::ios::trunc);
    if (workAbsPath.empty() == false)
    {
        batchFile << workPath.root_name().string() << std::endl;
        batchFile << "cd " << workPath << std::endl;
    }

    for (auto& envArg : _envArgs)
    {
        batchFile << "SET " << envArg.first << "=" << envArg.second << std::endl;
    }
    batchFile << _command << std::endl;
    batchFile.close();

    std::wstring redirectCommand = _batchFilePath;
    redirectCommand.append(L" 2>&1");
    _pipeFile = _wpopen(redirectCommand.c_str(), L"rt");
    if (_pipeFile == NULL)
    {
        terminate();
        return false;
    }
    return true;
}

bool BatchFileCmd::polling()
{
    char pipeBuf[PIPE_SIZE];

    if (fgets(pipeBuf, PIPE_SIZE, _pipeFile) == NULL)
    {
        return false;
    }

    if (_pollingCallback)
    {
        _pollingCallback(pipeBuf);
    }

    return true;
}

bool BatchFileCmd::terminate()
{
    if (_pipeFile != NULL)
    {
        _endOfFile = std::feof(_pipeFile);
        _errorCode = std::ferror(_pipeFile);
        _returnCode = _pclose(_pipeFile);
    }

    try
    {
        std::filesystem::remove(_batchFilePath);
        _removeFile = 1;
    }
    catch (...)
    {
        _removeFile = 0;
    }
    return true;
}