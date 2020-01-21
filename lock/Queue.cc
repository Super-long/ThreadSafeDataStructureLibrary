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

template<typename T>
class Threadsafe_Queue{
    private:
        struct node{
            std::shared_ptr<T> data;//指针拷贝一份大多优于数据拷贝一份
            std::unique_ptr<node> next;
        };

        std::mutex head_mutex;
        std::mutex tail_mutex;
        std::unique_ptr<node> head; //没有引用计数的损耗
        node* tail;
        
        node* get_tail(){
            std::lock_guard<std::mutex> guard(tail_mutex);
            return tail;
        }

        std::unique_ptr<node> pop_head(){
            std::lock_guard<std::mutex> guard(head_mutex);
            if(head.get() == get_tail()){
                return nullptr;
            }
            auto Return_head = std::move(head);
            head = std::move(Return_head->next);
            return Return_head;
        }
    public:
        Threadsafe_Queue() : 
            head(new node), tail(head.get()){} //精髓 使用一个傀儡节点以减小锁的粒度

        //try_pop push均为异常安全
        std::shared_ptr<T> try_pop(){
            auto head = pop_head();
            return head ? head->data : std::shared_ptr<T>();
        }

        void push(T value){
            std::shared_ptr<T> new_node = 
                std::make_shared<T>(std::move(value));
            std::unique_ptr<node> Temp = 
                std::make_unique<node>();
            node* const new_tail = Temp.get();
            std::lock_guard<std::mutex> guard(tail_mutex);
            tail->data = new_node;
            tail->next = std::move(Temp);
            tail = new_tail;           
        }
};

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