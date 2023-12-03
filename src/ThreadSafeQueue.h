#ifndef CALLCENTRE_THREADSAFEQUEUE_H
#define CALLCENTRE_THREADSAFEQUEUE_H


#include <mutex>
#include <deque>
#include <condition_variable>


namespace call_c {

    template<typename T>
    class tsqueue {
    public:
        explicit tsqueue(size_t n) : capacity(n) {

        };

        tsqueue(const tsqueue<T> &) = delete; // non copied

    public:
        const T &front() {
            std::scoped_lock lock(muxQueue);
            if (deqQueue.empty())
                throw std::out_of_range("tsqueue is empty, front() is forbidden");
            return deqQueue.front();
        }

        const T &back() {
            std::scoped_lock lock(muxQueue);
            if (deqQueue.empty())
                throw std::out_of_range("tsqueue is empty, back() is forbidden");
            return deqQueue.back();
        }

        bool push_back(const T &item) {
            std::scoped_lock lock(muxQueue);

            if (deqQueue.size() >= capacity)
                return false;

            deqQueue.emplace_back(std::move(item));

            std::unique_lock<std::mutex> ul(muxBlocking); // wake up wait function
            cvBlocking.notify_one();

            return true;
        }

        bool push_front(const T &item) {
            std::scoped_lock lock(muxQueue);

            if (deqQueue.size() >= capacity)
                return false;

            deqQueue.emplace_front(std::move(item));

            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();

            return true;
        }

        bool empty() {
            std::scoped_lock lock(muxQueue);
            return deqQueue.empty();
        }

        bool overload() {
            std::scoped_lock lock{muxQueue};
            return deqQueue.size() == capacity;
        }

        size_t count() {
            std::scoped_lock lock(muxQueue);
            return deqQueue.size();
        }

        void clear() {
            std::scoped_lock lock(muxQueue);
            deqQueue.clear();
        }

        T pop_front() {
            std::scoped_lock lock(muxQueue);
            if (deqQueue.empty())
                throw std::out_of_range("tsqueue is empty, pop_front() is forbidden");
            T t(std::move(deqQueue.front()));
            deqQueue.pop_front();
            return t;
        }

        T pop_back() {
            std::scoped_lock lock(muxQueue);
            if (deqQueue.empty())
                throw std::out_of_range("tsqueue is empty, pop_back() is forbidden");
            T t(std::move(deqQueue.back()));
            deqQueue.pop_back();
            return t;
        }

        void wait()
        {
            while (empty()) {
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.wait(ul);
            }
        }


    protected:
        size_t capacity;

        std::mutex muxQueue;
        std::deque<T> deqQueue;

        std::condition_variable cvBlocking;
        std::mutex muxBlocking;

    };

}

#endif //CALLCENTRE_THREADSAFEQUEUE_H
