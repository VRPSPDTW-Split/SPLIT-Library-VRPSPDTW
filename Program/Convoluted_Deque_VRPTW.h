#ifndef CONVOLUTED_DEQUE_VRPTW_H
#define CONVOLUTED_DEQUE_VRPTW_H

#include "Node.h"

class Convoluted_Deque_VRPTW{ 
private:
    Node frontDummy;
    Node backDummy;
    Node* feasibleFront;
    Node* noTimeWarp;

public:
    Convoluted_Deque_VRPTW(): frontDummy(-1), backDummy(-1), feasibleFront(nullptr), noTimeWarp(nullptr) {
        frontDummy.next = &backDummy;
        backDummy.prev  = &frontDummy;
    }
    
    inline bool size_greater_than_1(){return (backDummy.prev == &frontDummy || backDummy.prev->prev == &frontDummy) ? false : true;}
    inline int front(){return frontDummy.next->value;}
    inline int front_2(){return frontDummy.next->next->value;}
    inline bool is_empty(){return backDummy.prev == &frontDummy;}
    inline int back(){return backDummy.prev->value;}
    inline bool has_no_time_warp(){return noTimeWarp != nullptr;}
    inline int no_time_warp(){return noTimeWarp->value;}
    inline void move_no_warp_next(){
        noTimeWarp = noTimeWarp->next;
        if (noTimeWarp == &backDummy) {
            noTimeWarp = nullptr;
        }
    }
    inline bool has_feasible(){return feasibleFront != nullptr;}
    inline int feas(){return feasibleFront->value;}
    inline bool feasible_has_prev(){return feasibleFront != nullptr && feasibleFront->prev != &frontDummy;}
    inline int feasible_prev(){return feasibleFront->prev->value;}
    inline int next(){return feasibleFront->next->value;}
    inline void move_feas_next(){
        feasibleFront = feasibleFront->next;
        if (feasibleFront == &backDummy) {
            feasibleFront = nullptr;
        }
    }

    inline void pop_front(){
        Node* removed = frontDummy.next;
        Node* next = removed->next;
        if (removed == feasibleFront) {
            feasibleFront = next == &backDummy ? nullptr : next;
        }
        if (removed == noTimeWarp) {
            noTimeWarp = next == &backDummy ? nullptr : next;
        }
        frontDummy.next = next;
        frontDummy.next->prev = &frontDummy;
    }

    inline void pop_front_2(){
        Node* first = frontDummy.next;
        Node* removed = first->next;
        Node* next = removed->next;
        if (removed == feasibleFront) {
            feasibleFront = next == &backDummy ? nullptr : next;
        }
        if (removed == noTimeWarp) {
            noTimeWarp = next == &backDummy ? nullptr : next;
        }
        first->next = next;
        next->prev = first;
    }

    inline void push_back(Node* node){
        if(feasibleFront==nullptr){
            feasibleFront = node;
        }
        if(noTimeWarp==nullptr){
            noTimeWarp = node;
        }
        node->prev = backDummy.prev;
        node->next = &backDummy;
        node->prev->next = node;
        node->next->prev = node;
    }

    inline void pop_back(){
        if(backDummy.prev == feasibleFront){
            feasibleFront = nullptr;
        }
        if(backDummy.prev == noTimeWarp){
            noTimeWarp = nullptr;
        }
        backDummy.prev = backDummy.prev->prev;
        backDummy.prev->next = &backDummy;
    }

    inline void remove_feasible_prev(){
        Node* removed = feasibleFront->prev;
        if (removed == noTimeWarp){
            noTimeWarp = feasibleFront;
        }
        feasibleFront->prev = removed->prev;
        feasibleFront->prev->next = feasibleFront;
    }
    ~Convoluted_Deque_VRPTW() {}
};

#endif
