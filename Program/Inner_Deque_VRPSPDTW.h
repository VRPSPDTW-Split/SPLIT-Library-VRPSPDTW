#ifndef INNER_DEQUE_VRPSPDTW_H
#define INNER_DEQUE_VRPSPDTW_H

#include "Node.h"

class Inner_Deque_VRPSPDTW
{
private:
    Node frontDummy;
    Node backDummy;
    int highest_point;

public:
    Inner_Deque_VRPSPDTW()
        : frontDummy(-1), backDummy(-1), highest_point(-1){
        frontDummy.next = &backDummy;
        backDummy.prev  = &frontDummy;
    }

    Inner_Deque_VRPSPDTW(const Inner_Deque_VRPSPDTW&) = delete;
    Inner_Deque_VRPSPDTW& operator=(const Inner_Deque_VRPSPDTW&) = delete;

    inline bool is_empty(){return backDummy.prev == &frontDummy;}
    inline int get_front(){return frontDummy.next->value;}
    inline int get_next_front(){return frontDummy.next->next->value;}
    inline int get_back(){return backDummy.prev->value;}
    inline int get_next_back(){return backDummy.prev->prev->value;}
    inline int get_highest_point(){return highest_point;}
    inline void set_highest_point(int point){highest_point = point;}

    inline void push_back(Node* node){
        node->prev = backDummy.prev;
        node->next = &backDummy;
        node->prev->next = node;
        backDummy.prev   = node;
    }

    inline void pop_back(){
        backDummy.prev = backDummy.prev->prev;
        backDummy.prev->next = &backDummy;
    }

    inline void pop_front(){
        frontDummy.next = frontDummy.next->next;
        frontDummy.next->prev = &frontDummy;
    }

    // Splices deque in front of this deque in O(1). The newest element in
    // deque becomes adjacent to the oldest element in this deque. The nodes
    // remain externally owned, and deque is left empty.
    inline void merge(Inner_Deque_VRPSPDTW& deque){
        if (&deque == this || deque.is_empty()){
            return;
        }

        Node* oldFront = frontDummy.next;
        frontDummy.next = deque.frontDummy.next;
        frontDummy.next->prev = &frontDummy;
        deque.backDummy.prev->next = oldFront;
        oldFront->prev = deque.backDummy.prev;

        deque.frontDummy.next = &deque.backDummy;
        deque.backDummy.prev = &deque.frontDummy;
    }

    ~Inner_Deque_VRPSPDTW(){}
};

#endif
