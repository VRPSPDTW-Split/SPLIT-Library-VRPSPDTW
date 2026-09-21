#ifndef NODE_H
#define NODE_H



// Node struct for queue implementation
struct Node
{
    int value;
    Node* next;
    Node* prev;

    Node(int val) : value(val), next(nullptr), prev(nullptr) {}
    Node() : value(0), next(nullptr), prev(nullptr) {}
    ~Node() {}
};

#endif

