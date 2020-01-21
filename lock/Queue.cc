#include <exception> //exception
#include <memory> //shared_ptr
#include <mutex> //mutex lock_guard
#include <queue> //queue
#include <condition_variable> //condition_variable

#include<vector>
#include<thread>
#include<algorithm>
#include<functional>
#include<iostream>

/**
 * 最精彩的三步
 * 傀儡节点
 * 数据存为智能指针 解决值的异常安全
 * 
*/

template<typename T>
class Threadsafe_Queue{
    private:
        struct node{
            std::shared_ptr<T> data;//指针拷贝一份大多优于数据拷贝一份
            std::unique_ptr<node> next;
        };

        std::mutex head_mutex;
        std::mutex tail_mutex;
        std::unique_ptr<node> head;
        node* tail;
        std::condition_variable data_cond;
        
        std::unique_ptr<node> pop_head(){
            auto Return_head = std::move(head);
            head = std::move(Return_head->next);
            return Return_head;
        }

        node* get_tail(){
            std::lock_guard<std::mutex> guard(tail_mutex);
            return tail;
        }

        std::unique_ptr<node> wait_pop_head(){
            std::unique_lock<std::mutex> head_lock(head_mutex);
            data_cond.wait(head_lock, [&](){return head.get()!=get_tail();})
            return pop_head();
        }

        void wait_pop_head(T& value){
            std::unique_lock<std::mutex> head_lock(head_mutex);
            data_cond.wait(head_lock, [&](){return head.get()!=get_tail();})
            value = std::move(*head->data);//数据存为智能指针 完美解决异常安全
            pop_head();
        }

        std::unique_ptr<node> try_pop_head(){
            std::lock_guard<std::mutex> guard(head_mutex);
            if(head.get() == get_tail()){
                return nullptr;
            }
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head(T& value){
            std::lock_guard<std::mutex> guard(head_mutex);
            if(head.get() == get_tail()){
                return nullptr;
            }
            value = std::move(&head->data);
            return pop_head();
        }        

    public:
        Threadsafe_Queue() : 
            head(new node), tail(head.get()){} //精髓 使用一个傀儡节点以减小锁的粒度

        Threadsafe_Queue(const Threadsafe_Queue&) = delete;
        Threadsafe_Queue& operator=(const Threadsafe_Queue&) = delete;
        
        std::shared_ptr<T> wait_and_pop();
        void wait_and_pop(T& value);

        std::shared_ptr<T> try_pop();
        bool try_pop(T& value);

        void push(T value);

        bool empty(){
            std::lock_guard<std::mutex> head_lock(head_mutex);
            return (head.get() == get_tail());
        }
};

template<typename T>
std::shared_ptr<T>
Threadsafe_Queue<T>::wait_and_pop(){
    auto const old_head = wait_pop_head();
    return old_head->data;
}

template<typename T>
void 
Threadsafe_Queue<T>::wait_and_pop(T& value){
    wait_pop_head(value);
}

template<typename T>
std::shared_ptr<T>
Threadsafe_Queue<T>::try_pop(){
    auto const old_head = try_pop_head(); //TODO const
    return old_head ? old_head->data : std::shared_ptr<T>();
}

template<typename T>
bool
Threadsafe_Queue<T>::try_pop(T& value){
    return std::static_cast<bool>(try_pop_head(value));
}

template<typename T>
void 
Threadsafe_Queue<T>::push(T value){
    std::shared_ptr<T> new_node = 
        std::make_shared<T>(std::move(value));
    std::unique_ptr<node> Temp = 
        std::make_unique<node>();
    node* const new_tail = Temp.get();
    {
        std::lock_guard<std::mutex> guard(tail_mutex);
        tail->data = new_node;
        tail->next = std::move(Temp);
        tail = new_tail;  
    }
    data_cond.notify_one();
}


int main(){
    Threadsafe_Queue<int> que;
    std::vector<std::thread> vec(10);
    for(int i = 0; i < 10; i++){
        vec[i] = std::thread(&Threadsafe_Queue<int>::push, &que, i);
    }
    std::for_each(vec.begin(), vec.end(), std::mem_fn(&std::thread::join));
    for(size_t i = 0; i < 10; i++)
    {
        std::cout << *que.try_pop() << std::endl;
    }
    return 0;
}