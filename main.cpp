#include <mbed.h>
#include <Semaphore.h>
#include <DigitalOut.h>

#include "MbedAsyncI2C/MbedAsyncI2C.hpp"

class BigEndianShort
{
public:
    uint16_t value() const
    {
        return (static_cast<uint16_t>(highByte) << 8) | static_cast<uint16_t>(lowByte);;
    }

    operator uint16_t () const
    {
        return value();
    }
private:
    uint8_t highByte;
    uint8_t lowByte;
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

DigitalOut led1(LED1);

int main() {
    printf("Hello\n\r");
    int i = 0;
    while (true) {
        led1 = !led1;
        printf("Enter %d\n\r", ++i);
        i2c.schedule<TemperatureRegister>(I2CTransferType::read, 104, [](auto const& data){
            printf("Temperature1 %f\n\r", data.value());
        });
        i2c.schedule<TemperatureRegister>(I2CTransferType::read, 104, [](auto const& data){
            printf("Temperature2 %f\n\r", data.value());
        });

        wait(0.3);
    }
}
