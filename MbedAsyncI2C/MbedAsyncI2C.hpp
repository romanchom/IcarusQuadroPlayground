#pragma once

#include "CircularAllocator.hpp"

#include <mbed.h>
#include <I2C.h>
#include <Queue.h>
#include <Thread.h>
#include <Semaphore.h>

#include <cstddef>
#include <functional>

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
struct I2CRegisterTask : I2CTask
{
    Register data;

    explicit I2CRegisterTask() :
        I2CTask(sizeof(Register), Register::address),
        data()
    {}
};

template<size_t MAX_QUEUED_TRANSFERS, size_t BUFFER_SIZE>
class MbedAsyncI2C
{
public:
    explicit MbedAsyncI2C(PinName sda, PinName scl) :
        mDriver(sda, scl),
        mTransferSemaphore(0, 1),
        mThread()
    {
        mDriver.frequency(400000);
        mThread.start({this, &MbedAsyncI2C::transferLoop});
    }

    template<typename Register, typename Callback>
    void schedule(I2CTransferType type, uint8_t hostAddress, Callback const& callback)
    {
        using Task = I2CRegisterTask<Register>;
        auto task = mAllocator.template allocate<Task>();
        task->hostAddress = hostAddress;
        task->type = type;
        task->callback = [callback](I2CTask* data) {
            auto const task = static_cast<Task*>(data);
            callback(task->data);
        };

        mQueue.put(task);
    }
private:
    void transferLoop()
    {
        while (true) {
            auto task = reinterpret_cast<I2CTask*>(mQueue.get().value.p);

            char* writeAddress = reinterpret_cast<char*>(&task->registerAddress);
            int writeSize = 1;
            char* readAddress = writeAddress + 1;
            int readSize = 0;
            if (task->type == I2CTransferType::write) {
                writeSize += task->size;
            } else {
                readSize = task->size;
            }

            mDriver.transfer(task->hostAddress << 1,
                writeAddress, writeSize,
                readAddress, readSize,
                event_callback_t(this, &MbedAsyncI2C::transferComplete));
            mTransferSemaphore.wait();

            task->callback(task);
        }
    }

    void transferComplete(int result)
    {
        mTransferSemaphore.release();
    }

    CircularAllocator<BUFFER_SIZE> mAllocator;
    Queue<I2CTask, MAX_QUEUED_TRANSFERS> mQueue;
    I2C mDriver;
    Semaphore mTransferSemaphore;
    Thread mThread;
};