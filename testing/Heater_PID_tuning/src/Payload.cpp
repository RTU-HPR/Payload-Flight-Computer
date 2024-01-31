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
    success = false;
  }

  // Wire1
  if (Wire1.setSCL(config.WIRE1_SCL) && Wire1.setSDA(config.WIRE1_SDA))
  {
    Wire1.begin();
    Serial.println("Wire1 communication bus initialized");
  }
  else
  {
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

  Serial.println("Pyro channels set to output and pulled low");

  Serial.println("Launch rail switch set to input");

  // Set the heater pin to output and pull it low
  pinMode(config.heater_config.heater_pin, OUTPUT_12MA);
  digitalWrite(config.heater_config.heater_pin, LOW);
  // The analog write range and frequency has to be changed for heater PWM to work properly
  analogWriteRange(1000); // Don't change this value
  analogWriteFreq(100);   // Don't change this value

  // Initialise all sensors
  if (!sensors.begin(config))
  {
    Serial.println("Error initializing sensors");
  }
  else
  {
    Serial.println("Sensors initialized successfully");
  }

  // Initialise the heater
  heater.begin(config.heater_config);
  heater.enableHeater(sensors.data.containerTemperature.filtered_temperature);
  Serial.println("Heater initialized successfully");

  Serial.println();
}