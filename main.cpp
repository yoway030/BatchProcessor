#include "BatchProcessor.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

int main(void)
{
    // execute sample batch file
    std::string cmd = "test.bat";   // call this batch
    
    // enable this
    //std::string cmd = "dir | find batch";
    //std::filesystem::path cmd = "test.bat";   // copy this batch with env args

    BatchProcessor batchProcessor({ cmd.data(), cmd.length() }, { {"outver", "123"} });
    
    // write batch file and execute
    if (batchProcessor.execute() == true)
    {
        char pipeBuf[BatchProcessor::PIPE_SIZE];
        auto start = std::chrono::steady_clock::now();

        while (batchProcessor.polling(pipeBuf) == true)
        {
            std::cout << pipeBuf;

            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            if (delta > std::chrono::milliseconds(5000))
            {
                break;
            }
        }

        std::cout << "do something" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(4000));

        while (batchProcessor.polling(pipeBuf) == true)
        {
            std::cout << pipeBuf;
        }

        // need terminate for delete temporory batch file and take return code.
        batchProcessor.terminate();

        BatchProcessor::ExecuteResult execResult = batchProcessor.getExecResult();
        std::cout << "error : " << execResult.errorCode << std::endl;
        std::cout << "close : " << execResult.endOfFile << std::endl;
        std::cout << "return : " << execResult.returnCode << std::endl;
        std::cout << "remove : " << execResult.removeFile << std::endl;
    }
}