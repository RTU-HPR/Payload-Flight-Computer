#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Heater.h>

// Get performance monitoring global variables
extern int total_loop_time;
extern int continuous_actions_time;
extern int sensor_read_time;

class Actions
{
private:
    // Continuous actions
    void runContinousActions(Sensors &sensorsCommunication &communication, Logging &logging, Heater &heater, Config &config);

    void runCommandReceiveAction(Communication &communication, Logging &logging, Config &config);
    bool commandReceiveActionEnabled = true;

    void runSensorAction(Sensors &sensors);
    bool sensorActionEnabled = true;

    void runHeaterAction(Heater &heater, const Sensors &sensors);
    bool heaterActionEnabled = true;

public:
    void runAllActions(Sensors &sensors, Communication &communication, Heater &heater, Config &config);
};