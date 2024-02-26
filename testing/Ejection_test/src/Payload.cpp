#include "Payload.h"

void Payload::begin()
{
  // Initialize PC serial
  Serial.begin(config.PC_BAUDRATE);
  while (!Serial)
  {
    delay(100);
  }

  // Enable sensor power
  pinMode(config.SENSOR_POWER_ENABLE_PIN, OUTPUT_12MA);
  digitalWrite(config.SENSOR_POWER_ENABLE_PIN, HIGH);

  // Set the recovery channels to output and pull them low
  pinMode(config.RECOVERY_CHANNEL_1, OUTPUT_12MA);
  pinMode(config.RECOVERY_CHANNEL_2, OUTPUT_12MA);
  digitalWrite(config.RECOVERY_CHANNEL_1, LOW);
  digitalWrite(config.RECOVERY_CHANNEL_2, LOW);

  // Set the buzzer pin to output and pull it low
  pinMode(config.BUZZER_PIN, OUTPUT_12MA);
  digitalWrite(config.BUZZER_PIN, LOW);

  // Set the launch rail switch to input
  pinMode(config.LAUNCH_RAIL_SWITCH_PIN, INPUT);
}