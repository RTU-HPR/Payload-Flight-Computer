#pragma once
// Main libraries
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Public libraries
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

  struct WiFi_Config
  {
    char ssid[32];
    char pass[32];
    unsigned int tmPort;
    unsigned int tcPort;
  };

  WiFi_Config wifi_config{
      "Samsung S20", // Gundars phone hotspot name
      "123456789",   // Gundars phone hotspot password
      10035,         // This ports should not change
      10045          // This ports should not change
  };

  // MS56XX
  MS56XX::MS56XX_Config ms56xx_config = {
      .wire = &Wire,
      .i2c_address = MS56XX::MS56XX_I2C_ADDRESS::I2C_0x76, // or 0x76
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
      .wire = &Wire,
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
      .pin = 27,
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

  // Container barometer
  struct BMP180_Config
  {
    TwoWire *wire;
    int i2c_address;
  };

  BMP180_Config BMP180_config = {
      .wire = &Wire,
      .i2c_address = 0x77, // or 0x77
  };

  // Container temperature sensor
  struct STS35_Config
  {
    TwoWire *wire;
    int i2c_address;
  };

  STS35_Config STS35_config = {
      .wire = &Wire,
      .i2c_address = 0x4B, // or 0x4A
  };

  // Sensor power
  const int SENSOR_POWER_ENABLE_PIN = 17;

  // Wire0
  const int WIRE0_SCL = 1;
  const int WIRE0_SDA = 0;

  // SPI0
  const int SPI0_RX = 4;
  const int SPI0_TX = 3;
  const int SPI0_SCK = 2;

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
      .heater_pin = 22,
      // P = 6.5 I = 0.00002 D = 100000
      // Seems to be good values, but it is slow to reach the target temperature, but it is stable and does not overshoot
      // P = 8 I = 0.000015 D = 120000
      // Seems to be better values, target temperature is reached faster, there is no overshoot, but it doesn't like small target changes
      // P = 7.5 I = 0.000015 D = 140000
      // Horrible values, it constantly overshoots the temperature steps
      // P = 7.5 I = 0.000015 D = 0
      // Not great, overshoots the temperature steps, but it is a bit better than with D = 140000
      // P = 10 I = 0.000008 D = 0
      // Way too slow to reach the temperature steps - inital heating is relatively fast, but then as it approaches the temperature steps, it becomes incredibly slow
      // P = 7 I = 0.000025 D = 0
      // Not great, it overshoots and is slow
      // P = 70 I = 0.00025 D = 0
      // Increased the PWM duty cycle from 1000 to 10000. Stability is increased and seems to work better, but only tested 30 to 35 degrees
      // P = 70 I = 0.00025 D = 50000
      // Worked well, until 30 degrees, then it started to slowly overshoot. The derivative term had close to no effect
      // P = 80 I = 0.00025 D = 500000
      // Overshoots the temperature steps, but the temperature increases nicely
      // P = 60 I = 0.00025 D = 500000
      // Eveything was good until 30 degrees, then it started to overshoot
      // P = 80 I = 0.000075 D = 1500000
      // Multiple changes in the algorithm related to the temperature steps.
      // Great values, very minimal overshoot, temperature increases nicely. Between a critically damped and underdamped system.
      // P = 85 I = 0.00007 D = 1800000
      // Bit more aggressive than the previous values, no overshoot until 34 degrees, then it started to slowly overshoot
      // P = 90 I = 0.00006 D = 1900000
      // From inital impression it looks that the P term should be way higher
      // P = 120 I = 0.00006 D = 1900000
      // Slow to reach step temperature when starting from room temperature. I term values seems to be too low
      // P = 150 I = 0.00006 D = 1900000
      // The same as with P = 120, no noticeable difference
      // P = 150 I = 0.00007 D = 2500000
      // Tested from 20 to 33 degrees, it doesn't want to go any higher than 32.8 degrees, and it is very slow to reach that temperature, but up until like 30 degrees it is very good
      // P = 180 I = 0.000125 D = 2500000
      // The same issue as with P = 150, but a bit better. Also tested from 20 to 33 degrees
      // P = 180 I = 0.0002 D = 2500000
      // Quickly reaches around 33.4 degrees, but then it takes a really long time to continue increasing the temperature. Looks like a higher I term is needed
      // P = 200 I = 0.000225 D = 2500000
      // Works really well. Reached target temp in around 1600 seconds. Started oscilating around target temp, because the heater pwm gets set to zero, when the temperature is above target temp by 0.1 degrees
      // P = 200 I = 0.000235 D = 2500000 Changed the behaviour of the heater pwm, when the temperature is above target temp by 0.1 degrees
      // Worked really well. Reaches target is 1300 seconds. Still there are minimal oscillations around the target temperature, because of the heater pwm behaviour.
      // P = 200 I = 0.000250 D = 2500000 Changed the behaviour of the heater pwm, and chaged the passed in temperature to raw temperature.
      // Absolutely great. The temperature stayed around 0.07 degrees above target.
      // P = 250 I = 0.000250 D = 2500000
      // Works really well. No oscillations. Pretty much non existent overshoot. Reaches target temp in 1300 seconds.
      // P = 300 I = 0.000250 D = 2500000
      // TBD
      .Kp = 300,
      .Ki = 0.000250,
      .Kd = 2500000,
      .Kp_limit = 10000,
      .Ki_limit = 10000,
      .Kd_limit = 10000,
      .pwm_min = 0,
      .pwm_max = 10000,
      .target_temp = 35,
  };
};