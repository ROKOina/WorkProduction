#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include <functional>
#include <future>

class ThreadPool
{
    using ui32 = std::uint_fast32_t;
    using ui64 = std::uint_fast64_t;

public:
    ThreadPool(const ui32& thread_count = std::thread::hardware_concurrency())
        : thread_count_{ thread_count ? thread_count : std::thread::hardware_concurrency() } {
        threads.reset(new std::thread[thread_count_]);

        for (ui32 i = 0; i < thread_count_; ++i) {
            threads[i] = std::thread(&ThreadPool::worker, this);
        }
    }

    ~ThreadPool() {
        {
            // 新しいタスクを追加しないようにタスクキューをロックする。
            std::lock_guard<std::mutex> lock(tasks_mutex);
            running = false;
        }

        // すべて起床
        condition.notify_all();

        for (ui32 i = 0; i < thread_count_; ++i) {
            threads[i].join();
        }
    }

    ui32 thread_count() const {
        return thread_count_;
    }

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
    //タスク・キューに0個以上の引数と戻り値を持つ関数を投入する、そして、最終的な戻り値の future を取得します。
    template <typename F, typename... Args, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
#else
    template <typename F, typename... Args, typename R = typename std::result_of<std::decay_t<F>(std::decay_t<Args>...)>::type>
#endif
    std::future<R> submit(F&& func,Args&&... args) {
        auto task = std::make_shared<std::packaged_task<R()>>([func, args...]() {
            return func(args...);
            });
        auto future = task->get_future();

        push_task([task]() { (*task)(); });
        return future;
    }

private:

    template <typename F>
    void push_task(const F& task) {
        {
            const std::lock_guard<std::mutex> lock(tasks_mutex);

            if (!running) {
                throw std::runtime_error("Cannot schedule new task after shutdown.");
            }

            tasks.push(std::function<void()>(task));
        }

        condition.notify_one();
    }

    //プール内の各スレッドに割り当てられるワーカー関数。
    //アトミック変数runningがtrueに設定されている限り、タスクをキューから継続的に取り出して実行する。
    void worker() {
        for (;;) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(tasks_mutex);
                condition.wait(lock, [&] {return !tasks.empty() || !running; });

                if (!running && tasks.empty()) {
                    return;
                }

                task = std::move(tasks.front());
                tasks.pop();
            }

            task();
        }
    }

private:

    std::queue<std::function<void()>> tasks{}
    ;
    std::unique_ptr<std::thread[]> threads;
    std::condition_variable condition;

    mutable std::mutex tasks_mutex{};

    const ui32 thread_count_;
    std::atomic<bool> running{ true };
};