#include <Actions.h>

void Actions::runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  // Only start timed actions 10 seconds after turning on to make sure everything is initialised
  if (millis() < config.TIMED_ACTION_INITIAL_DELAY)
  {
    return;
  }

  if (dataEssentialSendActionEnabled)
  {
    runEssentialDataSendAction(sensors, navigation, communication, logging, config);
  }

  if (buzzerActionEnabled)
  {
    runBuzzerAction(config);
  }
}

void Actions::runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config)
{
  if (millis() - lastCommunicationCycle >= config.COMMUNICATION_ESSENTIAL_DATA_SEND_TIME && millis() - lastCommunicationCycle <= config.COMMUNICATION_CYCLE_INTERVAL)
  {
    String msg_str = createEssentialDataPacket(sensors, navigation, logging, config);

    // Create packet
    uint16_t ccsds_packet_length;
    byte *ccsds_packet = create_ccsds_telemetry_packet(config.PFC_ESSENTIAL_DATA_RESPONSE, dataEssentialResponseId, navigation.navigation_data.gps.epoch_time, 0, msg_str, ccsds_packet_length);

    // Send packet
    if (!communication.sendRadio(ccsds_packet, ccsds_packet_length))
    {
      // Free memory after the packet has been sent
      delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
      return;
    }
    dataEssentialResponseId++;
    dataEssentialSendActionEnabled = false;
    Serial.println("Sending essential data");

    // Free memory after the packet has been sent
    delete[] ccsds_packet; // VERY IMPORTANT, otherwise a significant memory leak will occur
  }
}

void Actions::runBuzzerAction(Config &config)
{
  // If the descent flag is set, or the payload has been on for more than 2 hours, start the beeping
  if (config.config_file_values.descent_flag == 1 or millis() > config.BUZZER_ACTION_START_TIME)
  {
    if (millis() - buzzerLastStateTime >= config.BUZZER_BEEP_TIME)
    {
      buzzerLastStateTime = millis();
      digitalWrite(config.BUZZER_PIN, !digitalRead(config.BUZZER_PIN));
    }
  }
}

String Actions::createEssentialDataPacket(Sensors &sensors, Navigation &navigation, Logging &logging, Config &config)
{
  String packet = "";
  packet += String(navigation.navigation_data.gps.lat, 6);
  packet += ",";
  packet += String(navigation.navigation_data.gps.lng, 6);
  packet += ",";
  packet += String(navigation.navigation_data.gps.altitude, 2);
  packet += ",";
  packet += String(sensors.data.onBoardBaro.altitude, 2);
  packet += ",";
  packet += String(navigation.navigation_data.gps.satellites);
  packet += ",";
  packet += String(!logging.infoErrorQueueEmpty());

  return packet;
}