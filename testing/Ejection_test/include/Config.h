#pragma once
// Main libraries
#include <Arduino.h>

class Config
{
public:
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
  // Sensor power
  const int SENSOR_POWER_ENABLE_PIN = 17;

  // logging
  const int PC_BAUDRATE = 115200;

  // Parachute
  const int RECOVERY_CHANNEL_1 = 21;
  const int RECOVERY_CHANNEL_2 = 20;
  const int RECOVERY_CHANNEL_SENSE_1 = 19;
  const int RECOVERY_CHANNEL_SENSE_2 = 18;
  const int RECOVERY_CHANNEL_FIRE_TIME = 5000;

  const int LAUNCH_RAIL_SWITCH_PIN = 10;
  const int LAUNCH_RAIL_SWITCH_OFF_THRESHOLD = 5000;
  const int DESCENT_TIME_BEFORE_PARACHUTE_DEPLOYMENT = 30000;
  const int LAUNCH_RAIL_SWITCH_ALTITUDE_THRESHOLD = 300;

  // Buzzer
  const int BUZZER_PIN = 16;
  const int BUZZER_BEEP_TIME = 2000;
  const int BUZZER_ACTION_START_TIME = 7200 * 1000; // 7200 seconds after turning on == 2 hours
};