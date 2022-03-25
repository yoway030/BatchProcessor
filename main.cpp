#include "BatchCmd.h"
#include <iostream>
#include <filesystem>

int main(void)
{
    // execute sample batch file
    std::string cmd = "test.bat";   // call this batch
    
    // enable this
    //std::string cmd = "dir | find batch";
    //std::filesystem::path cmd = "test.bat";   // copy this batch with env args


    BatchFileCmd fileCmd(cmd, { {"outver", "123"} }, 
        [](const std::string& str) -> int
        {
            std::cout << str;
            return 0;
        });
    
    // write batch file and execute
    if (fileCmd.execute() == true)
    {
        while (fileCmd.polling() == true)
        {
        }

        // need terminate for delete temporory batch file and take return code.
        fileCmd.terminate();

        std::cout << "error : " << fileCmd.errorCode() << std::endl;
        std::cout << "close : " << fileCmd.endOfFile() << std::endl;
        std::cout << "return : " << fileCmd.returnCode() << std::endl;
    }
}