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

  // Run the communication action
  if (communicationActionEnabled)
  {
    runCommunicationAction(communication, sensors, config, heater);
  }
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

void Actions::runCommunicationAction(Communication &communication, const Sensors &sensors, const Config &config, Heater &heater)
{
  // Send data to the PC
  String data = "";
  data += String(millis());
  data += ",";
  data += String(sensors.data.containerTemperature.filtered_temperature);
  data += ",";
  data += String(sensors.data.containerBaro.temperature);
  data += ",";
  data += String(heater.getTargetTemperature());
  data += ",";
  data += String(heater.getCurrentTemperatureStep());
  data += ",";
  data += String(sensors.data.containerBaro.pressure);
  data += ",";
  float p, i, d;
  heater.getPidValues(p, i, d);
  data += String(p, 2);
  data += ",";
  data += String(i, 2);
  data += ",";
  data += String(d, 2);
  data += ",";
  data += String(heater.getHeaterPwm());

  communication.WifiSendData(data);
}