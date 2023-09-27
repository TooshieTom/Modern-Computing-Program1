#include <vector>
#include "job.h"

class RenderJob : public Job {
    public:
        RenderJob();
        ~RenderJob(){};

        std::vector<int> data;      
};
