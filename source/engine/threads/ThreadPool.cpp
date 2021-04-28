#include "ThreadPool.hpp"


namespace engine {


    ThreadPool::ThreadPool() : m_done(false) {
        const unsigned threadCount = std::thread::hardware_concurrency() - 1;

        for (int i = 0; i < threadCount; ++i) {
            try {
                m_pool.emplace_back(&ThreadPool::waitTask, this);
            } catch (...) {
                m_done = true;
                throw ;
            }
        }
    }

    ThreadPool::~ThreadPool() = default;;

    void ThreadPool::stop() {
        {
            std::unique_lock<std::mutex> lock(m_poolMutex);
            m_done = true;
        }
        m_condition.notify_all();

        for (auto& thread : m_pool)
            thread.join();

        m_pool.clear();
    }

    void ThreadPool::waitTask() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(m_poolMutex);
                m_condition.wait(lock, [=]{return m_done || !m_tasks.empty();});

                if (m_done && m_tasks.empty()) break;

                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
            task();
        }
    }

    bool ThreadPool::empty() {
        return m_tasks.empty();
    }

    void ThreadPool::submit(Task f) {
        {
            std::unique_lock<std::mutex> lock(m_poolMutex);
            m_tasks.push(std::move(f));
        }
        m_condition.notify_one();
    }

    std::vector<std::thread> &ThreadPool::getThreads() {
        return m_pool;
    }

} // namespace engine