#include "compilejob.h"
#include <iostream>
#include <string>
#include <array>

void CompileJob::Execute() {
    std::array<char, 128 > buffer;
    std::string command = "make automated";

    //Redirect cerr to cout
    command.append(" 2>&1"); 

    //open pipe and run command
    FILE* pipe = popen(command.c_str(), "r");

    if(!pipe) {
        std::cout << "popen Failed: failed to open pipe" << std::endl;
        return;
    }

    //read until end of process:
    while(fgets(buffer.data(), 128, pipe) != NULL) {
        this->output.append(buffer.data());
    }

    //close pipe and get return code
    this->returnCode = pclose(pipe);

    std::cout << "Job " << this->GetUniqueID() << " Has been executed" << std::endl;
}

void CompileJob::JobCompletedCallback() {
    std::cout << "Compile Job " << this->GetUniqueID() << " Return Code" << this->returnCode << std::endl;
    std::cout << "Compile Job " << this->GetUniqueID() << " Output \n" << this->output << std::endl;
}