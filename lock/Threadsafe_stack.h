#ifndef THREADSAFESTACK_H_
#define THREADSAFESTACK_H_

#include <exception> //exception
#include <memory> //shared_ptr
#include <mutex> //mutex lock_guard
#include <stack> //stack
#include <thread> //thread

struct empty_stack : std::exception{
    const char* what() const throw(){ //Equivalent to noexcept
        exception::what();
    }
};

struct possible_lost_value : std::exception{
    const char* what() const throw(){
        exception::what();
    }
};

template<typename T>
class threadsafe_stack{
    private:
        std::stack<T> data;
        mutable std::mutex m;
    public:
        threadsafe_stack(){}
        threadsafe_stack(const threadsafe_stack& other){
            std::lock_guard<std::mutex> lock(other.m);
            data = other.data; //copy
        }

        threadsafe_stack& operator=(const threadsafe_stack&);

        //TODO:改成const T& demo_quicksort就会编译错误 我找了两个小时才编译成功 但不知道为什么
        void push(T new_value){
            std::lock_guard<std::mutex> lock(m);
            data.push(std::move(new_value));
        }

        std::shared_ptr<T> pop(){ //缺点: 可能T是一个很小的对象 资源消耗大
            std::lock_guard<std::mutex> lock(m);

            if(data.empty()) throw empty_stack(); //抛错有时效率太低
            std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top()))); //一个顶层const 不允许改变值
            data.pop();
            return res;
        }

        //Requires stored objects to be copied.
        void pop(T& value){
            std::lock_guard<std::mutex> lock(m);
            if(data.empty()) throw empty_stack();
            value = data.top();
            data.pop();
        }

        //There is Optimal operation with movement construction.
        T pop_return_calue() noexcept(false) { //don't overload. 
            std::lock_guard<std::mutex> lock(m);
            if(std::is_nothrow_copy_constructible<T>::value 
                || std::is_nothrow_move_constructible<T>::value){ //ensure safety.
                    if(data.empty()) throw empty_stack("return value");
                    auto Temp = data.top();
                    data.pop();
                    return Temp;
            }
            //possible construction failture. Means the value may be lost.
            throw possible_lost_value();
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(m);
            return data.empty();
        }

        //size操作在此类中可能引起多线程之间的问题 比如以size为判断条件 就可能出现在空栈取元素的情况
        size_t size() const {
            std::lock_guard<std::mutex> lock(m); 
            return data.size();
        }
};
//最大缺点 就是在存在显著竞争的时候,线程的序列化(serialization)可能会严重影响性能
//只能反复调用pop 并且捕获异常,

#endif //THREADSAFESTACK_H_