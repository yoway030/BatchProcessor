#include "BatchCmd.h"
#include <iostream>

int main(void)
{
    // execute sample batch file
    std::string cmd = "test.bat";
    // enable this
    //std::string cmd = "dir | find batch";


    BatchFileCmd fileCmd;
    
    // set command
    fileCmd.setCommand(cmd);
    
    // set enviroment variable
    fileCmd.addEnvArgs("outver", "123");

    // set callback
    fileCmd.setPollingCallback([](const std::string& str) -> int
        {
            std::cout << str;
            return 0;
        }
    );

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