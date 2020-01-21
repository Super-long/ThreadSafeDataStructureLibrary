#ifndef THREADSAFEQUEUE_H_
#define THREADSAFEQUEUE_H_

#include <exception> //exception
#include <memory> //shared_ptr
#include <mutex> //mutex lock_guard
#include <queue> //queue
#include <condition_variable> //condition_variable

template<typename Type>
class threadsafe_queue
{
    private:
        mutable std::mutex m;
        std::queue<Type> que;
        std::condition_variable data_cond;

    public:
        threadsafe_queue() = default;
        threadsafe_queue(const threadsafe_queue&) = default;
        threadsafe_queue(threadsafe_queue&&) noexcept = default;
        threadsafe_queue& operator=(const threadsafe_queue&) = delete;
        
        void push(const Type& new_value);
        void push(Type&& new_value);
        template<typename T>
        void push(T&& new_value);

        bool try_pop(Type& value);
        std::shared_ptr<Type> try_pop();

        void wait_and_pop(Type& value);
        std::shared_ptr<Type> wait_and_pop();

        bool empty() const;
        size_t size() const;
};

template<typename Type>
void 
threadsafe_queue<Type>::push(const Type& new_value){
    std::lock_guard<std::mutex> guard(m);
    que.emplace(new_value);
    data_cond.notify_one();
}

template<typename Type>
void 
threadsafe_queue<Type>::push(Type&& new_value){
    std::lock_guard<std::mutex> guard(m);
    que.emplace(std::move(new_value));
    data_cond.notify_one();
}

template<typename Type>
bool
threadsafe_queue<Type>::try_pop(Type& value){
    std::lock_guard<std::mutex> guard(m);
    if(que.empty()) return false;
    value = que.front();
    que.push();
    return true;
}

template<typename Type>
std::shared_ptr<Type>
threadsafe_queue<Type>::try_pop(){
    std::lock_guard<std::mutex> guard(m);
    if(que.empty()) return nullptr;
    auto res = std::make_shared<Type>(que.front());
    que.pop();
    return res;
}

template<typename Type>
void 
threadsafe_queue<Type>::wait_and_pop(Type& value){
    std::unique_lock<std::mutex> lk(m);
    data_cond.wait(lk, [this]{return !que.empty();});
    value = que.front();
    que.pop();
}

template<typename Type>
std::shared_ptr<Type>
threadsafe_queue<Type>::wait_and_pop(){
    std::unique_lock<std::mutex> lk(m);
    data_cond.wait(lk, [this]{return !que.empty();});
    try{//Guaranteed exception safety.
        auto res = std::make_shared<Type>(que.front());
        que.pop();
        return res;
    }catch(std::exception&){
        data_cond.notify_one();
    }
}

template<typename Type>
bool
threadsafe_queue<Type>::empty() const {
    std::lock_guard<std::mutex> guard;
    return que.empty();
}

/**
 * It's just a single operation. If it is used as a judgment condition,
 * It's may cause a race condition.
*/
template<typename Type>
size_t
threadsafe_queue<Type>::size() const {
    std::lock_guard<std::mutex> guard;
    return que.size();
}

#endif //THREADSAFEQUEUE_H_