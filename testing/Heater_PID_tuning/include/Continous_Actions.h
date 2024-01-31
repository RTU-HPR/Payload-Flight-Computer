#include <Actions.h>

void Actions::runContinousActions(Sensors &sensors, Communication &communication, Heater &heater, Config &config)
{
  // Run the sensor action
  if (sensorActionEnabled)
  {
    runSensorAction(sensors);
  }

  // Run the heater action
  if (heaterActionEnabled)
  {
    runHeaterAction(heater, sensors);
  }
}

void Actions::runCommandReceiveAction(Communication &communication, Config &config)
{
  
}

void Actions::runSensorAction(Sensors &sensors)
{
  // Read all sensors
  sensors.readSensors();
}

void Actions::runHeaterAction(Heater &heater, const Sensors &sensors)
{
  // Update the heater
  heater.update(sensors.data.containerTemperature.filtered_temperature);
}