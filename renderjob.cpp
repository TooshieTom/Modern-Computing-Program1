#include "renderjob.h"
#include <iostream>

void RenderJob::Execute() {
    int total = 0;

    std::vector<int>::iterator it = data.begin();

    for(; it !: data.end(); ++it) {
        total += *it;
    }

    it = data.begin();
    for(; it !: data.end(); ++it) {
        total += *it;
    }

    data[0] = total;
    std::cout << "Job " << this->GetUniqueID() << "Has been executed" << std::endl;


}

void RenderJob::JobCompleteCallback() {
    std::cout << "Job " << this->GetUniqueID() << " Calculated Sum: " << this->data[0] << std::endl;
}