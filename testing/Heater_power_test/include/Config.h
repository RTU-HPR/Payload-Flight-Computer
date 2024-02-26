#pragma once
// Main libraries
#include <Arduino.h>
#include <Wire.h>
#include <Adc_Voltage.h>
#include <Adafruit_INA219.h>

class Config
{
public:
  bool WAIT_PC = false;
  const bool LOG_TO_STORAGE = true;

  // Battery voltage reader
  AdcVoltage::AdcVoltage_Config battery_voltage_reader_config = {
      .pin = 26,                // Taken from the schematic
      .adc_resolution = 4095,   // 12 bit
      .reference_voltage = 3.3, // MCU voltage
      .R1_value = 51000,        // Taken from the schematic
      .R2_value = 24000,        // Taken from the schematic
  };

  // Sensor power
  const int SENSOR_POWER_ENABLE_PIN = 17;

  const int WIRE0_SCL = 1;
  const int WIRE0_SDA = 0;

  // logging
  const int PC_BAUDRATE = 115200;
};