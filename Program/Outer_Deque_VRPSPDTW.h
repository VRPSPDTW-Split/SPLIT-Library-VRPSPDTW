#ifndef OUTER_DEQUE_VRPSPDTW_H
#define OUTER_DEQUE_VRPSPDTW_H

#include "Inner_Deque_VRPSPDTW.h"

#include <cmath>
#include <memory>
#include <vector>

// Deque of inner deques grouped by a common running load maximum.
// Each inner deque uses Inner_Deque_VRPSPDTW for its retained predecessors.
class Outer_Deque_VRPSPDTW
{
private:
    static constexpr double EPS = 1.e-7;

    struct InnerDeque
    {
        Inner_Deque_VRPSPDTW predecessors;

        InnerDeque() : predecessors() {}
    };

    struct StackEntry
    {
        int deque;
        int best;
    };

    std::unique_ptr<InnerDeque[]> dequeData;
    std::vector<Node> predecessorNodes;
    std::vector<double> fixedKey;
    std::vector<StackEntry> leftStack;
    std::vector<StackEntry> rightStack;
    std::vector<int> dequeOrder;
    std::vector<int> dequesToMerge;
    const std::vector<double>* sumPickup;
    const std::vector<double>* sumLoad;
    int nextDeque;
    double multiplier;

    inline double get_load_maximum(int deque)
    {
        const int point = dequeData[deque].predecessors.get_highest_point();
        return (*sumPickup)[point] - (*sumLoad)[point];
    }

    inline double dequeScore(int deque)
    {
        InnerDeque& value = dequeData[deque];
        return fixedKey[value.predecessors.get_front()]
            + multiplier * get_load_maximum(deque);
    }

    inline int betterDeque(int first, int second)
    {
        if (first < 0) return second;
        if (second < 0) return first;
        const double firstScore = dequeScore(first);
        const double secondScore = dequeScore(second);
        if (firstScore < secondScore - EPS) return first;
        if (secondScore < firstScore - EPS) return second;
        return dequeData[first].predecessors.get_front()
            > dequeData[second].predecessors.get_front() ? first : second;
    }

    inline void pushLeft(int deque)
    {
        const int aggregate = leftStack.empty()
            ? deque : betterDeque(leftStack.back().best, deque);
        leftStack.push_back({deque, aggregate});
    }

    inline void pushRight(int deque)
    {
        const int aggregate = rightStack.empty()
            ? deque : betterDeque(rightStack.back().best, deque);
        rightStack.push_back({deque, aggregate});
    }

    void rebuild(int leftCount)
    {
        dequeOrder.clear();
        for (int i = static_cast<int>(leftStack.size()) - 1; i >= 0; --i)
            dequeOrder.push_back(leftStack[i].deque);
        for (const StackEntry& entry : rightStack)
            dequeOrder.push_back(entry.deque);

        leftStack.clear();
        rightStack.clear();
        for (int i = leftCount - 1; i >= 0; --i)
            pushLeft(dequeOrder[i]);
        for (int i = leftCount; i < static_cast<int>(dequeOrder.size()); ++i)
            pushRight(dequeOrder[i]);
    }

    inline void ensureLeft()
    {
        if (!leftStack.empty()) return;
        const int count = static_cast<int>(rightStack.size());
        if (count > 0) rebuild((count + 1) / 2);
    }

    inline void ensureRight()
    {
        if (!rightStack.empty()) return;
        const int count = static_cast<int>(leftStack.size());
        if (count > 0) rebuild(count / 2);
    }

    inline int backDeque()
    {
        ensureRight();
        return rightStack.back().deque;
    }

    inline int popFrontDeque()
    {
        ensureLeft();
        const int result = leftStack.back().deque;
        leftStack.pop_back();
        return result;
    }

    inline int popBackDeque()
    {
        ensureRight();
        const int result = rightStack.back().deque;
        rightStack.pop_back();
        return result;
    }

    inline void appendPredecessor(InnerDeque& deque, int predecessor)
    {
        while (!deque.predecessors.is_empty() &&
               fixedKey[deque.predecessors.get_back()]
                   >= fixedKey[predecessor] - EPS)
            deque.predecessors.pop_back();
        predecessorNodes[predecessor].value = predecessor;
        deque.predecessors.push_back(&predecessorNodes[predecessor]);
    }

