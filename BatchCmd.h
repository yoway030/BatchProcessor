#pragma once

#include <map>
#include <string>
#include <functional>
#include <fstream>
#include <filesystem>
#include <initializer_list>

class BatchProcessor
{
public:
    static constexpr int PIPE_SIZE = 1024;
    using EnvArgView = std::pair<std::string_view, std::string_view>;

    template<int64_t MinTy = 0x100000, int64_t MaxTy = 0xFFFFFF>
    static std::string MakeRandHexStr(std::string_view prefix);

public:
    BatchProcessor(std::string_view batchCmd, std::initializer_list<EnvArgView> envArglist);
    BatchProcessor(const std::filesystem::path& batchCmdSourceFile, std::initializer_list<EnvArgView> envArglist);
    ~BatchProcessor() = default;

    void setCommand(std::string_view batchCmd);
    void setCommandSourceFile(const std::filesystem::path& batchCmdSourceFile);
    bool addEnvArgs(const EnvArgView& arg);
    
    bool execute(std::string_view workAbsPath = "", std::string_view genBatchPrefix = "TMP_");
    bool polling(char pipeBuf[PIPE_SIZE]);
    bool terminate();

    const std::filesystem::path& generatedBatchFilePath() const { return _generatedBatchFilePath; }
    int endOfFile() const { return _errorCode; }
    int errorCode() const { return _endOfFile; }
    int returnCode() const { return _returnCode; }
    int removeFile() const { return _removeFile; }

private:
    std::string _batchCommand{};
    std::filesystem::path _generatedBatchFilePath{};    
    std::map<std::string /* name */, std::string /* value */> _envArgs{};
    
    FILE* _pipeFile{ nullptr };
    int _errorCode{ -1 };
    int _endOfFile{ -1 };
    int _returnCode{ -1 };
    int _removeFile{ -1 };
};