#include <Actions.h>
#include <Continous_Actions.h>
#include <Timed_Actions.h>

unsigned int last_print_time = 0;

void Actions::runAllActions(Config &config)
{
  // Check if a manual parachute deployment has been requested or the altitude has been updated
  if (Serial.available() > 0)
  {
    String msg = Serial.readString();
    if (msg == "deploy")
    {
      config.config_file_values.parachutes_deployed_flag = 1;
    }
    else if (msg.substring(0, 3) == "ALT")
    {
      altitude = msg.substring(3).toFloat();
    }
    else
    {
      Serial.println("Invalid command");
    }
  }

  // Check if the launch rail switch has been turned off and the descent has started
  runDescentAction(config);

  // Check if the recovery channel should be fired
  runRecoveryChannelManagerAction(config);

  // Check if the buzzer should be turned on
  runBuzzerAction(config);

  // Print general information every 2 seconds
  if (millis() - last_print_time > 5000)
  {
    last_print_time = millis();
    Serial.println("Current millis: " + String(millis()) + " ms");
    Serial.println("Altitude: " + String(altitude) + " m");
    Serial.println("Descent flag: " + String(config.config_file_values.descent_flag));
    Serial.println("Remaining descent time: " + String(config.config_file_values.remaining_descent_time) + " ms");
    Serial.println("Parachutes deployed flag: " + String(config.config_file_values.parachutes_deployed_flag));
    Serial.println("Launch rail switch state: " + String(digitalRead(config.LAUNCH_RAIL_SWITCH_PIN)));
    Serial.println("Launch rail switch off time: " + String(config.LAUNCH_RAIL_SWITCH_OFF_THRESHOLD) + " ms");
    Serial.println("Recovery channel 1 should be fired: " + String(recoveryChannelShouldBeFired[0]));
    Serial.println("Recovery channel 2 should be fired: " + String(recoveryChannelShouldBeFired[1]));
    Serial.println("Recovery channel 1 fire time: " + String(config.RECOVERY_CHANNEL_FIRE_TIME) + " ms");
    Serial.println("Recovery channel 2 fire time: " + String(config.RECOVERY_CHANNEL_FIRE_TIME) + " ms");
    Serial.println("Total loop time: " + String(total_loop_time) + " ms");

    last_print_time = millis();
  }
}
