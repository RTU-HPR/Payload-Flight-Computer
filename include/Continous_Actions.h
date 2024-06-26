#include <Actions.h>

// Performance monitoring
unsigned long last_sensor_read_millis = 0;
unsigned long last_gps_read_millis = 0;
unsigned long last_logging_millis = 0;

// Get Servo objects
extern Servo servo_2;

void Actions::runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config)
{
  // Receive any commands
  if (commandReceiveActionEnabled)
  {
    runCommandReceiveAction(communication, logging, config);
  }

  // Check if the communication cycle should be started
  if (getCommunicationCycleStartActionEnabled)
  {
    runGetCommunicationCycleStartAction(navigation, config);
  }

  // Run the sensor action
  if (sensorActionEnabled)
  {
    last_sensor_read_millis = millis();
    runSensorAction(sensors);
    sensor_read_time = millis() - last_sensor_read_millis;
  }

  // Run the heater action
  if (heaterActionEnabled)
  {
    runHeaterAction(heater, sensors);
  }

  // Run the GPS action
  if (gpsActionEnabled)
  {
    last_gps_read_millis = millis();
    runGpsAction(navigation);
    gps_read_time = millis() - last_gps_read_millis;
  }

  // Run the ranging action
  if (rangingSendActionEnabled)
  {
    runRangingAction(navigation, config);
  }

  // Run the logging action
  if (loggingActionEnabled)
  {
    last_logging_millis = millis();
    runLoggingAction(logging, navigation, sensors, heater, config);
    logging_time = millis() - last_logging_millis;
    if (loggable_packed_id % 50 == 0)
    {
      Serial.println("Logged packet count: " + String(loggable_packed_id) + " | Turned on time: " + String(millis() / 1000) + " s");
    }
  }

  if (descentActionEnabled)
  {
    runDescentAction(logging, config, sensors, navigation);
  }

  // Run the recovery channel manager action
  if (recoveryChannelManagerActionEnabled)
  {
    runRecoveryChannelManagerAction(config);
  }

  // Check the battery voltage
  if (batteryVoltageCheckEnabled)
  {
    checkBatteryVoltage(sensors, logging, config);
  }
}

