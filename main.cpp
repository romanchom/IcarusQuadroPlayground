#include <mbed.h>
#include <Semaphore.h>
#include <DigitalOut.h>

#include "MbedAsyncI2C/MbedAsyncI2C.hpp"

class BigEndianShort
{
public:
    uint16_t value() const
    {
        return *this;
    }

    operator uint16_t () const
    {
        return ((highByte) << 8) | lowByte;
    }
private:
    uint16_t highByte : 8;
    uint16_t lowByte : 8;
};

class TemperatureRegister {
public:
    enum { address = 65 };
    float value() const
    {
        return static_cast<float>(mValue.value()) / 333.87f + 21.0f;
    }

private:
    BigEndianShort mValue;
};

MbedAsyncI2C<4, 64> i2c(PB_9, PB_8);

Semaphore readingSem(0, 1);
float temperature;

DigitalOut led1(LED1);

int main() {
    printf("Hello\n\r");
    while (true) {
        led1 = !led1;
        printf("Enter\n\r");
        i2c.scheduleRead<TemperatureRegister>(104, [](TemperatureRegister const& data){
            temperature = data.value();
            readingSem.release();
        });
        readingSem.wait();
        printf("Temperature %f\n\r", temperature);

        wait(0.3);
    }
}
