#include <Actions.h>

extern unsigned int total_loop_time;

void Actions::runRecoveryChannelManagerAction(Config &config)
{
  // Check if the recovery channel should be fired
  for (int i = 0; i < 2; i++)
  {
    if (recoveryChannelShouldBeFired[i])
    {
      // If the recovery channel has not been fired yet, enable it
      if (recoveryChannelFireTimes[i] == 0)
      {
        recoveryChannelFireTimes[i] = millis();
        if (i == 0)
        {
          digitalWrite(config.RECOVERY_CHANNEL_1, HIGH);
          Serial.println("Recovery channel 1 fired");
        }
        else if (i == 1)
        {
          digitalWrite(config.RECOVERY_CHANNEL_2, HIGH);
          Serial.println("Recovery channel 2 fired");
        }
      }

      // Check if the recovery channel should be toggled off
      if (millis() - recoveryChannelFireTimes[i] >= config.RECOVERY_CHANNEL_FIRE_TIME)
      {
        // Disable the recovery channel
        if (i == 0)
        {
          if (digitalRead(config.RECOVERY_CHANNEL_SENSE_1) == HIGH)
          {
            Serial.println("Recovery channel 1 was not turned on, trying again");
            digitalWrite(config.RECOVERY_CHANNEL_1, HIGH);
            return;
          }
          else
          {
            Serial.println("Recovery channel 1 has been on for " + String(config.RECOVERY_CHANNEL_FIRE_TIME) + " ms, turning off");
            digitalWrite(config.RECOVERY_CHANNEL_1, LOW);
            Serial.println("Recovery channel 1 turned off");
          }
        }
        else if (i == 1)
        {
          if (digitalRead(config.RECOVERY_CHANNEL_SENSE_2) == HIGH)
          {
            Serial.println("Recovery channel 2 was not turned on, trying again");
            digitalWrite(config.RECOVERY_CHANNEL_2, HIGH);
            return;
          }
          else
          {
            Serial.println("Recovery channel 2 has been on for " + String(config.RECOVERY_CHANNEL_FIRE_TIME) + " ms, turning off");
            digitalWrite(config.RECOVERY_CHANNEL_2, LOW);
            Serial.println("Recovery channel 2 turned off");
          }
        }

        // Reset the recovery channel flag, but keep the fire time as it will not be fired again
        recoveryChannelShouldBeFired[i] = false;
      }
    }
  }
}

void Actions::runDescentAction(Config &config)
{
  // If parachute has been deployed, there is nothing to do
  if (config.config_file_values.parachutes_deployed_flag == 1)
  {
    return;
  }

  // Check if the descent has already been recorded as started
  if (config.config_file_values.descent_flag == 1)
  {
    // Check if the remaining descent time has elapsed
    if (config.config_file_values.remaining_descent_time <= 0)
    {
      // Deploy the parachute
      recoveryChannelShouldBeFired[0] = true;
      recoveryChannelShouldBeFired[1] = true;
      config.config_file_values.parachutes_deployed_flag = 1;
    }
    else
    {
      // Decrease the remaining descent time by the time since the last loop
      config.config_file_values.remaining_descent_time -= total_loop_time;
    }
  }
  else
  {
    // Check if the launch rail switch is off
    if (digitalRead(config.LAUNCH_RAIL_SWITCH_PIN) == LOW)
    {
      if (launchRailSwitchOffTime == 0)
      {
        launchRailSwitchOffTime = millis();
      }

      // Check if the launch rail switch has been off for the threshold time
      if (millis() - launchRailSwitchOffTime >= config.LAUNCH_RAIL_SWITCH_OFF_THRESHOLD)
      {
        // Check if the altitude is above the threshold
        if (altitude > config.LAUNCH_RAIL_SWITCH_ALTITUDE_THRESHOLD)
        {
          return;
        }

        // Record the descent as started
        config.config_file_values.descent_flag = 1;
        config.config_file_values.remaining_descent_time = config.DESCENT_TIME_BEFORE_PARACHUTE_DEPLOYMENT;
      }
    }
    // Reset the launch rail switch off time if the switch is high
    else
    {
      launchRailSwitchOffTime = 0;
    }
  }
}