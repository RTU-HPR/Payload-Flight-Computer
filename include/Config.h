#pragma once
// Main libraries
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LittleFS.h>
#include <SDFS.h>
#include <cppQueue.h>
#include <Servo.h>


// Public libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DSL.h>
#include <NTC_Thermistor.h>
#include <Adafruit_BMP085.h>
#include <ClosedCube_STS35.h>
#include <ClosedCube_I2C.h>
#include <SimpleKalmanFilter.h>
#include "Adafruit_MCP9808.h"

// Our wrappers
#include <RadioLib_wrapper.h>
#include <ranging_wrapper.h>
#include <Gps_Wrapper.h>
#include <Sd_card_wrapper.h>

// Our functions
#include <Ccsds_packets.h>

// Our sensor libaries
#include <MS56XX.h>
#include <Adc_Voltage.h>

// Used radio module
#define radio_module SX1268

class Config
{
public:
  bool WAIT_PC = false;
  const bool LOG_TO_STORAGE = true;

  // 433 MHz LoRa
  RadioLib_Wrapper<radio_module>::Radio_Config radio_config{
      .frequency = 434.5,
      .cs = 5,
      .dio0 = 8,
      .dio1 = 9,
      .family = RadioLib_Wrapper<radio_module>::Radio_Config::Chip_Family::Sx126x,
      .rf_switching = RadioLib_Wrapper<radio_module>::Radio_Config::Rf_Switching::Dio2,
      // .rx_enable = 0, // only needed if rf_switching = gpio
      // .tx_enable = 0, // only needed if rf_switching = gpio
      .reset = 7,
      .sync_word = 0xF4,
      .tx_power = 22,
      .spreading = 11,
      .coding_rate = 8,
      .signal_bw = 62.5,
      .frequency_correction = false,
      .spi_bus = &SPI,
  };

  // Ranging 2.4 GHZ LoRa
  Ranging_Wrapper::Ranging_Slave RANGING_SLAVES[3] =
      {
          {.position = {0, 0, 0}, .address = 0x12345678}, // Recovery station
          {.position = {0, 0, 0}, .address = 0xABCD9876}, // Secondary receiver on rotator
          {.position = {0, 0, 0}, .address = 0xABCD9876}, // Secondary receiver on rotator
  };
  Ranging_Wrapper::Mode ranging_mode = Ranging_Wrapper::Mode::MASTER;
  const int RANGING_LORA_TIMEOUT = 200;
  Ranging_Wrapper::Lora_Device ranging_device =
      {
          .FREQUENCY = 2405.6,
          .CS = 15,
          .DIO0 = 11, // busy
          .DIO1 = 12,
          .RESET = 13,
          .SYNC_WORD = 0xF5,
          .TXPOWER = 14,
          .SPREADING = 10,
          .CODING_RATE = 7,
          .SIGNAL_BW = 406.25,
          .SPI = &SPI,
  };

  // GPS
  Gps_Wrapper::Gps_Config_I2C gps_config{
      .config = {
          .timeout = 5000,                       // Time it takes for anything to timeout
          .measurement_rate = 500,               // how often measurement will be taken in ms
          .navigation_frequency = 2,             // how often tu updated navigation in s
          .dynamic_model = DYN_MODEL_AIRBORNE2g, // DYN_MODEL_AIRBORNE2g
          .com_settings = COM_TYPE_UBX,          // COM_TYPE_UBX
          .auto_pvt = true                       // for neo6m dont use this
      },
      .wire = &Wire,
      .i2c_address = 0x42, // Default
  };

  // SD card
  String TELMETRY_FILE_HEADER = "index,time_on_ms,gps_epoch_time,gps_hour:gps_minute:gps_second,gps_lat,gps_lng,gps_altitude,gps_speed,gps_satellites,gps_heading,gps_pdop,container_temp,container_temp_filtered,container_baro_temp,container_baro_pressure,mcp9808_temperature,heater_enabled,heater_current_temp_step,heater_target_temp,heater_pwm,heater_pid_p,heater_pid_i,heater_pid_d,onboard_baro_temp,onboard_baro_pressure,onboard_baro_altitude,outside_thermistor_temp,imu_accel_x,imu_accel_y,imu_accel_z,imu_heading,imu_pitch,imu_roll,imu_gyro_x,imu_gyro_y,imu_gyro_z,imu_temp,ranging_0_distance,ranging_0_f_error,ranging_0_rssi,ranging_0_snr,ranging_0_time,ranging_1_distance,ranging_1_f_error,ranging_1_rssi,ranging_1_snr,ranging_1_time,ranging_2_distance,ranging_2_f_error,ranging_2_rssi,ranging_2_snr,ranging_2_time,ranging_position_lat,ranging_position_lng,ranging_position_height,battery_voltage,used_heap,loop_time,continuous_actions_time,timed_actions_time,requested_actions_time,gps_read_time,logging_time,sensor_read_time,on_board_baro_read_time,imu_read_time,battery_voltage_read_time,container_baro_read_time,container_temperature_read_time,outside_thermistor_read_time";
  String INFO_FILE_HEADER = "time,info";
  String ERROR_FILE_HEADER = "time,error";
  String CONFIG_FILE_HEADER = "descent_flag,remaining_descent_time,parachutes_deployed_flag";

