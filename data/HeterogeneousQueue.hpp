#pragma once

#include "../common/Semaphore.hpp"
#include <assert.h>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

namespace Data {

/**
 * Dynamically growing queue able to contain heterogeneous elements derived
 * from defined interface @p T.
 *
 * The objects are actually copied to the buffer itself so they need to be
 * copyable or movable.
 *
 * Thread-safe for one reader and one writer.
 *
 * Note that there's overhead for each element in the buffer.
 */
template <typename T>
class HeterogeneousQueue
{
public:
    /**
     * Construct a queue with initial buffer size of @initialSizeInBytes.
     *
     * A new buffer doubling the previous size is allocated whenever
     * the buffer runs out.
     */
    HeterogeneousQueue(size_t initialSizeInBytes);
    ~HeterogeneousQueue();

    /**
     * Push new element of type @p U to the buffer.
     * Will allocate more space if there's not enough to push immediately.
     */
    template <typename U>
    void enqueue(U&& element);

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

    struct Block
    {
        Block(size_t sizeInBytes)
          : sizeInBytes_{sizeInBytes},
            buffer_{std::make_unique<byte[]>(sizeInBytes_)},
            begin_{buffer_.get()},
            end_{begin_ + sizeInBytes_},
            writePosition_{buffer_.get()},
            freeSpace_(sizeInBytes_)
        {
            // std::cout << "New block, size: " << sizeInBytes_ << std::endl;
        }

        ~Block() = default;

        size_t sizeInBytes_;
        std::unique_ptr<byte[]> buffer_;
        byte* begin_;
        byte* end_;
        byte* writePosition_; // write position

        Common::Semaphore freeSpace_;
        void releaseSpace(size_t bytes) { freeSpace_.notify(bytes); }
        void waitForSpace(size_t bytes) { freeSpace_.wait(bytes); }
    };

    std::unique_ptr<Block> writeBlock_; // TODO: consider std::forward_list and a iterator to it
    std::vector<std::unique_ptr<Block>> decayingBlocks_;

    Common::Semaphore queuedMessages_;
    void notifyNewElement();
    void waitForElement();

    struct Envelope
    {
        Envelope(byte* next, const T* element, size_t size, Block* block);
        virtual ~Envelope() = default;
        bool isNull() const;

        const Envelope* next_;
        const T* element_;
        size_t size_;
        Block* block_;
    };

    template <typename U>
    struct ElementEnvelope : public Envelope
    {
        ElementEnvelope(byte* next, U&& element, size_t size, Block* block);
        ~ElementEnvelope() override = default;

        std::remove_reference_t<U> concreteElement_;
    };

    const Envelope* currentEnvelope_;
    bool hasCurrentEnvelope_;

    template <typename U>
    void insertElement(U&& element, size_t envelopeSize);

    void insertPadding(size_t paddingSize);

    size_t getPotentialFreeSpaceAtBack(Block& block) const;

    template <typename U>
    size_t calculateEnvelopeSize(const U& element)
    {
        static const auto maxAlignment = alignof(std::max_align_t);
        const size_t unalignedSize = sizeof(ElementEnvelope<U>);

        const size_t remainder = unalignedSize % maxAlignment;
        if (remainder == 0)
            return unalignedSize;

        return unalignedSize + maxAlignment - remainder;
    }

    bool hasCurrentEnvelope() const { return hasCurrentEnvelope_; }

    bool isCurrentEnvelopePadding() const { return currentEnvelope_->element_ == nullptr; }

    void releaseCurrentEnvelope()
    {
        const Envelope* toBeReleasedEnvelope = currentEnvelope_;
        currentEnvelope_ = currentEnvelope_->next_;

        const size_t size = toBeReleasedEnvelope->size_;
        Block* block = toBeReleasedEnvelope->block_;
        toBeReleasedEnvelope->~Envelope();
        block->releaseSpace(size);
    }

