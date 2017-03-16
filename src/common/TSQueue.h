#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>


// An implementation of a thread safe queue based on Anthony Williams
// book, C++ Concurrency In Action.
// Annotation is my own.
template<typename T>
class TSQueue
{
public:
    TSQueue()
    {
    }

    // In push, first notice how we use std::lock_guard when locking the 
    // mutex. The mutex is guaranteed to be released when the method exits.
    // Second, note the use of std::move. This effectively casts the object
    // new_value to type T&& allowing it be "moved" to the called function.
    // After this call, the value of new_value is undetermined.
    // Finally, notice the use of notify_one which will notify the blocking
    // thread to wake up.
    //void push(T new_value)
    //{
    //    std::lock_guard<std::mutex> lk(m_mut);
    //    m_queue.push(std::move(new_value));
    //    m_cond.notify_one();
    //}

    void push(T& new_value)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        m_queue.push(std::move(new_value));
        m_cond.notify_one();
    }

    // In wait_and_pop, first notice the use of a unique_lock. This 
    // allows the mutex to be released when the condition_variables
    // wait method is called and reacquired when wait is woken.
    // Second, notice the use of wait and the simple lambda function
    // that checks if the queue is empty. The version of wait used takes
    // a unique lock and an expression of type Predicate (which a lambda
    // function is). The object is passed via the capture list to the
    // lambda function, allow it to inspect the queue and return 
    // a bool indicating the queue is empty. If the queue is not empty,
    // the mutex is reacquired.
    // Finally, notice that the front of the queue is "moved" to the 
    // parameter value which was passed by reference so the front of the
    // queue is returned.
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(m_mut);
        m_cond.wait(lk,[this]{return !m_queue.empty();});
        value=std::move(m_queue.front());
        m_queue.pop();
    }

    // Similar to the previous version of wait_and_pop except a shared pointer
    // is returned.
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(m_mut);
        m_cond.wait(lk,[this]{return !m_queue.empty();});
        std::shared_ptr<T> res(
            std::make_shared<T>(std::move(m_queue.front())));
        m_queue.pop();
        return res;
    }


    // In try_pop, we simply check if the queue is empty and if not, 
    // we "move" the first item in the queue to the value parameter passed
    // by reference.
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_queue.empty())
            return false;
        value=std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    //Similar to the previous version of try_pop except a shared pointer is
    // returned
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(m_mut);
        if (m_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));

        m_queue.pop();
        return res;
    }

    // In empty, we simply indicate if the function is empty.
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(m_mut);
        return m_queue.empty();
    }
        
private:
    mutable std::mutex m_mut;
    std::queue<T> m_queue;
    std::condition_variable m_cond;
};
#endif
