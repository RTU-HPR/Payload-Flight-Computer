#pragma once
#include <Config.h>

class Actions
{
private:
    void runDescentAction(Config &config);
    bool descentActionEnabled = true;
    unsigned long launchRailSwitchOffTime = 0;

    void runRecoveryChannelManagerAction(Config &config);
    bool recoveryChannelManagerActionEnabled = true;
    bool recoveryChannelShouldBeFired[2] = {false, false};
    unsigned long recoveryChannelFireTimes[2] = {0, 0};

    void runBuzzerAction(Config &config);
    bool buzzerActionEnabled = true;
    unsigned long buzzerLastStateTime = 0;

public:
    void runAllActions(Config &config);

    float altitude = 0;
};