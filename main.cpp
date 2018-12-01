#include <mbed.h>
#include <Semaphore.h>
#include <DigitalOut.h>

#include "MbedAsyncI2C/MbedAsyncI2C.hpp"
#include "MPU-9255-driver/Registers.hpp"


MbedAsyncI2C<4, 64> i2c(PB_9, PB_8);

DigitalOut led1(LED1);

int main() {
    printf("Hello\n\r");
    int i = 0;
    while (true) {
        led1 = !led1;
        printf("\n\n", ++i);
        i2c.schedule<TemperatureRegister>(I2CTransferType::read, 104, [](auto const& data){
            printf("Temperature1 %f\n\r", data.value());
        });
        i2c.schedule<AccelerometerMeasurements>(I2CTransferType::read, 104, [](auto const& data){
            printf("Accelerometer %f %f %f\n\r", data.x(), data.y(), data.z());
        });
        i2c.schedule<GyroscopeMeasurements>(I2CTransferType::read, 104, [](auto const& data){
            printf("Gyroscope %f %f %f\n\r", data.x(), data.y(), data.z());
        });

        wait(0.9);
    }
}
