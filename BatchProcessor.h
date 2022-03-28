#pragma once

#include <map>
#include <string>
#include <filesystem>
#include <initializer_list>

class BatchProcessor
{
public:
    static constexpr int PIPE_SIZE = 1024;

    using EnvArgView = std::pair<std::string_view, std::string_view>;
    using EvnArgMap = std::map<std::string /* name */, std::string /* value */>;

    struct ExecuteResult
    {
        int errorCode{ -1 };
        int endOfFile{ -1 };
        int returnCode{ -1 };
        int removeFile{ -1 };
    };

    template<int64_t MinTy = 0x100000, int64_t MaxTy = 0xFFFFFF>
    static std::string MakeRandHexStr(std::string_view prefix);

public:
    BatchProcessor(std::string_view batchCmd, std::initializer_list<EnvArgView> envArglist);
    BatchProcessor(const std::filesystem::path& batchSourceFile, std::initializer_list<EnvArgView> envArglist);
    ~BatchProcessor() = default;

    bool execute(std::string_view workAbsPath = "", std::string_view genBatchPrefix = "TMP_");
    bool polling(char pipeBuf[PIPE_SIZE]);
    bool terminate();

    const std::string& getBatchCommand() const { return _batchCommand; }
    const std::filesystem::path& getBatchSourcePath() const { return _batchSourcePath; }
    const EvnArgMap& getEnvArgs() const { return _envArgs; }

    bool isExecuted() const { return _isExecuted; }
    bool isTerminated() const { return _isTerminated; }

    const std::filesystem::path& getBatchGeneratedPath() const { return _batchGeneratedPath; }
    const ExecuteResult& getExecResult() const { return _execResult; }

private:
    void setCommand(std::string_view batchCmd);
    void setCommand(std::string&& batchCmd);
    void setCommandSourceFile(const std::filesystem::path& _batchSourcePath);
    void setEnvArgs(std::initializer_list<EnvArgView> envArglist);

private:
    std::string _batchCommand{};
    std::filesystem::path _batchSourcePath{};
    EvnArgMap _envArgs{};

    bool _isExecuted{ false };
    bool _isTerminated{ false };

    std::filesystem::path _batchGeneratedPath{};
    FILE* _pipeFile{ nullptr };
    ExecuteResult _execResult;
};