    inline void prependPredecessors(InnerDeque& destination, InnerDeque& prefix)
    {
        while (!prefix.predecessors.is_empty() &&
               fixedKey[prefix.predecessors.get_back()]
                   >= fixedKey[destination.predecessors.get_front()] - EPS)
            prefix.predecessors.pop_back();
        destination.predecessors.merge(prefix.predecessors);
    }

public:
    struct Minimum
    {
        int predecessor;
        double score;
    };

    Outer_Deque_VRPSPDTW()
        : sumPickup(nullptr), sumLoad(nullptr), nextDeque(0), multiplier(0.0) {}

    void reset(int size, double penaltyMultiplier,
               const std::vector<double>& pickupValues,
               const std::vector<double>& loadValues)
    {
        dequeData.reset(new InnerDeque[size + 1]);
        predecessorNodes.resize(size + 1);
        fixedKey.resize(size + 1);

        leftStack.clear();
        rightStack.clear();
        dequeOrder.clear();
        dequesToMerge.clear();
        leftStack.reserve(size + 1);
        rightStack.reserve(size + 1);
        dequeOrder.reserve(size + 1);
        dequesToMerge.reserve(size + 1);

        sumPickup = &pickupValues;
        sumLoad = &loadValues;
        nextDeque = 0;
        multiplier = penaltyMultiplier;
    }

    inline bool empty() const
    {
        return leftStack.empty() && rightStack.empty();
    }

    // Gamma_2.front
    inline int front()
    {
        ensureLeft();
        return dequeData[leftStack.back().deque].predecessors.get_front();
    }

    // Gamma_2.back
    inline int back()
    {
        ensureRight();
        return dequeData[rightStack.back().deque].predecessors.get_back();
    }

    // Gamma_2.pushBack(i, x)
    inline void pushBack(int predecessor, int highestPoint, double key)
    {
        fixedKey[predecessor] = key;

        if (!empty())
        {
            const int last = backDeque();
            const double highestValue = (*sumPickup)[highestPoint]
                - (*sumLoad)[highestPoint];
            if (std::abs(get_load_maximum(last) - highestValue) <= EPS)
            {
                popBackDeque();
                appendPredecessor(dequeData[last], predecessor);
                pushRight(last);
                return;
            }
        }

        const int deque = nextDeque++;
        InnerDeque& value = dequeData[deque];
        value.predecessors.set_highest_point(highestPoint);
        appendPredecessor(value, predecessor);
        pushRight(deque);
    }

    // Gamma_2.popFront; highestPoint receives the inner deque's label.
    inline int popFront(int& highestPoint)
    {
        const int deque = popFrontDeque();
        InnerDeque& value = dequeData[deque];
        const int predecessor = value.predecessors.get_front();
        highestPoint = value.predecessors.get_highest_point();
        value.predecessors.pop_front();

        if (!value.predecessors.is_empty())
            pushLeft(deque);

        return predecessor;
    }

    // Gamma_2.popBack
    inline int popBack()
    {
        const int deque = popBackDeque();
        InnerDeque& value = dequeData[deque];
        const int predecessor = value.predecessors.get_back();
        value.predecessors.pop_back();

        if (!value.predecessors.is_empty())
            pushRight(deque);

        return predecessor;
    }

    // Gamma_2.updateLoads(x)
    inline void updateLoads(int newHighestPoint)
    {
        dequesToMerge.clear();
        while (!empty())
        {
            const int deque = backDeque();
            const double highestValue = (*sumPickup)[newHighestPoint]
                - (*sumLoad)[newHighestPoint];
            if (get_load_maximum(deque) >= highestValue - EPS)
                break;
            dequesToMerge.push_back(popBackDeque());
        }
        if (dequesToMerge.empty()) return;

        const int merged = dequesToMerge.front();
        InnerDeque& destination = dequeData[merged];
        for (int p = 1; p < static_cast<int>(dequesToMerge.size()); ++p)
            prependPredecessors(destination, dequeData[dequesToMerge[p]]);
        destination.predecessors.set_highest_point(newHighestPoint);
        pushRight(merged);
    }

    // Gamma_2.minimum
    inline Minimum minimum()
    {
        const int leftBest = leftStack.empty() ? -1 : leftStack.back().best;
        const int rightBest = rightStack.empty() ? -1 : rightStack.back().best;
        const int deque = betterDeque(leftBest, rightBest);
        return {dequeData[deque].predecessors.get_front(), dequeScore(deque)};
    }
};

#endif