    const T& currentEnvelopedElement() const { return *(currentEnvelope_->element_); }
};

template <typename T>
HeterogeneousQueue<T>::HeterogeneousQueue(size_t initialSizeInBytes)
  : writeBlock_{std::make_unique<Block>(initialSizeInBytes)},
    decayingBlocks_{},
    queuedMessages_{0},
    currentEnvelope_{reinterpret_cast<Envelope*>(writeBlock_->buffer_.get())},
    hasCurrentEnvelope_(false)
{
    // std::cout << "Initial writeBlock_: " << (long long)writeBlock_.get() << std::endl;
    // std::cout << "Initial writeBlock_->begin_: " << (long long)writeBlock_->begin_ << std::endl;
    // std::cout << "Initial writeBlock_->end_: " << (long long)writeBlock_->end_ << std::endl;
    // std::cout << "Initial currentEnvelope_: " << (long long)currentEnvelope_ << std::endl;
}

template <typename T>
HeterogeneousQueue<T>::~HeterogeneousQueue()
{
    while (!isEmpty())
    {
        (void)dequeue();
    }

    // Release the last one, if any
    if (hasCurrentEnvelope_)
    {
        // std::cout << "Releasing last" << std::endl;
        releaseCurrentEnvelope();
    }
}

template <typename T>
template <typename U>
void HeterogeneousQueue<T>::enqueue(U&& element)
{
    const size_t envelopeSize = calculateEnvelopeSize(element);
    const size_t minimumSpaceNeeded = envelopeSize + sizeof(Envelope);

    const size_t potentialSpaceAtBack = getPotentialFreeSpaceAtBack(*writeBlock_);
    assert(potentialSpaceAtBack >= sizeof(Envelope));

    const size_t freeSpaceInBlock = writeBlock_->freeSpace_.getCount();

    // std::cout << "W: Free space in block: " << freeSpaceInBlock << std::endl;

    const bool fitsInBack = potentialSpaceAtBack >= minimumSpaceNeeded && freeSpaceInBlock >= minimumSpaceNeeded;
    const bool fitsInBegin = freeSpaceInBlock >= (potentialSpaceAtBack + minimumSpaceNeeded);
    if (fitsInBack)
    {
        // std::cout << "W: Inserting to back: " << envelopeSize << std::endl;

        writeBlock_->waitForSpace(envelopeSize);
        insertElement(std::forward<U>(element), envelopeSize);
    }
    else if (fitsInBegin)
    {
        // std::cout << "W: Inserting padding: " << potentialSpaceAtBack << std::endl;
        // std::cout << "W: Inserting to begin: " << envelopeSize << std::endl;

        writeBlock_->waitForSpace(potentialSpaceAtBack + envelopeSize);

        // If we are able to fill to the beginning of the block, it means any previous blocks have already
        // been fully dequed, and we can release their memory.
        decayingBlocks_.clear();

        insertPadding(potentialSpaceAtBack); // To fill leftover
        insertElement(std::forward<U>(element), envelopeSize);
    }
    else
    {
        // std::cout << "W: Inserting to new block: " << envelopeSize << std::endl;

        // Does not fit in block, allocate new block and insert there
        auto newWriteBlock = std::make_unique<Block>(writeBlock_->sizeInBytes_ * 2);

        writeBlock_->waitForSpace(sizeof(Envelope));
        (void)new (writeBlock_->writePosition_)
            Envelope(newWriteBlock->begin_, nullptr, sizeof(Envelope), writeBlock_.get());

        decayingBlocks_.push_back(std::move(writeBlock_));
        writeBlock_ = std::move(newWriteBlock);

        writeBlock_->waitForSpace(envelopeSize);
        insertElement(std::forward<U>(element), envelopeSize);
    }

    notifyNewElement();
}

template <typename T>
bool HeterogeneousQueue<T>::isEmpty() const
{
    return queuedMessages_.getCount() == 0;
}

template <typename T>
const T& HeterogeneousQueue<T>::dequeue()
{
    if (hasCurrentEnvelope_)
    {
        // std::cout << "R: Releasing old" << std::endl;
        releaseCurrentEnvelope();
        waitForElement();
    }
    else
    {
        waitForElement();
        hasCurrentEnvelope_ = true;
    }

    while (isCurrentEnvelopePadding())
    {
        // std::cout << "R: Releasing padding" << std::endl;
        // std::cout << "R: Read padding from: " << (long long)currentEnvelope_ << " in block: " << (long
        // long)currentEnvelope_->block_ << std::endl;
        releaseCurrentEnvelope();
    }

    // std::cout << "R: Read element from: " << (long long)currentEnvelope_ << " in block: " << (long
    // long)currentEnvelope_->block_ << std::endl;  std::cout << "R: Free space in block: " <<
    // (int)currentEnvelope_->block_->freeSpace_.getCount() << std::endl;

    return currentEnvelopedElement();
}

template <typename T>
void HeterogeneousQueue<T>::notifyNewElement()
{
    queuedMessages_.notify();
}
template <typename T>
void HeterogeneousQueue<T>::waitForElement()
{
    queuedMessages_.wait();
}

template <typename T>
HeterogeneousQueue<T>::Envelope::Envelope(byte* next, const T* element, size_t size, Block* block)
  : next_(reinterpret_cast<Envelope*>(next)), element_(element), size_(size), block_(block)
{
}

template <typename T>
bool HeterogeneousQueue<T>::Envelope::isNull() const
{
    return element_ == nullptr;
}

template <typename T>
template <typename U>
HeterogeneousQueue<T>::ElementEnvelope<U>::ElementEnvelope(byte* next, U&& element, size_t size, Block* block)
  : Envelope(next, &concreteElement_, size, block), concreteElement_(std::forward<U>(element))
{
    // std::cout << "W: Element address: " << (long long)&concreteElement_ << " in block: " << (long long)block <<
    // std::endl;
}

template <typename T>
template <typename U>
void HeterogeneousQueue<T>::insertElement(U&& element, size_t envelopeSize)
{
    byte* next = writeBlock_->writePosition_ + envelopeSize;

    // std::cout << "W: Inserting element to: " << (long long)writeBlock_->writePosition_ << " in block: " << (long
    // long)writeBlock_.get() << std::endl;

    (void)new (writeBlock_->writePosition_)
        ElementEnvelope<U>(next, std::forward<U>(element), envelopeSize, writeBlock_.get());
    writeBlock_->writePosition_ = next;
}

template <typename T>
void HeterogeneousQueue<T>::insertPadding(size_t paddingSize)
{
    // std::cout << "W: Inserting padding to: " << (long long)writeBlock_->writePosition_ << " in block: " << (long
    // long)writeBlock_.get() << std::endl;

    (void)new (writeBlock_->writePosition_) Envelope(writeBlock_->begin_, nullptr, paddingSize, writeBlock_.get());
    writeBlock_->writePosition_ = writeBlock_->begin_;
}

template <typename T>
size_t HeterogeneousQueue<T>::getPotentialFreeSpaceAtBack(Block& block) const
{
    return static_cast<size_t>(block.end_ - block.writePosition_);
}

} // namespace Data
