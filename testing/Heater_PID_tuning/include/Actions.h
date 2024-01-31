#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Heater.h>

class Actions
{
private:
    // Continuous actions
    void runContinousActions(Sensors &sensors, Communication &communication, Heater &heater, Config &config);

    void runCommandReceiveAction(Communication &communication, Config &config);
    bool commandReceiveActionEnabled = true;

    void runSensorAction(Sensors &sensors);
    bool sensorActionEnabled = true;

    void runHeaterAction(Heater &heater, const Sensors &sensors);
    bool heaterActionEnabled = true;

public:
    void runAllActions(Sensors &sensors, Communication &communication, Heater &heater, Config &config);
};