  struct Config_File_Values
  {
    int descent_flag;
    long remaining_descent_time;
    int parachutes_deployed_flag;
  };

  Config_File_Values config_file_values = {
      .descent_flag = 0,
      .remaining_descent_time = DESCENT_TIME_BEFORE_PARACHUTE_DEPLOYMENT,
      .parachutes_deployed_flag = 0,
  };

  SD_Card_Wrapper::Config sd_card_config = {
      // spi bus
      .spi_bus = &SPI,
      .cs_pin = 14,
      .data_file_path_base = "/PFC_TELEMETRY_",
      .info_file_path_base = "/PFC_INFO_",
      .error_file_path_base = "/PFC_ERROR_",
      .config_file_path = "/PFC_CONFIG",
      .data_file_header = TELMETRY_FILE_HEADER,
      .info_file_header = INFO_FILE_HEADER,
      .error_file_header = ERROR_FILE_HEADER,
      .config_file_header = CONFIG_FILE_HEADER,
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

// Info/error queue
#define QUEUE_IMPLEMENTATION FIFO
#define INFO_ERROR_QUEUE_SIZE 20
#define INFO_ERROR_MAX_LENGTH 100

  // Watchdog
  const int WATCHDOG_TIMER = 8000; // Max is 8400

  // Sensor power
  const int SENSOR_POWER_ENABLE_PIN = 17;

  // Battery
  const float BATTERY_LOW_VOLTAGE = 5.5;
  const int BATTERY_LOW_BEEP_INTERVAL = 200;

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
      .Kp = 800,
      .Ki = 0.0004,
      .Kd = 0,
      .Kp_limit = 10000,
      .Ki_limit = 10000,
      .Kd_limit = 10000,
      .pwm_min = 0,
      .pwm_max = 10000,
      .target_temp = 35,
  };

  // Parachute
  const int RECOVERY_CHANNEL_1 = 21;
  const int RECOVERY_CHANNEL_2 = 20;
  const int RECOVERY_CHANNEL_SENSE_1 = 19;
  const int RECOVERY_CHANNEL_SENSE_2 = 18;
  const int RECOVERY_CHANNEL_FIRE_TIME = 30000;

  // Servos
  const int SERVO_INITIAL_POSITION = 75;
  const int SERVO_FINAL_POSITION = 0;

  const int LAUNCH_RAIL_SWITCH_PIN = 10;
  const int LAUNCH_RAIL_SWITCH_OFF_THRESHOLD = 5000;
  const int DESCENT_TIME_BEFORE_PARACHUTE_DEPLOYMENT = 30000;
  const int LAUNCH_RAIL_SWITCH_ALTITUDE_THRESHOLD = 300;

  // Buzzer
  const int BUZZER_PIN = 16;
  const int BUZZER_BEEP_TIME = 2000;
  const int BUZZER_ACTION_START_TIME = 7200 * 1000; // 7200 seconds after turning on == 2 hours

  // Actions
  const int TIMED_ACTION_INITIAL_DELAY = 10000;
  // Data send action interval
  // 15 second cycle
  // 0 to 4 seconds - listen for command
  // 4 to 9 seconds - send command response if required
  // 9 to 12 seconds - balloon sends essential data
  // 12 to 15 seconds - send essential data
  const int COMMUNICATION_CYCLE_INTERVAL = 15; // Seconds
  const int COMMUNICATION_RESPONSE_SEND_TIME_START = 4000;
  const int COMMUNICATION_RESPONSE_SEND_TIME_END = 9000;
  const int COMMUNICATION_ESSENTIAL_DATA_SEND_TIME_START = 12000;
  const int COMMUNICATION_ESSENTIAL_DATA_SEND_TIME_END = 15000;

  // Sendable commands
  const int PFC_ESSENTIAL_DATA_RESPONSE = 100;
  const int PFC_COMPLETE_DATA_RESPONSE = 101;
  const int PFC_INFO_ERROR_RESPONSE = 102;
  const int PFC_FORMAT_RESPONSE = 103;
  const int PFC_RECOVERY_RESPONSE = 104;

  // Receiveable commands
  const int PFC_COMPLETE_DATA_REQUEST = 1000;
  const int PFC_INFO_ERROR_REQUEST = 1001;
  const int PFC_FORMAT_REQUEST = 1002;
  const int PFC_RECOVERY_REQUEST = 1003;
  const int RESET_SERVO_POSITION_REQUEST = 1005;
};