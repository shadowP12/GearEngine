#include "JobSystem.h"

namespace gear {
    void JobSystem::ThreadFunc(JobSystem* job_system) {
        while (job_system->_run) {
            std::unique_lock<std::mutex> munique(job_system->_queue_mutex, std::defer_lock);
            munique.lock();

            job_system->_num_idle_threads = job_system->_num_idle_threads + 1;

            // 处理虚假唤醒
            while (job_system->_run && job_system->_jobs.empty()) {
                // 通知主线程有线程处于空闲状态
                job_system->_idle_cond.notify_all();
                // 当前工作继续等待
                job_system->_queue_cond.wait(munique);
            }

            job_system->_num_idle_threads = job_system->_num_idle_threads - 1;

            if (!job_system->_jobs.empty()) {
                // 从任务队列中获取任务
                JobFunc job = job_system->_jobs.front();
                job_system->_jobs.pop();
                munique.unlock();
                JobArgs args;
                job(args);
            }
            else {
                munique.unlock();
            }
        }
        // 当该工作线程结束时，需要通知主线程有线程处于空闲状态
        std::unique_lock<std::mutex> munique(job_system->_queue_mutex, std::defer_lock);
        munique.lock();
        job_system->_num_idle_threads = job_system->_num_idle_threads + 1;
        job_system->_idle_cond.notify_all();
        munique.unlock();
    }

    JobSystem::JobSystem(uint32_t num_threads) {
        _run = true;
        _num_threads = std::max<uint32_t>(std::thread::hardware_concurrency() - 1, 1);
        _num_threads = std::min<uint32_t>(num_threads, std::min<uint32_t>(_num_threads, MAX_THREADS));
        _num_idle_threads = 0;

        for (uint32_t i = 0; i < _num_threads; ++i) {
            _threads[i] = std::thread(ThreadFunc, this);
        }
    }

    JobSystem::~JobSystem() {
        // 通知所有的工作线程JobSystem已经停止运行
        std::unique_lock<std::mutex> munique(_queue_mutex, std::defer_lock);
        munique.lock();
        _run = false;
        munique.unlock();
        _queue_cond.notify_all();

        // 等待所有工作线程结束
        for (uint32_t i = 0; i < _num_threads; ++i) {
            _threads[i].join();
        }
    }

    void JobSystem::ExecuteJob(JobFunc func) {
        std::unique_lock<std::mutex> munique(_queue_mutex, std::defer_lock);
        munique.lock();
        _jobs.push(func);
        munique.unlock();
        _queue_cond.notify_all();
    }

    bool JobSystem::IsJobSystemIdle() {
        std::unique_lock<std::mutex> munique(_queue_mutex, std::defer_lock);
        munique.lock();
        bool idle = (_jobs.empty() && _num_idle_threads == _num_threads) || !_run;
        munique.unlock();
        return idle;
    }

    void JobSystem::WaitIdleSystemIdle() {
        std::unique_lock<std::mutex> munique(_queue_mutex, std::defer_lock);
        munique.lock();
        if ((!_jobs.empty() || _num_idle_threads != _num_threads) && _run) {
            _idle_cond.wait(munique);
        }
        munique.unlock();
    }
}