#ifndef PROTOTYPE_ACTION_RPG_THREADPOOL_HPP
#define PROTOTYPE_ACTION_RPG_THREADPOOL_HPP

#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>


namespace engine {

    class ThreadPool {
        using Task = std::function<void()>;

    public:
        ThreadPool();

        ~ThreadPool();

        void stop();

        void submit(Task f);

        bool empty();

        std::vector<std::thread>& getThreads();

    private:
        void waitTask();

    private:
        bool m_done;
        std::vector<std::thread> m_pool;
        std::queue<Task> m_tasks;
        std::mutex m_poolMutex;
        std::condition_variable m_condition;
    };

} // namespace engine


#endif //PROTOTYPE_ACTION_RPG_THREADPOOL_HPP
