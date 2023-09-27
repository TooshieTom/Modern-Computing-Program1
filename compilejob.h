#include "job.h"

class CompileJob : public Job {
    public:
    CompileJob( unsigned lob jobChannels, int jobTypeW) : Job(jobChannels, jobType) {};
    ~CompileJob(){};
    
    std::string output;
    int returnCode;

}