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
    /** Construct a HeterogeneousRingBuffer */
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

    /** Actual buffer and pointers to it */
    ///@{
    byte buffer_[BYTES];
    byte* begin_;
    byte* end_;
    byte* writePosition_;
    ///@}

    /** Counter for free space in the buffer */
    Common::Semaphore freeSpace_;
    void releaseSpace(size_t bytes);
    void waitForSpace(size_t bytes);

    /** Counter for elements in the queue */
    Common::Semaphore queuedElements_;
    void notifyNewElement();
    void waitForElement();

    /** Wrapper for a queue element. Keeps up a single-linked list of wrappers in the queue. */
    struct Envelope
    {
        Envelope(byte* next, T* element);
        const Envelope* next_;  ///< Future position of next wrapper
        T* element_;            ///< Wrapped element
    };
    /** Template to allow heterogeneous elements */
    template <typename U> struct ElementEnvelope : public Envelope
    {
        ElementEnvelope(byte* next, const U& element);
        U concreteElement_;
    };

    /**
     * The envelope we are currently reading and the methods
     * to query and modify it.
     *
     * Initially there's no current envelope. After first read
     * it is initialized and there after is will always point to
     * the current element. While the reading blocks waiting for a
     * new element the envelope is undefined.
     *
     * @see dequeue
     */
    ///@{
    const Envelope* currentEnvelope;

    bool hasCurrentEnvelope() const;
    bool isCurrentEnvelopePadding() const;
    void initializeCurrentEnvelope();
    void releaseCurrentEnvelope();
    size_t calculateCurrentEnvelopeSize() const;
    const T& currentEnvelopedElement() const;
    ///@}

    /**
     * Methods for writing new elements and determining a proper
     * position for them in the buffer.
     *
     * @see enqueue
     */
    ///@{
    size_t getPotentialFreeSpaceAtBack() const;
    void insertPadding();
    template <typename U> void insertElement(const U& element);
    template <typename U> size_t calculateEnvelopeSize(const U& element);
    ///@}
};

template <typename T, size_t BYTES>
HeterogeneousRingBuffer<T, BYTES>::HeterogeneousRingBuffer() :
    buffer_(),
    begin_(buffer_),
    end_(buffer_ + BYTES),
    writePosition_(buffer_),
    freeSpace_(BYTES),
    queuedElements_(0),
    currentEnvelope(nullptr)
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

    notifyNewElement();
}

template <typename T, size_t BYTES>
bool HeterogeneousRingBuffer<T, BYTES>::isEmpty() const
{
    return queuedElements_.getCount() == 0;
}

template <typename T, size_t BYTES>
const T& HeterogeneousRingBuffer<T, BYTES>::dequeue()
{
    if (hasCurrentEnvelope())
    {
        releaseCurrentEnvelope();
    }

    waitForElement();

    if (!hasCurrentEnvelope())
    {
        initializeCurrentEnvelope();
    }

    while (isCurrentEnvelopePadding())
    {
        releaseCurrentEnvelope();
    }

    return currentEnvelopedElement();
}

template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::releaseSpace(size_t bytes)
{
    freeSpace_.notify(bytes);
}
template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::waitForSpace(size_t bytes)
{
    freeSpace_.wait(bytes);
}
template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::notifyNewElement()
{
    queuedElements_.notify();
}
template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::waitForElement()
{
    queuedElements_.wait();
}

template <typename T, size_t BYTES>
HeterogeneousRingBuffer<T, BYTES>::Envelope::Envelope(byte* next, T* element) :
    next_(reinterpret_cast<Envelope*>(next)),
    element_(element)
{
}

template <typename T, size_t BYTES>
template <typename U>
HeterogeneousRingBuffer<T, BYTES>::ElementEnvelope<U>::ElementEnvelope(byte* next, const U& element) :
    Envelope(next, &concreteElement_),
    concreteElement_(element)
{
}

template <typename T, size_t BYTES>
bool HeterogeneousRingBuffer<T, BYTES>::hasCurrentEnvelope() const
{
    return currentEnvelope != nullptr;
}

template <typename T, size_t BYTES>
bool HeterogeneousRingBuffer<T, BYTES>::isCurrentEnvelopePadding() const
{
    return currentEnvelope->element_ == nullptr;
}

template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::initializeCurrentEnvelope()
{
    currentEnvelope = reinterpret_cast<Envelope*>(begin_);
}

template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::releaseCurrentEnvelope()
{
    const size_t size = calculateCurrentEnvelopeSize();
    currentEnvelope = currentEnvelope->next_;
    releaseSpace(size);
}

template <typename T, size_t BYTES>
size_t HeterogeneousRingBuffer<T, BYTES>::calculateCurrentEnvelopeSize() const
{
    const byte* self = reinterpret_cast<const byte*>(currentEnvelope);
    const byte* next = reinterpret_cast<const byte*>(currentEnvelope->next_);

    return next > self ? next - self : end_ - self;
}

template <typename T, size_t BYTES>
const T& HeterogeneousRingBuffer<T, BYTES>::currentEnvelopedElement() const
{
    return *currentEnvelope->element_;
}

template <typename T, size_t BYTES>
size_t HeterogeneousRingBuffer<T, BYTES>::getPotentialFreeSpaceAtBack() const
{
    return static_cast<size_t>(end_ - writePosition_);
}

template <typename T, size_t BYTES>
void HeterogeneousRingBuffer<T, BYTES>::insertPadding()
{
    Envelope* nullElement = new (writePosition_) Envelope(begin_, nullptr);
    writePosition_ = begin_;
}

template <typename T, size_t BYTES>
template <typename U>
void HeterogeneousRingBuffer<T, BYTES>::insertElement(const U& element)
{
    byte* next = writePosition_ + calculateEnvelopeSize(element);
    ElementEnvelope<U>* newElement = new (writePosition_) ElementEnvelope<U>(next, element);
    writePosition_ = next;
}

template <typename T, size_t BYTES>
template <typename U>
size_t HeterogeneousRingBuffer<T, BYTES>::calculateEnvelopeSize(const U& element)
{
    static const auto maxAlignment = alignof(std::max_align_t);
    const size_t unalignedSize = sizeof(ElementEnvelope<U>);

    size_t remainder = unalignedSize % maxAlignment;
    if (remainder == 0)
        return unalignedSize;

    return unalignedSize + maxAlignment - remainder;
}

} // Data

#endif
