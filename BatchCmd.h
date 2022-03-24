#pragma once

#include <map>
#include <string>
#include <functional>
#include <fstream>
#include <filesystem>
#include <initializer_list>

class BatchFileCmd
{
public:
    static constexpr int PIPE_SIZE = 1024;
    using PollingCallbackFunc = std::function<int(const std::string&)>;

    template<int64_t MinTy = 0x100000, int64_t MaxTy = 0xFFFFFF>
    static std::string MakeRandHexStr(const std::string& prefix);

public:
    BatchFileCmd(const std::string& cmd, std::initializer_list<std::pair<std::string, std::string>> envArgs, 
        const PollingCallbackFunc& callbackFunc);
    ~BatchFileCmd() = default;
    
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
    std::string _command{};
    std::map<std::string /* name */, std::string /* value */> _envArgs{};
    PollingCallbackFunc _pollingCallback{};

    std::ofstream _batchFile{};
    std::filesystem::path _batchFilePath{};
    FILE* _pipeFile{nullptr};
    int _errorCode{-1};
    int _endOfFile{-1};
    int _returnCode{-1};
};

