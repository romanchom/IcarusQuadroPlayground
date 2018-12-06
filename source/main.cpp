
#include <Sensor.hpp>
#include <mbed.h>
#include <Semaphore.h>
#include <DigitalOut.h>

#include "MbedAsyncI2C/MbedAsyncI2C.hpp"


using I2C_t = MbedAsyncI2C<4, 128>;
MbedAsyncI2C<4, 128> i2c(PB_9, PB_8);

DigitalOut led1(LED1);

int main() {
    printf("Hello\n\r");
    MPU9255::Sensor<double, I2C_t> sensor(i2c);


    while (true) {
        led1 = !led1;
        printf("\n\n");
        
        {
            auto& v = sensor.acceleration();
            printf("Acceleration:            %f, %f, %f [m/s^2]\n\r", v.x(), v.y(), v.z());
        }
        {
            auto& v = sensor.angularVelocity();
            printf("Angular velocity:        %f, %f, %f [rad/s]\n\r", v.x(), v.y(), v.z());
        }
        {
            auto& v = sensor.magneticStrength();
            printf("Magnetic field strength: %f, %f, %f [T]\n\r", v.x(), v.y(), v.z());
        }
        printf("Temperature:             %f [K]\n\r", sensor.temperature());
        wait(1.0);
    }
}
