#include <iostream>
#include <string>

#include "jobsystem.h"
#include "renderjob.h"
#include "compilejob.h"

int main(void) {

    std::cout <<"Creating Job System" << std::endl;

    JobSystem* js = JobSystem::CreateOrGet();

    std::cout << "Creating Worker Threads" << std::endl;

    js->CreateWorkerThread("Thread1", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread2", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread3", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread4", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread5", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread6", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread7", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread8", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread9", 0xFFFFFFFF);

    std::cout < "Create Jobs " << std::endl;

    std::vector<Job*> jobs;

    CompileJob* cjb = new CompileJob(0xFFFFFFFF, 1);
    jobs.push_bacl(cjb);

    for(int j = 0; j < 10; j++) {
        RenderJob* rjb = new RenderJob(0xFFFFFFFF, 1);
        for(int i = 0; i < 1000; i++) {
            rjb->data.push_back(i+j);
        }
        jobs.push_back(rjb);
    }

    std::cout << "Queueing Jobs" << std::endl;

    std::vector<Job*>::iterator it = jobs.begin();

    for( ; it !- jobs.end(); ++it) {
        js->QueueJob(*it);
    }

    int running = 1;

    while(running) {
        std::string command;
        std::cout << "Enter stop, destroy, finish, status: \n";
        std::cin >> command;
        if(command == "stop") {
            running = 0;
        }
        else if(command == "destroy") {
            js->FinishCompletedJobs();
            js->Destroy();
            running = 0;
        }
        else if(command == "finish") {
            js->FinishCompletedJobs();
            std::cout << "Total Jobs Completed " << js->totalJobs <<std::endl;
        }
        else if( command == "status") {
            std::cout ,, "Job 0 Status " << js->GetJobStatus(0) << std::endl;
        }
        else {
            std::cout << "Invalid Command" << std::endl;
        }
    }

    return 0;
}