void Actions::runCommandReceiveAction(Communication &communication, Logging &logging, Config &config)
{
  byte *msg = new byte[256];
  uint16_t msg_length = 0;
  float rssi = 1;
  float snr = 0;
  double frequency = 0;
  bool checksum_good = false;

  // Check for any messages from Radio
  if (communication._radio->receive_bytes(msg, msg_length, rssi, snr, frequency))
  {
    // Check if checksum matches
    if (check_crc_16_cciit_of_ccsds_packet(msg, msg_length))
    {
      checksum_good = true;
    }
  }
  else
  {
    // Free memory after no message has been received
    delete[] msg; // VERY IMPORTANT, otherwise a significant memory leak will occur
    return;
  }

  // Check if the checksum is good
  if (checksum_good)
  {
    // Print the received message
    Serial.print("RADIO COMMAND | RSSI: " + String(rssi) + " | SNR: " + String(snr) + " FREQUENCY: " + String(frequency, 8) + " | MSG: ");

    for (int i = 0; i < msg_length; i++)
    {
      Serial.print(msg[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // A CCSDS Telecommand packet was received
    uint16_t apid = 0;
    uint16_t sequence_count = 0;
    byte *packet_data = new byte[msg_length];
    uint16_t packet_id = 0;
    uint16_t packet_data_length = 0;
    parse_ccsds_telecommand(msg, apid, sequence_count, packet_id, packet_data, packet_data_length);

    Serial.println("APID: " + String(apid));
    Serial.println("Sequence count: " + String(sequence_count));
    Serial.println("Packet data length: " + String(packet_data_length));
    Serial.println("Packet ID: " + String(packet_id));

    // Set the action flag according to the received command
    if (packet_id == config.PFC_COMPLETE_DATA_REQUEST)
    {
      completeDataRequestActionEnabled = true;
    }
    else if (packet_id == config.PFC_INFO_ERROR_REQUEST)
    {
      infoErrorRequestActionEnabled = true;
    }
    else if (packet_id == config.PFC_FORMAT_REQUEST)
    {
      formatStorageActionEnabled = true;
    }
    else if (packet_id == config.RESET_SERVO_POSITION_REQUEST)
    {
      servo_2.write(config.SERVO_INITIAL_POSITION);
    }
    else if (packet_id == config.PFC_RECOVERY_REQUEST)
    {
      recoveryFireActionEnabled = true;

      // Get the recovery channel
      Converter recoveryChannel[1];

      // The first value is the packet id, and the second is the recovery channel
      extract_ccsds_data_values(packet_data, recoveryChannel, "uint8");

      // Set the appropriate recovery channel flag
      if (recoveryChannel[0].i8 == 1)
      {
        recoveryChannelShouldBeFired[0] = true;
      }
      else if (recoveryChannel[0].i8 == 2)
      {
        recoveryChannelShouldBeFired[1] = true;
      }
      else
      {
        Serial.println("Invalid recovery channel: " + String(recoveryChannel[0].i8));
        recoveryFireActionEnabled = false;
      }
    }
    else
    {
      Serial.println("No mathcing command found");
    }

    // Free memory after the packet data has been parsed
    delete[] packet_data; // VERY IMPORTANT, otherwise a significant memory leak will occur
  }
  else if (!checksum_good)
  {
    Serial.println("Command with invalid checksum received: ");
    for (int i = 0; i < msg_length; i++)
    {
      Serial.print(msg[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }

  // Free memory after the message has been parsed
  delete[] msg; // VERY IMPORTANT, otherwise a significant memory leak will occur
}

void Actions::runSensorAction(Sensors &sensors)
{
  // Read all sensors
  sensors.readSensors();
}

void Actions::runHeaterAction(Heater &heater, const Sensors &sensors)
{
  // Update the heater
  heater.update(sensors._mcptemp);
}

void Actions::runGpsAction(Navigation &navigation)
{
  navigation.readGps(navigation.navigation_data);
}

void Actions::runLoggingAction(Logging &logging, Navigation &navigation, Sensors &sensors, Heater &heater, Config &config)
{
  // Log the data to the sd card
  String packet = createLoggablePacket(sensors, heater, navigation, config);
  logging.writeTelemetry(packet);
}

void Actions::runRangingAction(Navigation &navigation, Config &config)
{
  navigation.readRanging(config, navigation.navigation_data);
}

void Actions::runGetCommunicationCycleStartAction(Navigation &navigation, Config &config)
{
  // If there is no GPS time, start a backup communication cycle
  if (navigation.navigation_data.gps.epoch_time == 0)
  {
    if (millis() > 10000 && lastCommunicationCycle == 0)
    {
      lastCommunicationCycle = millis();
      dataEssentialSendActionEnabled = true;
    }
    else if (millis() - lastCommunicationCycle >= 30000)
    {
      lastCommunicationCycle = millis();
      dataEssentialSendActionEnabled = true;
      Serial.println("NO GPS TIME! Backup communication cycle started: " + String(lastCommunicationCycle));
    }
    return;
  }

  // If the last communication cycle was less than 3 seconds ago, do not check for a new cycle
  if (millis() - lastCommunicationCycle <= 3000)
  {
    return;
  }

  // Check if the cycle hasn't gone on for too long i.e. GPS time is stuck
  if (millis() - lastCommunicationCycle >= config.COMMUNICATION_CYCLE_INTERVAL * 1000 * 2)
  {
    lastCommunicationCycle = millis();
    dataEssentialSendActionEnabled = true;
    Serial.println("Communication cycle reset due to GPS time being stuck: " + String(lastCommunicationCycle));
  }

  // If the current second is a multiple of the communication cycle interval, start a new communication cycle
  if (navigation.navigation_data.gps.second % config.COMMUNICATION_CYCLE_INTERVAL == 0)
  {
    lastCommunicationCycle = millis();
    dataEssentialSendActionEnabled = true;
    Serial.println("New communication cycle started: " + String(lastCommunicationCycle) + " " + String(navigation.navigation_data.gps.hour) + ":" + String(navigation.navigation_data.gps.minute) + ":" + String(navigation.navigation_data.gps.second));
  }
}

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
          servo_2.write(config.SERVO_FINAL_POSITION);
          recoveryChannelShouldBeFired[i] = false;
          recoveryChannelFireTimes[i] = 0;
          Serial.println("Servo 2 engaged");
          return;
        }
      }

      // Check if the recovery channel should be toggled off
      if (millis() - recoveryChannelFireTimes[i] >= config.RECOVERY_CHANNEL_FIRE_TIME)
      {
        // Disable the recovery channel
        if (i == 0)
        {
          digitalWrite(config.RECOVERY_CHANNEL_1, LOW);
        }
        else if (i == 1)
        {
          servo_2.write(config.SERVO_FINAL_POSITION);
          // digitalWrite(config.RECOVERY_CHANNEL_2, LOW);
        }

        // Reset the recovery channel flag, but keep the fire time as it will not be fired again
        recoveryChannelShouldBeFired[i] = false;
        recoveryChannelFireTimes[i] = 0;
      }
    }
  }
}

void Actions::runDescentAction(Logging &logging, Config &config, Sensors &sensors, Navigation &navigation)
{
  // DESCENT ACTION HAS BEEN DISABLED AS IT WON'T BE DROPPED ON THE FIRST FLIGHT
  // // If parachute has been deployed, there is nothing to do
  // if (config.config_file_values.parachutes_deployed_flag == 1)
  // {
  //   return;
  // }

  // // Check if the descent has already been recorded as started
  // if (config.config_file_values.descent_flag == 1)
  // {
  //   // Check if the remaining descent time has elapsed
  //   if (config.config_file_values.remaining_descent_time <= 0)
  //   {
  //     // Deploy the parachute
  //     recoveryChannelShouldBeFired[0] = true;
  //     recoveryChannelShouldBeFired[1] = true;
  //     config.config_file_values.parachutes_deployed_flag = 1;
  //     logging.writeConfig(config);
  //   }
  //   else
  //   {
  //     // Decrease the remaining descent time by the time since the last loop
  //     config.config_file_values.remaining_descent_time -= total_loop_time;
  //     logging.writeConfig(config);
  //   }
  // }
  // else
  // {
  //   // Check if the launch rail switch is off
  //   if (digitalRead(config.LAUNCH_RAIL_SWITCH_PIN) == LOW)
  //   {
  //     if (launchRailSwitchOffTime == 0)
  //     {
  //       launchRailSwitchOffTime = millis();
  //     }
      
  //     // Check if the launch rail switch has been off for the threshold time
  //     if (millis() - launchRailSwitchOffTime >= config.LAUNCH_RAIL_SWITCH_OFF_THRESHOLD)
  //     {
  //       // Only allow the descent to be recorded as started if the altitude is above the threshold
  //       if ((sensors.data.onBoardBaro.altitude < config.LAUNCH_RAIL_SWITCH_ALTITUDE_THRESHOLD) or (navigation.navigation_data.gps.altitude < config.LAUNCH_RAIL_SWITCH_ALTITUDE_THRESHOLD))
  //       {
  //         return;
  //       }

  //       // Record the descent as started
  //       config.config_file_values.descent_flag = 1;
  //       config.config_file_values.remaining_descent_time = config.DESCENT_TIME_BEFORE_PARACHUTE_DEPLOYMENT;
  //       logging.writeConfig(config);
  //     }
  //   }
  //   // Reset the launch rail switch off time if the switch is high
  //   else
  //   {
  //     launchRailSwitchOffTime = 0;
  //   }
  // }
}

void Actions::checkBatteryVoltage(Sensors &sensors, Logging &logging, Config &config)
{
  // Check if the battery voltage is below the threshold
  if (sensors.data.battery.voltage <= config.BATTERY_LOW_VOLTAGE)
  {
    if (millis() - batteryVoltageLowLastBeepTime >= config.BATTERY_LOW_BEEP_INTERVAL)
    {
      batteryVoltageLowLastBeepState = !batteryVoltageLowLastBeepState;
      digitalWrite(config.BUZZER_PIN, batteryVoltageLowLastBeepState);
      batteryVoltageLowLastBeepTime = millis();
    }
  }
  else
  {
    digitalWrite(config.BUZZER_PIN, LOW);
    batteryVoltageLowLastBeepState = false;
  }
}

String Actions::createLoggablePacket(Sensors &sensors, Heater &heater, Navigation &navigation, Config &config)
{
  String packet = "";
  packet += String(loggable_packed_id);
  packet += ",";
  packet += String(millis());
  packet += ",";
  // GPS
  packet += String(navigation.navigation_data.gps.epoch_time);
  packet += ",";
  packet += String(navigation.navigation_data.gps.hour);
  packet += ":";
  packet += String(navigation.navigation_data.gps.minute);
  packet += ":";
  packet += String(navigation.navigation_data.gps.second);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lat, 7);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lng, 7);
  packet += ",";
  packet += String(navigation.navigation_data.gps.altitude, 2);
  packet += ",";
  packet += String(navigation.navigation_data.gps.speed, 2);
  packet += ",";
  packet += String(navigation.navigation_data.gps.satellites);
  packet += ",";
  packet += String(navigation.navigation_data.gps.heading);
  packet += ",";
  packet += String(navigation.navigation_data.gps.pdop);
  packet += ",";
  // Container temperature/pressure
  packet += String(sensors.data.containerTemperature.temperature, 2);
  packet += ",";
  packet += String(sensors.data.containerTemperature.filtered_temperature, 2);
  packet += ",";
  packet += String(sensors.data.containerBaro.temperature, 2);
  packet += ",";
  packet += String(sensors.data.containerBaro.pressure);
  packet += ",";
  packet += String(sensors._mcptemp, 1);
  packet += ",";
  // Heating system
  float p, i, d;
  heater.getPidValues(p, i, d);
  packet += String(heater.isHeaterEnabled());
  packet += ",";
  packet += String(heater.getCurrentTemperatureStep(), 2);
  packet += ",";
  packet += String(heater.getTargetTemperature(), 2);
  packet += ",";
  packet += String(heater.getHeaterPwm());
  packet += ",";
  packet += String(p, 2);
  packet += ",";
  packet += String(i, 2);
  packet += ",";
  packet += String(d, 2);
  packet += ",";
  // Onboard temperature/pressure
  packet += String(sensors.data.onBoardBaro.temperature, 2);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.pressure);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.altitude, 2);
  packet += ",";
  packet += String(sensors.data.outsideThermistor.temperature, 2);
  packet += ",";
  // IMU
  packet += String(sensors.data.imu.accel.acceleration.x, 4);
  packet += ",";
  packet += String(sensors.data.imu.accel.acceleration.y, 4);
  packet += ",";
  packet += String(sensors.data.imu.accel.acceleration.z, 4);
  packet += ",";
  packet += String(sensors.data.imu.accel.acceleration.heading, 3);
  packet += ",";
  packet += String(sensors.data.imu.accel.acceleration.pitch, 3);
  packet += ",";
  packet += String(sensors.data.imu.accel.acceleration.roll, 3);
  packet += ",";
  packet += String(sensors.data.imu.gyro.gyro.x, 4);
  packet += ",";
  packet += String(sensors.data.imu.gyro.gyro.y, 4);
  packet += ",";
  packet += String(sensors.data.imu.gyro.gyro.z, 4);
  packet += ",";
  packet += String(sensors.data.imu.temp.temperature, 2);
  // Ranging
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].distance, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].f_error, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].rssi, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].snr, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[0].time, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].distance, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].f_error, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].rssi, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].snr, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[1].time, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[2].distance, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[2].f_error, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[2].rssi, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[2].snr, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging[2].time, 2);
  packet += ",";
  packet += String(navigation.navigation_data.ranging_position.lat, 7);
  packet += ",";
  packet += String(navigation.navigation_data.ranging_position.lng, 7);
  packet += ",";
  packet += String(navigation.navigation_data.ranging_position.height, 2);
  packet += ",";
  // Battery/Heater current
  packet += String(sensors.data.battery.voltage, 2);
  packet += ",";
  // Performance/debugging
  packet += String(rp2040.getUsedHeap());
  packet += ",";
  packet += String(total_loop_time);
  packet += ",";
  packet += String(continuous_actions_time);
  packet += ",";
  packet += String(timed_actions_time);
  packet += ",";
  packet += String(requested_actions_time);
  packet += ",";
  packet += String(gps_read_time);
  packet += ",";
  packet += String(logging_time);
  packet += ",";
  packet += String(sensor_read_time);
  packet += ",";
  packet += String(on_board_baro_read_time);
  packet += ",";
  packet += String(imu_read_time);
  packet += ",";
  packet += String(battery_voltage_read_time);
  packet += ",";
  packet += String(container_baro_read_time);
  packet += ",";
  packet += String(container_temperature_read_time);
  packet += ",";
  packet += String(outside_thermistor_read_time);

  loggable_packed_id++;

  return packet;
}