#include <Actions.h>
#include <Continous_Actions.h>

void Actions::runAllActions(Sensors &sensors, Communication &communication, Heater &heater, Config &config)
{
  runContinousActions(sensors, communication, heater, config);
}
