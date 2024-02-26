#pragma once
#include <Config.h>
#include <Actions.h>

class Payload
{
  public:
    Config config;
    Actions actions;

    /**
     * @brief Initialise the PFC
    */
    void begin();
  };
