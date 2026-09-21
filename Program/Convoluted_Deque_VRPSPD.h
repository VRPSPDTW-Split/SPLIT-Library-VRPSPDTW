#ifndef CONVOLUTED_DEQUE_VRPSPD_H
#define CONVOLUTED_DEQUE_VRPSPD_H

#include "Node.h"

class Convoluted_Deque_VRPSPD
{
private:
    Node frontDummy;
    Node backDummy;
    Node* bestPtr;

public:
    Convoluted_Deque_VRPSPD(): frontDummy(-1), backDummy(-1), bestPtr(nullptr){
        frontDummy.next = &backDummy;
        backDummy.prev  = &frontDummy;
    }

    inline bool is_empty(){return backDummy.prev == &frontDummy;}
    inline int back(){return backDummy.prev->value;}
    inline int best(){return bestPtr->value;}
    inline bool has_prev(){return bestPtr->prev != &frontDummy;}
    inline int prev(){return bestPtr->prev->value;}
    inline bool has_next(){return bestPtr->next != &backDummy;}
    inline int next(){return bestPtr->next->value;}
    inline void remove_prev(){bestPtr->prev = bestPtr->prev->prev; bestPtr->prev->next = bestPtr;}
    inline void remove_next(){bestPtr->next = bestPtr->next->next; bestPtr->next->prev = bestPtr;}
    inline void move_prev(){bestPtr = bestPtr->prev;}
    inline void move_next(){bestPtr = bestPtr->next;}

    void push_back(Node* node){
        if(bestPtr == nullptr){
            bestPtr = node;
        }
        node->prev = backDummy.prev;
        node->next = &backDummy;
        node->prev->next = node;
        backDummy.prev   = node;
    }

    void pop_back(){
        if(backDummy.prev == bestPtr){
            bestPtr = nullptr;
        }
        backDummy.prev = backDummy.prev->prev;
        backDummy.prev->next = &backDummy;
    }

    ~Convoluted_Deque_VRPSPD(){}
};

#endif