#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#define MAX_THREADS 8

namespace gear {
    class JobSystem {
    public:
        struct JobArgs {
        };

        typedef std::function<void(JobArgs)> JobFunc;

        JobSystem(uint32_t num_threads);

        ~JobSystem();

        void ExecuteJob(JobFunc func);

        bool IsJobSystemIdle();

        void WaitIdleSystemIdle();

    private:
        static void ThreadFunc(JobSystem*);

    private:
        volatile bool _run;
        uint32_t _num_threads;
        uint32_t _num_idle_threads;
        std::queue<JobFunc> _jobs;
        std::thread _threads[MAX_THREADS];
        std::mutex _queue_mutex;
        std::condition_variable _queue_cond;
        std::condition_variable _idle_cond;
    };
}