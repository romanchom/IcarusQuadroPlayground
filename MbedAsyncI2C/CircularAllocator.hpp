#include <cstdint>
#include <cstddef>

template<size_t SIZE>
class CircularAllocator
{
public:
    explicit CircularAllocator() :
        mHead(mBuffer)
    {}

    template<typename T>
    T* allocate()
    {
        auto pointer = allocate(sizeof(T));
        return new (pointer) T();
    }

    uint8_t* allocate(size_t size)
    {
        uint8_t* ret;
        if (mHead + size <= mBuffer + SIZE) {
            ret = mHead;
            mHead += size;
        } else {
            ret = mBuffer;
            mHead = mBuffer + size;
        }
        return ret;
    }
private:
    uint8_t mBuffer[SIZE];
    uint8_t* mHead;
};
