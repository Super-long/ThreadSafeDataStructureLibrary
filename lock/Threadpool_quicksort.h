#ifndef THREADPOOL_QUICKSORT_H_
#define THREADPOOL_QUICKSORT_H_

#include <list>
#include <algorithm>
#include <atomic>
#include <thread>
#include <future>
#include <vector>
#include <functional>
#include "Threadsafe_stack.h"

#include<iostream>
template<typename T>
class sorter{
    private:
        struct chunk_to_sort{
            std::list<T> data;
            std::promise<std::list<T>> pro;
            //chunk_to_sort(const chunk_to_sort&) = delete;//debug
            chunk_to_sort(chunk_to_sort&&) = default;
            chunk_to_sort() = default;
        };
        std::vector<std::thread> pool;
        std::atomic_bool end_of_data;
        const uint64_t max_thread_count;
        threadsafe_stack<chunk_to_sort> chunks;

    public:
        sorter() : end_of_data(false), chunks(),
            max_thread_count(std::thread::hardware_concurrency() - 1){}
        
        ~sorter(){
            end_of_data = true;
            std::for_each(pool.begin(), pool.end(), std::mem_fn(&std::thread::join));
        }

        std::list<T> do_sort(std::list<T>& chunk_data){
            if(chunk_data.empty()) return std::move(chunk_data);

            std::list<T> result;
            result.splice(result.begin(), chunk_data, chunk_data.begin());
            const T& partition_val = *result.begin();
            auto divide_point = std::partition(chunk_data.begin(), chunk_data.end(),
                [&](const T& val){return val < partition_val;});
            
            chunk_to_sort new_lower_chunk;
            new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk_data,
                    chunk_data.begin(), divide_point);
            std::future<std::list<T>> new_lower = new_lower_chunk.pro.get_future();
            chunks.push(std::move(new_lower_chunk));
            if(pool.size() <= max_thread_count){
                pool.emplace_back(std::thread(&sorter<T>::sort_thread, this));
            }

            std::list<T> new_higher(do_sort(chunk_data));
            result.splice(result.end(), new_higher);
            while(new_lower.wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready){
                try_sort_chunk();
            }
            result.splice(result.begin(), new_lower.get());
            return std::move(result);
        }

        void sort_thread(){
            while(!end_of_data){
                try_sort_chunk();
                std::this_thread::yield();
            }
        }

        void try_sort_chunk(){
            try{
                std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
                if(chunk){
                    sort_chunk(chunk);
                }
            }catch(std::exception&){}
        }

        void sort_chunk(const std::shared_ptr<chunk_to_sort>& ptr){
            ptr->pro.set_value(do_sort(ptr->data));
        }
};

template<typename T>
std::list<T> parallel_quicksort(std::list<T>& input){
    if(input.size() < 2){
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
} 

#endif //THREADPOOL_QUICKSORT_H_