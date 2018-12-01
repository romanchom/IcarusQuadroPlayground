#pragma once

#include "CircularAllocator.hpp"

#include <mbed.h>
#include <I2C.h>

#include <cstddef>
#include <functional>

namespace util {
template<typename T, size_t SIZE>
class Queue {
public:
    explicit Queue() :
        mTail(mBuffer),
        mAllocatedSize(0u)
    {}

    bool empty() const
    {
        return 0 == mAllocatedSize;
    }

    void push(T value)
    {
        auto head = mTail + mAllocatedSize;
        if (head >= mBuffer + SIZE) {
            head -= SIZE;
        }
        *head = value;
        ++mAllocatedSize;
    }

    T pop()
    {
        T ret = *mTail;
        ++mTail;
        --mAllocatedSize;
        return ret;
    }

    T back()
    {
        return *mTail;
    }
private:
    T mBuffer[SIZE];
    T* mTail;
    size_t mAllocatedSize;
};
}

enum class I2CTransferType : uint8_t
{
    none,
    read,
    write
};

struct I2CTask
{
    std::function<void(I2CTask*)> callback;
    uint8_t size;
    I2CTransferType type;
    uint8_t hostAddress;
    uint8_t registerAddress;
protected:
    explicit I2CTask(uint8_t payloadSize, uint8_t registerAddr) :
        size(payloadSize),
        registerAddress(registerAddr)
    {}
};

template<typename Register>
struct I2CWriteTask : I2CTask
{
    Register data;

    explicit I2CWriteTask() :
        I2CTask(sizeof(Register), Register::address),
        data()
    {}
};

template<size_t MAX_QUEUED_TRANSFERS, size_t BUFFER_SIZE>
class MbedAsyncI2C
{
public:
    explicit MbedAsyncI2C(PinName sda, PinName scl) :
        mDriver(sda, scl)
    {}

    template<typename Register, typename Callback>
    void scheduleWrite(uint8_t hostAddress, Callback const& callback)
    {
        // CriticalSectionLock lock;

        using Task = I2CWriteTask<Register>;
        auto task = mAllocator.template allocate<Task>();
        task->hostAddress = hostAddress;
        task->type = I2CTransferType::write;
        task->callback = [callback](I2CTask* data) {
            auto task = static_cast<Task*>(data);
            callback(task->data);
        };

        bool shouldStart = mQueue.empty();
        mQueue.push(task);

        if(shouldStart) {
            startTask();
        }
    }

        template<typename Register, typename Callback>
    void scheduleRead(uint8_t hostAddress, Callback const& callback)
    {
        // CriticalSectionLock lock;

        using Task = I2CWriteTask<Register>;
        auto task = mAllocator.template allocate<Task>();
        task->hostAddress = hostAddress;
        task->type = I2CTransferType::read;
        task->callback = [callback](I2CTask* data) {
            auto task = static_cast<Task*>(data);
            callback(task->data);
        };

        bool shouldStart = mQueue.empty();
        mQueue.push(task);

        if(shouldStart) {
            startTask();
        }
    }
private:
    void startTask()
    {
        auto task = mQueue.back();
        auto writeStart = reinterpret_cast<char*>(&task->registerAddress);
        constexpr size_t registerAddressSize = 1;
        if (task->type == I2CTransferType::write) {
            mDriver.transfer(task->hostAddress << 1,
                writeStart, registerAddressSize + task->size,
                nullptr, 0,
                event_callback_t(this, &MbedAsyncI2C::transferComplete));
        } else {
            mDriver.transfer(task->hostAddress << 1,
                writeStart, registerAddressSize,
                writeStart + 1, task->size,
                event_callback_t(this, &MbedAsyncI2C::transferComplete));
        }
    }

    void transferComplete(int result)
    {
        auto taskCompleted = mQueue.pop();
        taskCompleted->callback(taskCompleted);
        if (!mQueue.empty()) {
            startTask();
        }
    }

    CircularAllocator<BUFFER_SIZE> mAllocator;
    util::Queue<I2CTask*, MAX_QUEUED_TRANSFERS> mQueue;
    I2C mDriver;
};