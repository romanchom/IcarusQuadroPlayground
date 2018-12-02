#include <mbed.h>
#include <Semaphore.h>
#include <DigitalOut.h>

#include "MbedAsyncI2C/MbedAsyncI2C.hpp"
#include "MPU-9255-driver/Registers.hpp"


MbedAsyncI2C<4, 128> i2c(PB_9, PB_8);

DigitalOut led1(LED1);

constexpr uint8_t agAddress = 104;
constexpr uint8_t magAddress = 12;

namespace AG = MPU9255::AccelerometerGyroscope;
namespace Mag = MPU9255::Magnetometer;

int main() {
    printf("Hello\n\r");

    // enable direct i2c access to magnetometer
    AG::InterruptBypassConfiguration config = {};
    config.enableBypass = true;
    i2c.scheduleWrite(agAddress, config, [](){});
    wait(0.1);

    // enable continous measurement
    Mag::Control1 control = {};
    control.mode = Mag::OperationMode::continousMeasurement2;
    i2c.scheduleWrite(magAddress, control, [](){
        printf("Magnetometer init\n\r");
    });

    AG::AccelerometerConfiguration1 accConfig = {};
    accConfig.fullScaleSelect = AG::AccelerometerRange::g4;
    i2c.scheduleWrite(agAddress, accConfig, [](){});

    wait(0.1);

    while (true) {
        led1 = !led1;
        printf("\n\n");
        i2c.scheduleRead<AG::TemperatureRegister>(agAddress, [](auto const& data){
            printf("Temperature1 %d\n\r", data.value());
        });
        i2c.scheduleRead<AG::AccelerometerMeasurements>(agAddress, [](auto const& data){
            printf("Accelerometer %d %d %d\n\r", data.x(), data.y(), data.z());
        });
        i2c.scheduleRead<AG::GyroscopeMeasurements>(agAddress, [](auto const& data){
            printf("Gyroscope %d %d %d\n\r", data.x(), data.y(), data.z());
        });
        i2c.scheduleRead<Mag::Measurements>(magAddress, [](auto const& data){
            printf("Magnetometer %d %d %d\n\r", data.x(), data.y(), data.z());
        });
        wait(1.0);
    }
}
