#pragma once

#include <map>
#include <string>
#include <functional>
#include <fstream>
#include <filesystem>

class BatchFileCmd
{
public:
    static constexpr int PIPE_SIZE = 1024;
    using PollingCallbackFunc = std::function<int(const std::string&)>;

public:
    
    void setCommand(const std::string& cmd);
    bool addEnvArgs(const std::string& name, const std::string& value);
    void setPollingCallback(const PollingCallbackFunc& func);

    bool execute(const std::string& tmpFilePrefix = "");
    bool polling();
    bool terminate();
    
    // terminate 이후 값 확인 가능
    int endOfFile();
    int errorCode();
    int returnCode();   // 일반적으로 0이 정상종료

private:
    std::string _command;
    std::map<std::string /* name */, std::string /* value */> _envArgs;
    PollingCallbackFunc _pollingCallback;

    std::ofstream _batchFile;
    std::filesystem::path _batchFilePath;
    FILE* _pipeFile{nullptr};
    int _errorCode{-1};
    int _endOfFile{-1};
    int _returnCode{-1};
};

