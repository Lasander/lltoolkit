#ifndef DATA_HETEROGENEOUSRINGBUFFER_HPP_
#define DATA_HETEROGENEOUSRINGBUFFER_HPP_

#include "../common/Semaphore.hpp"
#include <assert.h>

namespace Data {

/**
 * Fixed @p BYTES size ring buffer able to contain heterogeneous elements
 * derived from defined interface @p T.
 *
 * The objects are actually copied to the buffer itself so they need to be
 * copyable.
 *
 * Thread-safe for one reader and one writer.
 *
 * Note that there's overhead for each element in the buffer.
 */
template <typename T, size_t BYTES>
class HeterogeneousRingBuffer
{
public:
    HeterogeneousRingBuffer();

    /**
     * Push new element of type @p U to the buffer.
     * Will block waiting for space if there's not enough to push immediately.
     */
    template <typename U>
    void enqueue(const U& element);

    /** @return True if there are no elements in the buffer */
    bool isEmpty() const;

    /**
     * Read oldest element from the buffer
     * Will block if there's no elements in the buffer.
     *
     * @return Reference to oldest element. The reference is valid until next call to dequeue.
     */
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

    template <typename U>
    size_t calculateEnvelopeSize(const U& element)
    {
        static const auto maxAlignment = alignof(std::max_align_t);
        const size_t unalignedSize = sizeof(ElementEnvelope<U>);

        size_t remainder = unalignedSize % maxAlignment;
        if (remainder == 0)
            return unalignedSize;

        return unalignedSize + maxAlignment - remainder;
    }

};

template <typename T, size_t BYTES>
HeterogeneousRingBuffer<T, BYTES>::HeterogeneousRingBuffer() :
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
void HeterogeneousRingBuffer<T, BYTES>::enqueue(const U& element)
{
    const size_t envelopeSize = calculateEnvelopeSize(element);
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
bool HeterogeneousRingBuffer<T, BYTES>::isEmpty() const
{
    return queuedMessages.getCount() == 0;
}

template <typename T, size_t BYTES>
const T& HeterogeneousRingBuffer<T, BYTES>::dequeue()
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
void HeterogeneousRingBuffer<T, BYTES>::releaseSpace(size_t bytes)
{
    freeSpace.notify(bytes);
}
template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::waitForSpace(size_t bytes)
{
    freeSpace.wait(bytes);
}
template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::notifyNewMessage()
{
    queuedMessages.notify();
}
template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::waitForMessage()
{
    queuedMessages.wait();
}

template <typename T, size_t BYTES>
HeterogeneousRingBuffer<T, BYTES>::Envelope::Envelope(byte* next, T* element) :
    next_(next),
    element_(element)
{
}

template <typename T, size_t BYTES>
bool HeterogeneousRingBuffer<T, BYTES>::Envelope::isNull() const
{
    return element_ == nullptr;
}

template <typename T, size_t BYTES>
template <typename U>
HeterogeneousRingBuffer<T, BYTES>::ElementEnvelope<U>::ElementEnvelope(byte* next, const U& element) :
    Envelope(next, &concreteElement_),
    concreteElement_(element)
{
}

template <typename T, size_t BYTES>
template <typename U>
void HeterogeneousRingBuffer<T, BYTES>::insertElement(const U& element)
{
    byte* next = back_ + calculateEnvelopeSize(element);
    ElementEnvelope<U>* newElement = new (back_) ElementEnvelope<U>(next, element);
    back_ = next;
}

template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::insertPadding()
{
    Envelope* nullElement = new (back_) Envelope(begin_, nullptr);
    back_ = begin_;
}

template <typename T, size_t BYTES>
const typename HeterogeneousRingBuffer<T, BYTES>::Envelope* HeterogeneousRingBuffer<T, BYTES>::readFront()
{
    return reinterpret_cast<Envelope*>(front_);
}

template <typename T, size_t BYTES>
size_t HeterogeneousRingBuffer<T, BYTES>::getPotentialFreeSpaceAtBack() const
{
    return static_cast<size_t>(end_ - back_);
}

} // Data

#endif
