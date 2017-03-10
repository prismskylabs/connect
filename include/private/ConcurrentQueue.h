/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_CONCURRENTQUEUE_H
#define PRISM_CONCURRENTQUEUE_H

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#include <queue>

namespace prism
{
namespace connect
{

template<typename T> class ConcurrentQueue : private boost::noncopyable
{
public:
    ConcurrentQueue(size_t maxSize)
        : maxSz_(maxSize) {}

    virtual ~ConcurrentQueue() {}

    bool tryPush(const T& val)
    {
        boost::mutex::scoped_lock sl(mtx_);
        if (q_.size() == maxSz_) return false;

        q_.push(val);
        sl.unlock();
        emptyCond_.notify_one();

        return true;
    }

    bool tryPop(T &val)
    {
        boost::mutex::scoped_lock sl(mtx_);
        if (q_.empty()) return false;

        val = q_.front();
        q_.pop();
        sl.unlock();
        fullCond_.notify_one();

        return true;
    }

    void push(const T& val)
    {
        boost::mutex::scoped_lock sl(mtx_);
        while (q_.size() == maxSz_) { fullCond_.wait(sl); }

        q_.push(val);
        sl.unlock();
        emptyCond_.notify_one();
    }

    void pop(T& val)
    {
        boost::mutex::scoped_lock sl(mtx_);
        while (q_.empty()) { emptyCond_.wait(sl); }

        val = q_.front();
        q_.pop();
        sl.unlock();
        fullCond_.notify_one();
    }

    bool empty()
    {
        boost::mutex::scoped_lock sl(mtx_);
        return q_.empty();
    }

private:
    size_t maxSz_;
    typename std::queue<T> q_;
    mutable boost::mutex mtx_;
    boost::condition fullCond_;
    boost::condition emptyCond_;
};

typedef ConcurrentQueue<void*> ThreadSafeQueue;

} // namespace connect
} // namespace prism

#endif



