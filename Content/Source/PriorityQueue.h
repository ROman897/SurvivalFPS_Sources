#pragma once

template <typename InElementType>
struct TPriorityQueueNode {
    InElementType Element;
    float Priority;

    TPriorityQueueNode() = default;

    TPriorityQueueNode(const InElementType& InElement, float InPriority)
    {
        Element = InElement;
        Priority = InPriority;
    }

    bool operator<(const TPriorityQueueNode<InElementType>& Other) const
    {
        return Priority < Other.Priority;
    }
};

template <typename InElementType>
class TPriorityQueue {
public:
    TPriorityQueue() = default;

public:
    // Always check if IsEmpty() before Pop-ing!
    InElementType Pop()
    {
        TPriorityQueueNode<InElementType> Node;
        Array.HeapPop(Node);
        return Node.Element;
    }

    void Push(const InElementType& Element, float Priority)
    {
        Array.HeapPush(TPriorityQueueNode<InElementType>(Element, Priority));
    }

    bool IsEmpty() const
    {
        return Array.Num() == 0;
    }

private:
    TArray<TPriorityQueueNode<InElementType>> Array;
};