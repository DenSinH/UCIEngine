#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>


namespace util {

template<typename T>
class ThreadQueue {
    /*
     * Mostly to be used for command processor -> flipper interactions.
     * Idea is to only have one pushing thread and one pulling thread.
     * In that way, nothing can go wrong between checking whether the queue is empty, and popping an item.
     * */
public:
    explicit ThreadQueue() = default;

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& rel_time) {
        std::unique_lock<std::mutex> lock(mutex);
        bool result = cv.wait_for(lock, rel_time, [&]{ return !queue.empty(); });
        lock.unlock();
        return result;
    }

    std::size_t size() const {
        // warning: may be inaccurate
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

    T& front() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.front();
    }

    const T& front() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.front();
    }

    T& back() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.back();
    }

    const T& back() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.back();
    }

    void push(const T& value) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(value);
        }
        cv.notify_all();
    }

    void push(T&& value) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(value));
        }
        cv.notify_all();
    }

    void pop() {
        std::lock_guard<std::mutex> lock(mutex);
        queue.pop();
    }

    T pop_value() {
        std::lock_guard<std::mutex> lock(mutex);
        T value = std::move(queue.front());
        queue.pop();
        return std::move(value);
    }

private:
    mutable std::mutex mutex;
    std::queue<T> queue = {};
    std::condition_variable cv;
};

}