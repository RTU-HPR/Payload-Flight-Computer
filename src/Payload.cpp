#include "Payload.h"

bool Payload::initCommunicationBusses()
{
  bool success = true;

  // Wire0
  if (Wire.setSCL(config.WIRE0_SCL) && Wire.setSDA(config.WIRE0_SDA))
  {
    Wire.begin();
    Serial.println("Wire0 communication bus initialized");
  }
  else
  {
    String errorString = "Wire0 begin fail";
    logging.recordError(errorString);
    success = false;
  }

  // SPI
  if (SPI.setRX(config.SPI0_RX) && SPI.setTX(config.SPI0_TX) && SPI.setSCK(config.SPI0_SCK))
  {
    SPI.begin();
    Serial.println("SPI0 communication bus initialized");
  }
  else
  {
    String errorString = "SPI begin fail";
    logging.recordError(errorString);
    success = false;
  }

  return success;
}

void Payload::begin()
{
  // Initialize PC serial
  Serial.begin(config.PC_BAUDRATE);
  if (config.WAIT_PC)
  {
    while (!Serial)
    {
      delay(100);
    }
  }

  Serial.println("PC Serial initialized");

  // Initialize the communication busses
  if (initCommunicationBusses())
  {
    Serial.println("All communication busses initialized successfully");
  }
  else
  {
    Serial.println("Error in initializing communication busses");
  }

  // Enable sensor power
  pinMode(config.SENSOR_POWER_ENABLE_PIN, OUTPUT_12MA);
  digitalWrite(config.SENSOR_POWER_ENABLE_PIN, HIGH);

  Serial.println("Sensor power enabled");

  // Set the recovery channels to output and pull them low
  pinMode(config.RECOVERY_CHANNEL_1, OUTPUT_12MA);
  pinMode(config.RECOVERY_CHANNEL_2, OUTPUT_12MA);
  digitalWrite(config.RECOVERY_CHANNEL_1, LOW);
  digitalWrite(config.RECOVERY_CHANNEL_2, LOW);

  Serial.println("Recovery channels set to output and pulled low");

  // Set the buzzer pin to output and pull it low
  pinMode(config.BUZZER_PIN, OUTPUT_12MA);
  digitalWrite(config.BUZZER_PIN, LOW);

  // Set the launch rail switch to input
  pinMode(config.LAUNCH_RAIL_SWITCH_PIN, INPUT);

  Serial.println("Launch rail switch set to input");

  // Set the heater pin to output and pull it low
  pinMode(config.heater_config.heater_pin, OUTPUT_12MA);
  digitalWrite(config.heater_config.heater_pin, LOW);
  // The analog write range and frequency has to be changed for heater PWM to work properly
  analogWriteRange(1000); // Don't change this value
  analogWriteFreq(100);   // Don't change this value

  // Initialize the SD card
  if (!logging.begin(config))
  {
    String errorString = "SD begin fail";
    logging.recordError(errorString);
  }
  else
  {
    Serial.println("SD card initialized successfully");

    // Read config file
    if (!logging.readConfig(config))
    {
      String infoString = "New config file created";
      logging.recordInfo(infoString);
    }
    else
    {
      Serial.println("Config file read successfully");
    }
  }

  // Initialise the radio
  if (!communication.beginRadio(config))
  {
    String errorString = "Radio begin fail";
    logging.recordError(errorString);
  }
  else
  {
    Serial.println("Radio initialized successfully");
  }

  Serial.println();

  // Send inital error string
  if (!logging.infoErrorQueueEmpty())
  {
    String infoError = "";
    while (!logging.infoErrorQueueEmpty())
    {
      infoError += logging.readFromInfoErrorQueue();
      if (!logging.infoErrorQueueEmpty())
      {
        infoError += ",";
      }
    }
    Serial.println("INITAL INFO/ERROR: " + infoError);
    Serial.println();
    logging.writeError(infoError);
  }

  // Initialise all sensors
  if (!sensors.begin(logging, config))
  {
    Serial.println("Error initializing sensors");
  }
  else
  {
    Serial.println("Sensors initialized successfully");
  }

  // Send inital error string
  if (!logging.infoErrorQueueEmpty())
  {
    String infoError = "";
    while (!logging.infoErrorQueueEmpty())
    {
      infoError += logging.readFromInfoErrorQueue();
      if (!logging.infoErrorQueueEmpty())
      {
        infoError += ",";
      }
    }
    Serial.println("SENSOR ERROR: " + infoError);
    Serial.println();
    logging.writeError(infoError);
  }

  // Initialise GPS
  if (!navigation.beginGps(config.gps_config))
  {
    String errorString = "GPS begin fail";
    Serial.println(errorString);
    logging.recordError(errorString);
  }
  else
  {
    Serial.println("GPS initialized successfully");
  }

  // Initialise ranging
  if (!navigation.beginRanging(config.ranging_device, config.ranging_mode))
  {
    String errorString = "Ranging begin fail";
    Serial.println(errorString);
    logging.recordError(errorString);
  }
  else
  {
    Serial.println("Navigation initialized successfully");
  }

  // Initialise the heater
  heater.begin(config.heater_config);
  heater.enableHeater(sensors.data.containerTemperature.filtered_temperature);
  Serial.println("Heater initialized successfully");

  Serial.println();
}