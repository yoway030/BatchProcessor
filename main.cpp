#include "BatchCmd.h"
#include <iostream>
#include <filesystem>
#include <chrono>

int main(void)
{
    // execute sample batch file
    std::string cmd = "test.bat";   // call this batch
    
    // enable this
    //std::string cmd = "dir | find batch";
    //std::filesystem::path cmd = "test.bat";   // copy this batch with env args


    BatchProcessor fileCmd({ cmd.data(), cmd.length() }, { {"outver", "123"} });
    
    // write batch file and execute
    if (fileCmd.execute() == true)
    {
        char pipeBuf[BatchProcessor::PIPE_SIZE];
        auto start = std::chrono::steady_clock::now();

        while (fileCmd.polling(pipeBuf) == true)
        {
            std::cout << pipeBuf;

            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            if (delta > std::chrono::milliseconds(5000))
            {
                break;
            }
        }

        std::cout << "do something" << std::endl;

        while (fileCmd.polling(pipeBuf) == true)
        {
            std::cout << pipeBuf;
        }

        // need terminate for delete temporory batch file and take return code.
        fileCmd.terminate();

        std::cout << "error : " << fileCmd.errorCode() << std::endl;
        std::cout << "close : " << fileCmd.endOfFile() << std::endl;
        std::cout << "return : " << fileCmd.returnCode() << std::endl;
        std::cout << "remove : " << fileCmd.removeFile() << std::endl;
    }
}