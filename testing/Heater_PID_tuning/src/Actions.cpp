#include <Actions.h>
#include <Continous_Actions.h>
#include <Timed_Actions.h>
#include <Requested_Actions.h>

// Performance monitoring
extern int continuous_actions_time;
extern int timed_actions_time;
extern int requested_actions_time;
unsigned long last_continous_actions_millis = 0;
unsigned long last_timed_actions_millis = 0;
unsigned long last_requested_actions_millis = 0;

void Actions::runAllActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config)
{
  last_continous_actions_millis = millis();
  runContinousActions(sensors, navigation, communication, logging, heater, config);
  continuous_actions_time = millis() - last_continous_actions_millis;
}
