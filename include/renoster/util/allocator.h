#pragma once

#ifndef RENOSTER_UTIL_ALLOCATOR_H_
#define RENOSTER_UTIL_ALLOCATOR_H_

#include <cstdlib>
#include <list>
#include <memory>

namespace renoster {

class Allocator {
public:
    static constexpr size_t MaxAlignment = 256;

    Allocator(size_t minBlockSize = 4096)
        : _minBlockSize(minBlockSize) {}

    ~Allocator();

    void * Alloc(size_t numBytes, size_t alignment = 1);

    template <typename T, typename ... Args>
    T * New(Args && ... args);

    void Reset();

private:
    class Block {
    public:
        Block(size_t numBytes)
            : _bytesUsed(0), _numBytes(numBytes)
        {
            _bytes = static_cast<uint8_t *>(
                    std::aligned_alloc(Allocator::MaxAlignment, numBytes));
        }

        void * Alloc(size_t bytes, size_t alignment);

        void Reset()
        {
            _bytesUsed = 0;
        }

        void Free()
        {
            std::free(_bytes);
            _bytesUsed = 0;
            _numBytes = 0;
        }

    private:
        size_t _bytesUsed;
        size_t _numBytes;
        uint8_t * _bytes = nullptr;
    };

    const size_t _minBlockSize;

    std::list<Block> _usedBlocks;
    std::list<Block> _availableBlocks;
};

inline Allocator::~Allocator()
{
    for (Block & block : _usedBlocks) {
        block.Free();
    }
    _usedBlocks.clear();

    for (Block & block : _availableBlocks) {
        block.Free();
    }
    _availableBlocks.clear();
}

inline void * Allocator::Block::Alloc(size_t numBytes, size_t alignment)
{
    uintptr_t bytesPtr = reinterpret_cast<uintptr_t>(_bytes);
    uintptr_t unusedPtr = bytesPtr + _bytesUsed;
    uintptr_t alignedPtr = (unusedPtr + alignment - 1) & ~(alignment - 1);
    uintptr_t endPtr = alignedPtr + numBytes;

    if (endPtr <= bytesPtr + _numBytes) {
        _bytesUsed = endPtr - bytesPtr;
        return reinterpret_cast<void *>(alignedPtr);
    }

    return nullptr;
}

inline void * Allocator::Alloc(size_t numBytes, size_t alignment) {
    // Use the current block
    if (!_usedBlocks.empty()) {
        void * ptr = _usedBlocks.begin()->Alloc(numBytes, alignment);
        if (ptr) {
            return ptr;
        }
    }

    // Search the available blocks
    for (auto it = _availableBlocks.begin(); it != _availableBlocks.end(); ++it) {
        void * ptr = it->Alloc(numBytes, alignment);
        if (ptr) {
            _usedBlocks.push_front(*it);
            _availableBlocks.erase(it);
            return ptr;
        }
    }
    
    // Allocate a new block
    size_t numBytesBlock = std::max(_minBlockSize, numBytes);
    _usedBlocks.emplace_front(numBytesBlock);
    return _usedBlocks.begin()->Alloc(numBytes, alignment);
}

template <typename T, typename ... Args>
T * Allocator::New(Args && ... args) {
    void * bytes = Alloc(sizeof(T), alignof(T));
    return new (bytes) T(args...);
}

inline void Allocator::Reset() {
    for (auto & block : _usedBlocks) {
        block.Reset();
    }

    _availableBlocks.splice(_availableBlocks.begin(), _usedBlocks);
}


} // namespace renoster

#endif // RENOSTER_UTIL_ALLOCATOR_H_
