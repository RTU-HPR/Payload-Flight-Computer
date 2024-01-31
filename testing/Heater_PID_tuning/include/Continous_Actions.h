#include <Actions.h>

// Performance monitoring
unsigned long last_sensor_read_millis = 0;
unsigned long last_gps_read_millis = 0;
unsigned long last_logging_millis = 0;

void Actions::runContinousActions(Sensors &sensors, Navigation &navigation, Communication &communication, Logging &logging, Heater &heater, Config &config)
{
  // Run the sensor action
  if (sensorActionEnabled)
  {
    last_sensor_read_millis = millis();
    runSensorAction(sensors);
    sensor_read_time = millis() - last_sensor_read_millis;
  }

  // Run the heater action
  if (heaterActionEnabled)
  {
    runHeaterAction(heater, sensors);
  }
}

void Actions::runCommandReceiveAction(Communication &communication, Logging &logging, Config &config)
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