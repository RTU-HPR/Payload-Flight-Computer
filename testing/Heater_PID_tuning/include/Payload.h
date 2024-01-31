#pragma once
#include <Config.h>
#include <Sensors.h>
#include <Actions.h>
#include <Communication.h>
#include <Heater.h>

class Payload
{
  private:
    /**
     * @brief Initialise the HardwareSerial, SPI, I2C communication busses
    */
    bool initCommunicationBusses();
    
  public:
    Config config;
    Sensors sensors;
    Actions actions;
    Communication communication;
    Heater heater;

    /**
     * @brief Initialise the PFC
    */
    void begin();
  };
