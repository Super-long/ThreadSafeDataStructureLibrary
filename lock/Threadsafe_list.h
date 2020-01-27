#ifndef _LIST_H_
#define _LIST_H_

#include <algorithm> //find_if
#include <memory> //unique_ptr shared_ptr
#include <mutex> //lock_guard mutex
#include <list> //list
#include <functional> //function

/**
 * @ 不完美的地方在于实现的是自迭代 即迭代由容器完成
 * @ 迭代器模式不好实现 实现意味着对迭代器的所有操作必须上锁 不如通过回调来实现
 * @ 优点在于锁粒度小,并发潜力较高 缺点在于锁太多 使得效率不高 
 * ps: 不好说和一个粒度大的锁的实现(一个锁锁定std::list)之间哪一个效率较高
*/
namespace Threadsafe{
    template<typename T>
    class Threadsafe_List{
        private:
            struct node{
                std::mutex m;
                std::shared_ptr<T> ptr;
                std::unique_ptr<node> next;

                node() : next() {}

                node(const T& data) : 
                    ptr(std::make_shared<T>(data)){}
            };
            node head;
        
        public:
            Threadsafe_List(){}

            ~Threadsafe_List(){
                remove_if([](const node&){return true;});
            }

            Threadsafe_List(const Threadsafe_List&) = delete;
            Threadsafe_List& operator=(const Threadsafe_List&) = delete;

            void push_front(const T& value){
                std::unique_ptr<node> new_node(new node(value));
                std::lock_guard<std::mutex> guard(head.m);
                new_node->next = std::move(head.next); //有头结点
                head.next = std::move(new_node);
            }

            template<typename Function>
            void for_each(const Function& f){
                node* current = &head;
                std::unique_lock<std::mutex> lk(current->m);
                while(node* const next = current->next.get()){
                    std::unique_lock<std::mutex> guard(next->m);
                    lk.unlock();//放在guard外可能next已变
                    f(*next->ptr);
                    current = next;
                    lk = std::move(guard);
                }
            }

            template<typename predicate>
            std::shared_ptr<T> find_first_of(const predicate& f){
                node* current = &head;
                std::unique_lock<std::mutex> lk(current->m);
                while(node* const next = current->next.get()){
                    std::unique_lock<std::mutex> guard(next->m);
                    if(f(*next->ptr)){
                        return next->ptr;
                    }
                    current = next;
                    lk = std::move(guard);
                }
                return std::shared_ptr<T>();
            }

            template<typename predicate>
            void remove_if(const predicate& p){
                node* current = &head;
                std::unique_lock<std::mutex> lk(current->m);
                while(node* const next = current->next.get()){
                    std::unique_lock<std::mutex> guard(next->m);
                    if(p(*next->ptr)){
                        std::unique_ptr<node> old_next = std::move(current->next);
                        current->next = std::move(next->next);
                        guard.unlock();
                    }else{
                        lk.unlock();
                        current = next;
                        lk = std::move(guard);
                    }
                }
            }

            //从泛型改为function的原因是thread参数使用泛型会有问题 那个问题现在不好解决
            void update(std::function<bool(T&)> predicate, std::function<void(T&)> function){
                node* current = &head;
                std::unique_lock<std::mutex> lk(current->m);
                while(node* const next = current->next.get()){
                    std::unique_lock<std::mutex> guard(next->m);
                    lk.unlock();
                    if(predicate(*next->ptr))
                        function(*next->ptr);
                    current = next;
                    lk = std::move(guard);
                }
            }

            std::list<T> get_list(){
                node* current = &head;
                std::list<T> stores;
                std::vector<std::unique_lock<std::mutex>> lks;
                {
                    //锁住这里意味着get_list的时候list是一个不变量
                    std::lock_guard<std::mutex> lk(head.m); 
                    while(node* const next = current->next.get()){
                        lks.emplace_back(std::unique_lock<std::mutex>(next->m));
                        stores.push_back(*next->ptr);
                        current = next;
                    }
                }
                return std::move(stores);
            }
    };
}

#endif //_List_H_