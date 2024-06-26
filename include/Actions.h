#pragma once
#include <Config.h>
#include <Communication.h>
#include <Sensors.h>
#include <Navigation.h>
#include <Logging.h>
#include <Heater.h>

// Get performance monitoring global variables
extern int total_loop_time;
extern int continuous_actions_time;
extern int timed_actions_time;
extern int requested_actions_time;
extern int gps_read_time;
extern int logging_time;
extern int sensor_read_time;

class Actions
{
private:
    // Prerequisite functions
    String createEssentialDataPacket(Sensors &sensors, Navigation &navigation, Logging &logging, Config &config);
    String createCompleteDataPacket(Sensors &sensors, Navigation &navigation, Heater &heater, Config &config);
    String createLoggablePacket(Sensors &sensors, Heater &heater, Navigation &navigation, Config &config);
    unsigned long loggable_packed_id = 1;

    // Continuous actions
    void runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config);

    void runCommandReceiveAction(Communication &communication, Logging &logging, Config &config);
    bool commandReceiveActionEnabled = true;

    void runSensorAction(Sensors &sensors);
    bool sensorActionEnabled = true;

    void runHeaterAction(Heater &heater, const Sensors &sensors);
    bool heaterActionEnabled = true;

    void runGpsAction(Navigation &navigation);
    bool gpsActionEnabled = true;

    void runLoggingAction(Logging &logging, Navigation &navigation, Sensors &sensors, Heater &heater, Config &config);
    bool loggingActionEnabled = true;

    void runRangingAction(Navigation &navigation, Config &config);
    bool rangingSendActionEnabled = true;

    void runGetCommunicationCycleStartAction(Navigation &navigation, Config &config);
    bool getCommunicationCycleStartActionEnabled = true;
    unsigned long lastCommunicationCycle = 0;

    void runRecoveryChannelManagerAction(Config &config);
    bool recoveryChannelManagerActionEnabled = true;
    bool recoveryChannelShouldBeFired[2] = {false, false};
    unsigned long recoveryChannelFireTimes[2] = {0, 0};

    void runDescentAction(Logging &logging, Config &config, Sensors &sensors, Navigation &navigation);
    bool descentActionEnabled = true;
    unsigned long launchRailSwitchOffTime = 0;

    void checkBatteryVoltage(Sensors &sensors, Logging &logging, Config &config);
    bool batteryVoltageCheckEnabled = true;
    bool batteryVoltageLowLastBeepState = false;
    bool batteryVoltageLowLastBeepTime = 0;

    // Timed actions
    void runTimedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);

    void runBuzzerAction(Config &config);
    bool buzzerActionEnabled = true;
    unsigned long buzzerLastStateTime = 0;

    void runEssentialDataSendAction(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Config &config);
    bool dataEssentialSendActionEnabled = true;
    uint16_t dataEssentialResponseId = 0;

    // Requested actions
    void runRequestedActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config);

    void runInfoErrorSendAction(Communication &communication, Logging &logging, Navigation &navigation, Config &config);
    bool infoErrorRequestActionEnabled = false;
    uint16_t infoErrorResponseId = 0;

    void runCompleteDataRequestAction(Sensors &sensors, Navigation &navigation, Communication &communication, Heater &heater, Config &config);
    bool completeDataRequestActionEnabled = false;
    uint16_t completeDataResponseId = 0;

    void runFormatStorageAction(Communication &communication, Logging &logging, Navigation &navigation, Config &config);
    bool formatStorageActionEnabled = false;
    uint16_t formatResponseId = 0;

    void runRecoveryFireAction(Communication &communication, Navigation &navigation, Config &config);
    bool recoveryFireActionEnabled = false;
    uint16_t recoveryResponseId = 0;

public:
    void runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config);
};