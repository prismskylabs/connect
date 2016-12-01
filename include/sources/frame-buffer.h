#ifndef PRISM_CONNECT_SOURCES_FrameBuffer_H_
#define PRISM_CONNECT_SOURCES_FrameBuffer_H_

#include <condition_variable>
#include <deque>
#include <iterator>
#include <mutex>
#include <thread>

namespace prism {
namespace connect {
namespace sources {

template <typename T>
class FrameBuffer {
  public:
    FrameBuffer() : max_size_(2000) {};

    bool Full() {
        std::unique_lock<std::mutex> mlock(mutex_);
        bool is_full = deque_.size() >= max_size_;
        return is_full;
    }

    T Pop() {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (deque_.empty()) {
            condition_variable_.wait(mlock);
        }
        auto output = std::move(deque_.front());
        deque_.pop_front();
        return std::move(output);
    }

    void Push(T item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        deque_.push_back(std::move(item));
        condition_variable_.notify_one();
    }

    int Size() {
        std::unique_lock<std::mutex> mlock(mutex_);
        int size = deque_.size();
        return size;
    }

    void MoveInto(std::vector<T>& container) {
        std::unique_lock<std::mutex> mlock(mutex_);
        container.insert(container.end(), std::make_move_iterator(deque_.begin()),
                         std::make_move_iterator(deque_.end()));
        deque_.clear();
    }

  private:
    int max_size_;
    std::deque<T> deque_;
    std::mutex mutex_;
    std::condition_variable condition_variable_;
};

} // namespace sources
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_SOURCES_FrameBuffer_H_
