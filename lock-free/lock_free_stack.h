#ifndef LOCK_FREE_STACK_H_
#define LOCK_FREE_STACK_H_

#include <atomic> //atomic
#include <memory> //unique_ptr

template<typename Type>
class lock_free_stack{
private:
    struct Node{
        std::shared_ptr<Type> data;
        Node* next;
        node(const Type& d) : data(std::make_shared<Type>(d)) {}
    };
    std::atomic<Node*> head;
public:
    void
    push(const Type& para){
        Node* const NewNode = new Node(para);
        NewNode->next = head.load();
        while(!head.compare_exchange_weak(NewNode->next, NewNode));
    }

    std::shared_ptr<Type>
    pop(){
        Node* old_head = head.load();
        while(!old_head && !head.compare_exchange_weak(old_head, old_head->next));
        return old_head ? old_head->next : std::shared_ptr<Type>();
    }
};

#endif //LOCK_FREE_STACK_H_