#pragma once
// Main libraries
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

// Public libraries
#include <PCF8575.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DSL.h>
#include <NTC_Thermistor.h>
#include <Adafruit_BMP085.h>
#include <ClosedCube_STS35.h>
#include <ClosedCube_I2C.h>
#include <SimpleKalmanFilter.h>

// Our sensor libaries
#include <MS56XX.h>
#include <Adc_Voltage.h>

class Config
{
public:
    bool WAIT_PC = false;
    const bool LOG_TO_STORAGE = true;

    // PCF8575 Port extender
    struct PCF8575_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    PCF8575_Config pcf_config = {
        .wire = &Wire1,
        .i2c_address = 0x20,
    };

    // MS56XX
    MS56XX::MS56XX_Config ms56xx_config = {
        .wire = &Wire1,
        .i2c_address = MS56XX::MS56XX_I2C_ADDRESS::I2C_0x76, // or 0x77
        .ms56xx_type = MS56XX::MS56XX_TYPE::MS5611,          // or MS5607
        .oversampling = MS56XX::MS56XX_OVERSAMPLING::OSR_STANDARD,
    };

    // IMU
    struct IMU_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    IMU_Config imu_config = {
        .wire = &Wire1,
        .i2c_address = 0x6B, // or 0x6A
    };

    // Thermistor
    struct Thermistor_Config
    {
        int pin;
        float reference_resistor;
        float nominal_resistor;
        float nominal_temperature;
        float b_coefficient;
        int adc_resolution;
    };

    Thermistor_Config outside_thermistor_config = {
        .pin = 28,
        .reference_resistor = 10000,
        .nominal_resistor = 10000,
        .nominal_temperature = 25,
        .b_coefficient = -4050,
        .adc_resolution = 4095, // 12 bit
    };

    // Battery voltage reader
    AdcVoltage::AdcVoltage_Config battery_voltage_reader_config = {
        .pin = 26,                // Taken from the schematic
        .adc_resolution = 4095,   // 12 bit
        .reference_voltage = 3.3, // MCU voltage
        .R1_value = 51000,        // Taken from the schematic
        .R2_value = 24000,        // Taken from the schematic
    };

    // Container heater voltage reader
    AdcVoltage::AdcVoltage_Config container_heater_voltage_reader_config = {
        .pin = 27,                // Taken from the schematic
        .adc_resolution = 4095,   // 12 bit
        .reference_voltage = 3.3, // MCU voltage
        .R1_value = 51000,        // Taken from the schematic
        .R2_value = 24000,        // Taken from the schematic
    };
    const float HEATER_RESISTOR_VALUE = 1.1;

    // Container barometer
    struct BMP180_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    BMP180_Config BMP180_config = {
        .wire = &Wire1,
        .i2c_address = 0x77, // or 0x76
    };

    // Container temperature sensor
    struct STS35_Config
    {
        TwoWire *wire;
        int i2c_address;
    };

    STS35_Config STS35_config = {
        .wire = &Wire1,
        .i2c_address = 0x4B, // or 0x4A
    };

    // Sensor power
    const int SENSOR_POWER_ENABLE_PIN = 17;

    // Wire0
    const int WIRE0_SCL = 1;
    const int WIRE0_SDA = 0;

    // Wire1
    const int WIRE1_SCL = 15;
    const int WIRE1_SDA = 14;

    // SPI0
    const int SPI0_RX = 4;
    const int SPI0_TX = 7;
    const int SPI0_SCK = 6;

    // Port extender
    const int PORT_EXTENDER_BUZZER_PIN = 1; // Buzzer
    const int PORT_EXTENDER_LED_2_PIN = 2;  // Status LED 2
    const int PORT_EXTENDER_LED_1_PIN = 3;  // Status LED

    // logging
    const int PC_BAUDRATE = 115200;

    // HEATER
    struct Heater_Config
    {
      int heater_pin;
      float Kp;
      float Ki;
      float Kd;
      float Kp_limit;
      float Ki_limit;
      float Kd_limit;
      int pwm_min;
      int pwm_max;
      float target_temp;
    };
    Heater_Config heater_config = {
      .heater_pin = 16,
      .Kp = 10,
      .Ki = 0.000025,
      .Kd = 10000,
      .Kp_limit = 1000,
      .Ki_limit = 1000,
      .Kd_limit = 500,
      .pwm_min = 0,
      .pwm_max = 1000,
      .target_temp = 35,
    };

    // Actions
    const int TIMED_ACTION_INITIAL_DELAY = 10000;
};