#include "BatchCmd.h"
#include <cstdio>
#include <iostream>
#include <random>
#include <chrono>
#include <sstream>

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

void BatchFileCmd::setCommand(const std::string& cmd)
{
    _command = cmd;
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

bool BatchFileCmd::execute(const std::string& tmpFilePrefix)
{
    std::string batFileName(BatchFileCmd::MakeRandHexStr(tmpFilePrefix) + ".bat");

    _batchFilePath = std::filesystem::current_path();
    _batchFilePath.append(batFileName);
    _batchFile.open(_batchFilePath, std::ios::trunc);

    for (auto& envArg : _envArgs)
    {
        _batchFile << "SET " << envArg.first << "=" << envArg.second << std::endl;
    }
    _batchFile << _command << std::endl;

    _pipeFile = _wpopen(_batchFilePath.c_str(), L"rt");
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

    _pollingCallback(pipeBuf);
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

    if (_batchFile.is_open() == true)
    {
        _batchFile.close();
        std::filesystem::remove(_batchFilePath);
    }

    return true;
}

int BatchFileCmd::endOfFile()
{
    return _endOfFile;
}

int BatchFileCmd::errorCode()
{
    return _errorCode;
}

int BatchFileCmd::returnCode()
{
    return _returnCode;
}