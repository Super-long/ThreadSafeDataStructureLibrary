#include <exception> //exception
#include <memory> //shared_ptr
#include <mutex> //mutex lock_guard
#include <thread> //thread
#include <vector> //vector
#include <future> //promise
#include <atomic> //atomic<bool>

class join_threads{
    std::vector<std::thread>& threads;
    public: 
        explicit join_threads(std::vector<std::thread>& thread_) 
            : threads(thread_) {}
        join_threads() = delete;

        ~join_threads(){
            for(size_t i = 0; i < threads.size(); i++){
                if(threads[i].joinable())
                    threads[i].join();
            }
        }
};

template<typename Iterator, typename Matchtype>
Iterator 
parallel_find(Iterator lhs, Iterator rhs,
        Matchtype match){
    struct find_element{
        void operator()(Iterator lhs, Iterator rhs,
        Matchtype match, std::promise<Iterator>* result,
        std::atomic<bool>* flag){
            try{
                for(; rhs != lhs & !flag->load(); ++lhs){
                    if(*lhs == match){
                        result->set_value(lhs);
                        flag->store(true);
                        return;
                    }
                }
            }catch(...){
                try{
                    result->set_exception(std::current_exception());
                    flag->store(true);
                }catch(...){}
                //Guaranted exception safety may promise had already be set, so discard exception.
            }
        }
    };
    
    unsigned long const length = std::distance(lhs, rhs);
    if(!length) 
        return rhs;
    constexpr unsigned long const min_per_thread = (64 + sizeof(Matchtype) -1)/sizeof(Matchtype);
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const num_threads = std::min(max_threads, static_cast<unsigned long>(std::thread::hardware_concurrency()));

    unsigned long const block_size = length / num_threads;
    std::promise<Iterator> result;
    std::atomic<bool> flag(false);
    std::vector<std::thread> threads(num_threads - 1);
    {
        join_threads joiner(threads);
        Iterator block_start = lhs;
        for(size_t i = 0; i < (num_threads - 1); i++){
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            
            threads[i] = std::thread(find_element(), block_start, block_end,
                match, &result, &flag);
            block_start = block_end;
        }
        find_element()(block_start, rhs, match, &result, &flag);
    }
    if(!flag.load())
        return rhs;
    return result.get_future().get();
}