#ifndef DATA_HETEROGENOUSRINGBUFFER_HPP_
#define DATA_HETEROGENOUSRINGBUFFER_HPP_

#include "../common/Semaphore.hpp"
#include <assert.h>

namespace Data {

/**
 * Fixed @p BYTES size ring buffer able to contain heterogeneous elements
 * derived from defined interface @p T.
 *
 * The objects are actually copied to the buffer itself so they need to be
 * copyable.
 */
template <typename T, size_t BYTES>
class HeterogenousRingBuffer
{
public:
    HeterogenousRingBuffer();

    template <typename U>
    void enqueue(const U& element);

    bool isEmpty() const;

    const T& dequeue();

private:
    using byte = unsigned char;

    byte buffer_[BYTES];
    byte* begin_;
    byte* end_;

    byte* front_;
    bool hasPreviousMessage;
    byte* back_;

    Common::Semaphore freeSpace;
    Common::Semaphore queuedMessages;

    void releaseSpace(size_t bytes);
    void waitForSpace(size_t bytes);
    void notifyNewMessage();
    void waitForMessage();

    struct Envelope
    {
        Envelope(byte* next, T* element);
        bool isNull() const;

        byte* next_;
        T* element_;
    };

    template <typename U>
    struct ElementEnvelope : public Envelope
    {
        ElementEnvelope(byte* next, const U& element);

        U concreteElement_;
    };

    template <typename U>
    void insertElement(const U& element);

    void insertPadding();

    const Envelope* readFront();

    size_t getPotentialFreeSpaceAtBack() const;
};

template <typename T, size_t BYTES>
HeterogenousRingBuffer<T, BYTES>::HeterogenousRingBuffer() :
    buffer_(),
    begin_(buffer_),
    end_(buffer_ + BYTES),
    front_(buffer_),
    hasPreviousMessage(false),
    back_(buffer_),
    freeSpace(BYTES),
    queuedMessages(0)
{
}

template <typename T, size_t BYTES>
template <typename U>
void HeterogenousRingBuffer<T, BYTES>::enqueue(const U& element)
{
    const size_t envelopeSize = sizeof(ElementEnvelope<U>);
    const size_t potentialSpaceAtBack = getPotentialFreeSpaceAtBack();
    assert(potentialSpaceAtBack >= sizeof(Envelope)); // An null envelope should always fit in the back

    const size_t minimumSpaceNeededAtBack = envelopeSize + sizeof(Envelope); // Extra space for potential null envelope
    const bool canFitInBack = potentialSpaceAtBack >= minimumSpaceNeededAtBack;
    if (canFitInBack)
    {
        waitForSpace(envelopeSize);
        insertElement(element);
    }
    else
    {
        // We'll need the leftover space in the back + space for the actual element from the beginning
        waitForSpace(potentialSpaceAtBack + envelopeSize);

        insertPadding(); // To fill leftover
        insertElement(element);
    }

    notifyNewMessage();
}

template <typename T, size_t BYTES>
bool HeterogenousRingBuffer<T, BYTES>::isEmpty() const
{
    return queuedMessages.getCount() == 0;
}

template <typename T, size_t BYTES>
const T& HeterogenousRingBuffer<T, BYTES>::dequeue()
{
    // If there's a previous message, release it
    if (hasPreviousMessage)
    {
        const byte* previousFront = front_;

        const Envelope* envelope = readFront();
        front_ = envelope->next_;
        releaseSpace(front_ - previousFront);

        hasPreviousMessage = false;
    }

    waitForMessage();

    // Skip any padding
    const Envelope* envelope = readFront();
    while (envelope->isNull())
    {
        releaseSpace(end_ - front_);
        front_ = envelope->next_;
        assert(front_ == begin_);

        envelope = readFront();
    }

    hasPreviousMessage = true;
    return *envelope->element_;
}

template <typename T, size_t BYTES>
void HeterogenousRingBuffer<T, BYTES>::releaseSpace(size_t bytes)
{
    freeSpace.notify(bytes);
}
template <typename T, size_t BYTES>
void HeterogenousRingBuffer<T, BYTES>::waitForSpace(size_t bytes)
{
    freeSpace.wait(bytes);
}
template <typename T, size_t BYTES>
void HeterogenousRingBuffer<T, BYTES>::notifyNewMessage()
{
    queuedMessages.notify();
}
template <typename T, size_t BYTES>
void HeterogenousRingBuffer<T, BYTES>::waitForMessage()
{
    queuedMessages.wait();
}

template <typename T, size_t BYTES>
HeterogenousRingBuffer<T, BYTES>::Envelope::Envelope(byte* next, T* element) :
    next_(next),
    element_(element)
{
}

template <typename T, size_t BYTES>
bool HeterogenousRingBuffer<T, BYTES>::Envelope::isNull() const
{
    return element_ == nullptr;
}

template <typename T, size_t BYTES>
template <typename U>
HeterogenousRingBuffer<T, BYTES>::ElementEnvelope<U>::ElementEnvelope(byte* next, const U& element) :
    Envelope(next, &concreteElement_),
    concreteElement_(element)
{
}

template <typename T, size_t BYTES>
template <typename U>
void HeterogenousRingBuffer<T, BYTES>::insertElement(const U& element)
{
    byte* next = back_ + sizeof(ElementEnvelope<U>);
    ElementEnvelope<U>* newElement = new (back_) ElementEnvelope<U>(next, element);
    back_ = next;
}

template <typename T, size_t BYTES>
void HeterogenousRingBuffer<T, BYTES>::insertPadding()
{
    Envelope* nullElement = new (back_) Envelope(begin_, nullptr);
    back_ = begin_;
}

template <typename T, size_t BYTES>
const typename HeterogenousRingBuffer<T, BYTES>::Envelope* HeterogenousRingBuffer<T, BYTES>::readFront()
{
    return reinterpret_cast<Envelope*>(front_);
}

template <typename T, size_t BYTES>
size_t HeterogenousRingBuffer<T, BYTES>::getPotentialFreeSpaceAtBack() const
{
    return static_cast<size_t>(end_ - back_);
}

} // Data

#endif
