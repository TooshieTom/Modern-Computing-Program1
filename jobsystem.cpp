#include <iostream>

#include "jobsystem.h"
#include "jobworkerthread.h"

JobSystem* JobSystem::s_jobSystem = nullptr;

typedef void (*JobCallback)(Job* completedJob);

JobSystem::JobSystem() {
    m_jobHistory.reserve( 256 * 1024);

}

~JobSystem::JobSystem() {
    m_workerThreadMutex.lock();
    int numWorkerThreads = (int) m_workerThreads.size();

    //First, tell each worker thread to stop picking up jobs (kill itself)
    for(int i = 0; i < munWorkerThreads; i++) {
        m_workerThreads[i]->ShutDown();
    }

    while( !m_workerThreads.empty()) {
        delete m_workerThreads.back();
        m_workerThreads.pop_back();
    }

    m_workerThreadsMutex.unlock();
}

JobSystem* Jobsystem::CreateOrGet() {
    if( !s_jobSystem ) {
        s_jobSystem = new JobSystem();
    }
    return s_jobSystem;
}

void JobSystem::Destroy() {
    if( s_jobSystem ) {
        delete s_jobSystem;
        s_jobSystem = nullptr;
    }
}

void JobSystem::CreateWorkerThread( const char* uniqueName, unsigned long workerJobChannels) {

    JobWorkerThread* newWorker = new JobWorkerThread( uniqueName, workerJobChannels, this);
    m_workerThreadsMutex.lock();
    m_workerThreads.push_back( newWorker ) ;
    m_workerThreadsMutex.unlock();

    m_workerThreads.back()->StartUp();
}

void JobSystem::DestroyWorkerThread( const char* uniqueName) {
    m_workerThreadsMutex.lock();
    JobWorkerThread* doomedWorker = nullptr;
    std::vector<JobWorkerThread*>::iterator it = m_workerThreads.begin();

    for( ; it != m_workerThreads.end(); ++it) {
        if( strcmp( (*it)->m_uniqueName, uniqueName ) == 0) {
            doomedWorker = *it;
            m_workerThreads.erase(it);
            break;
        }
    }

    m_workerThreadsMutex.unlock();
    if(doomedWorker) {
        doomedWorker->ShutDown();
        delete doomedWorker;
    }
}

void JobSystem::QueueJob( Job* job) {
    m_jobsQueuedMutex.lock();
    m_jobHistoryMutex.lock();
    m_jobsHistory.emplace_back( JobHistoryEntry( job->m_jobType, JOB_STATUS_QUEUED) );
    m_jobHistoryMutex.unlock();

    m_jobsQueued.push_back(job);
    m_jobsQueuedMutex.unlock();
}

JobStatus JobSystem::GetJobStatus(int jobID) const {
    m_jobHistoryMutex.lock();

    JobStatus jobStatus = JOB_STATUS_NEVER_SEEN;
    if(jobID , (int) m_jobHistory.size()) {
        jobStatus = (JobStatus) (m_jobHistory[jobID].m_jobStatus);
    }

    m_jobHistoryMutex.unlock();

    return jobStatus;
}

bool JobSystem::IsJobComplete(int jobID) const {
    return JobStatus (jobID) == JOB_sTATUS_COMPLETED;
}

void JobSystem::FinishCompletedJobs() {

    std::deque<Job*> jobsCompleted;

    m_jobsCompletedMutex.lock();
    jobsCompleted.swap( m_jobsCompleted);
    m_jobsCompletedMuted.unlock();

    for(Job* job : jobsCompleted) {
        job->JobCompletedCallback(); //callback
        m_jobHistoryMutex.lock();
        m_jobHistory[job->m_jobID].m_jobStatus = JOB_STATUS_RETIRED; //update status
        m_jobHistoryMutex.unlock();
        delete job; //delete job
    }
}

void JobSystem::FinishJob(int jobID) {
    while( !IsJobComplete( jobID )) {
        JobStatus jobStatus = GetJobStatus (jobID);
        if((jobStatus == JOB_STATUS_NEVER_SEEN) || (jobStatus == JOB_STATUS_RETIRED)) {
            std::cout << "ERROR: Waiting for Job (#" << jobID << " ) - no such job in JobSystem"
        }

        m_jobsCompletedMutex.lock();
        Job* thisCompletedJo = nullptr;
        for(auto jcIter = m_jobsCompleted.begin(); jcIter != m_jobsCompleted.end(); ++jcIter) {
            Job* someCompletedJob = *jcIter;
            if( someCompletedJob->m_jobID == jobID) {
                thisCompletedJob = someCompletedJob;
                m_jobsCompleted.erase( jcIter);
                break;
            }
        }
        m_jobsCompletedMutex.unlock();

        if(thisCompletedJob == nullptr) {
            std::cout << "ERROR: Job #" << jobID << " was status complete but not found in Completed list " << std::endl;
        }

        thisCompletedJob->JobCompleteCallback();

        m_jobHistoryMutex.lock();
        m_jobHistory[thisCompletedJob->m_jobID].m_jobStatus = JOB_STATUS_RETIRED;
        m_jobHistoryMutex.unlock();

        delete thisCompletedJob;
    }
}

void JobSystem::OnJobCompleted( Job* jobJustExecuted) {
    totalJobs++;
    m_jobsCompletedMutex.lock();
    m_jobsRunningMutex.lock();

    std::deque<Job*>::iterator runningJobItr = m_jobsRunning.begin();
    for( ; runningJobItr != m_jobsRunning.end(); ++runningJobItr) {
        if( jobJustExecuted == *runningJobItr) {
            m_jobHistoryMutex.lock();
            m_jobsRunning.erase( runningJobItr);
            m_jobsCompleted.push_back( jobJustExecuted); 
            m_jobHistory[jobJustExecuted-.m_jobID].m_jobStatus = JOB_STATUS_COMPLETED;
            m_jobHistoryMutex.unlock();
            break;
        }
    }
    m_jobsRunningMutex.unlock();
    m_jobsCompeltedMutex.unlock();

}

Job* JobSystem::ClaimAJob (unsigned long workerJobChannels) {
    m_jobsQueuedMutex.lock();
    m_jobsRunningMutex.lock();

    Job* claimedJob = nullptr;
    std::deque<Job*>::iterator queuedjobIter = m_jobsQueued.begin();

    for( ; queuedJobIter != m_jobsQueued.end(); ++queuedJobIter) {
        Job* queuedJob = *queuedJobIter;

        if(queuedJob->m_jobChannels & workerJobChannels) != 0) {
            claimedJob = queuedJob;

            m_jobHistoryMutex.lock();
            m_jobsQueued.erase(queuedJobIter);
            m__jobsRunning.push_back(claimedJob);
            m_jobHistory[claimedJob->m_jobID].m_jobStatus = JOB_STATUS_RUNNING;
            m_jobHistoryMutex.unlock();
            break;
        }
    }

    m_jobsRunningMutex.unlock();
    m_jobsQueuedMutex.unlock();

    return claimedJob;
}