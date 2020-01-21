#include<bits/stdc++.h>
#include "Queue.h"
using namespace std;

//test of wait_and_pop. void (*)(int&)
/* int main(){
    Threadsafe_Queue<int> que;
    std::vector<std::thread> vec(10);
    std::vector<int> store(10, 0);
    typedef void (Threadsafe_Queue<int>::*function)(int&);
    function f = &Threadsafe_Queue<int>::wait_and_pop;
    for(int i = 0; i < 10; i++){
        if(i&1)
            vec[i] = std::thread(&Threadsafe_Queue<int>::push, &que, i);
        else 
            vec[i] = std::thread(f, &que, std::ref(store[i]));
    }
    std::for_each(vec.begin(), vec.end(), std::mem_fn(&std::thread::join));
    for(size_t i = 0; i < 10; i++){
        std::cout << store[i] << std::endl;
    }
    return 0;
} */

//test of try_pop.  bool(*)(int&)
/* int main(){
    Threadsafe_Queue<int> que;
    std::vector<std::thread> vec(10);
    std::vector<int> store(10, 0);
    typedef bool(Threadsafe_Queue<int>::*function)(int&);
    function f = &Threadsafe_Queue<int>::try_pop;
    for(int i = 0; i < 10; i++){ 
        if(i&1)
            vec[i] = std::thread(&Threadsafe_Queue<int>::push, &que, i);
        else 
            vec[i] = std::thread(f, &que, std::ref(store[i]));
    }
    std::for_each(vec.begin(), vec.end(), std::mem_fn(&std::thread::join));
    for(size_t i = 0; i < 10; i++){
        std::cout << store[i] << std::endl;
    }
    return 0;
} */

//test of wait_and_pop.std::shared_ptr<int>(*)()
/* int main(){
    Threadsafe_Queue<int> que;
    std::vector<std::future<std::shared_ptr<int>>> vec(10);
    std::vector<int> store(10, 0);
    typedef std::shared_ptr<int>(Threadsafe_Queue<int>::*function)();
    function f = &Threadsafe_Queue<int>::wait_and_pop;
    for(int i = 0; i < 10; i++){ 
        if(i&1){
            auto T = std::thread(&Threadsafe_Queue<int>::push, &que, i);
            T.detach();
        }
        else 
            vec[i] = std::async(f, &que);
    }
    for(size_t i = 0; i < 10; i+=2)
    {
        cout << *(vec[i].get()) << endl;
    }
    return 0;
} */

//test of try_pop. std::shared_ptr<int>(*)()
int main(){
    Threadsafe_Queue<int> que;
    std::vector<std::future<std::shared_ptr<int>>> vec(5);
    typedef std::shared_ptr<int>(Threadsafe_Queue<int>::*function)();
    function f = &Threadsafe_Queue<int>::try_pop;
    for(int i = 0; i < 10; i++){ 
        if(i&1){
            auto T = std::thread(&Threadsafe_Queue<int>::push, &que, i);
            T.detach();
        }
        else 
            vec[i/2] = std::async(f, &que);
    }
    for(size_t i = 0; i < 5; ++i)
    { //当一次get以后继续get的时候就会抛出future_error
        auto T = vec[i].get();
        if(T)
            cout << i << " : " << *T << endl;
        else 
            cout << "vaild pointer : " << i << endl;
    }
    return 0;
}