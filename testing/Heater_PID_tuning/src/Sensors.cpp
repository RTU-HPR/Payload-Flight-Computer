#include "Sensors.h"

// Performance monitoring
unsigned int last_on_board_baro_read_millis = 0;
unsigned int last_imu_read_millis = 0;
unsigned int last_battery_voltage_read_millis = 0;
unsigned int last_container_heater_voltage_read_millis = 0;
unsigned int last_container_baro_read_millis = 0;
unsigned int last_container_temperature_read_millis = 0;
unsigned int last_outside_thermistor_read_millis = 0;

bool Sensors::begin(Config &config)
{
  bool success = true;
  // Change analogRead resolution
  // This is needed for higher accuracy battery voltage and thermistor readings
  analogReadResolution(12);

  // MAIN BOARD
  // Initialize MS56XX
  if (!beginOnBoardBaro(config))
  {
    success = false;
  }
  else
  {
    Serial.println("Onboard barometer initialization complete");
  }

  // Initialize IMU
  if (!beginImu(config))
  {
    success = false;
  }
  else
  {
    Serial.println("IMU initialization complete");
  }

  // Initialize thermistor
  if (!beginOutsideThermistor(config))
  {
    success = false;
  }
  else
  {
    Serial.println("Thermistor initialization complete");
  }

  // Initialize battery voltage reader
  if (!beginBatteryVoltageReader(config))
  {
    success = false;
  }
  else
  {
    Serial.println("Battery voltage reader initialization complete");
  }

  // HEATED CONTAINER
  // Initialize container barometer
  if (!beginContainerBaro(config))
  {
    success = false; // Disabled for now, since the sensor is not connected
  }
  else
  {
    Serial.println("Container barometer initialization complete");
  }

  // Initialize container temperature sensor
  if (!beginContainerTemperatureSensor(config))
  {
    success = false; // Disabled for now, since the sensor is not connected
  }
  else
  {
    Serial.println("Container temperature sensor initialization complete");
  }
  Serial.println();

  if (!mcp9808.begin(0x1F))
  {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1)
      ;
  }
  mcp9808.setResolution(3);
  delay(100);
  mcp9808.wake();

  return success;
}

void Sensors::readSensors()
{
  readOnBoardBaro();
  readImu();
  readBatteryVoltage();
  readOutsideThermistor();
  readContainerBarometer();
  readContainerTemperature();
  _mcptemp = mcp9808.readTempC();
}

bool Sensors::beginOnBoardBaro(Config &config)
{
  if (!_onBoardBaro.begin(config.ms56xx_config))
  {
    return false;
  }
  return true;
}

bool Sensors::beginImu(Config &config)
{
  // No point in putting in if statement, it will always return false
  // The problem is in the library, where the begin function returns false in any case
  _imu.begin_I2C(config.imu_config.i2c_address, config.imu_config.wire);

  // Set IMU settings
  _imu.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  _imu.setGyroRange(LSM6DS_GYRO_RANGE_1000_DPS);
  _imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
  _imu.setGyroDataRate(LSM6DS_RATE_104_HZ);
  return true;
}

bool Sensors::beginOutsideThermistor(Config &config)
{
  _outsideThermistor = NTC_Thermistor(
      config.outside_thermistor_config.pin,
      config.outside_thermistor_config.reference_resistor,
      config.outside_thermistor_config.nominal_resistor,
      config.outside_thermistor_config.nominal_temperature,
      config.outside_thermistor_config.b_coefficient,
      config.outside_thermistor_config.adc_resolution);
  return true;
}

bool Sensors::beginBatteryVoltageReader(Config &config)
{
  _batteryVoltageReader.begin(config.battery_voltage_reader_config);
  return true;
}

bool Sensors::beginContainerBaro(Config &config)
{
  if (!_containerBaro.begin(config.BMP180_config.i2c_address, config.BMP180_config.wire))
  {
    return false;
  }
  return true;
}

bool Sensors::beginContainerTemperatureSensor(Config &config)
{
  _containerTemperatureSensor = ClosedCube::Sensor::STS35(config.STS35_config.wire);
  _containerTemperatureSensor.address(config.STS35_config.i2c_address);

  // Test temp probe to see if its working
  float test = _containerTemperatureSensor.readTemperature();
  if (test > 100.00 || test < -100.0 || test == 0.00)
  {
    return false;
  }

  // Read the temperature sensor to get the first 50 value
  for (int i = 0; i < 50; i++)
  {
    data.containerTemperature.filtered_temperature = _containerTemperatureFilter.updateEstimate(_containerTemperatureSensor.readTemperature());
  }

  return true;
}

bool Sensors::readOnBoardBaro()
{
  if (_onBoardBaro.read(data.onBoardBaro))
  {
    return true;
  }
  Serial.println("Onboard barometer reading failed!");
  return false;
}

bool Sensors::readImu()
{
  // Read the sensor
  if (_imu.getEvent(&data.imu.accel, &data.imu.gyro, &data.imu.temp))
  {
    return true;
  }
  Serial.println("IMU reading failed!");
  return false;
}

bool Sensors::readBatteryVoltage()
{
  // Read voltage and do calculations
  if (_batteryVoltageReader.read(data.battery))
  {
    return true;
  }
  Serial.println("Battery voltage reading failed!");
  return false;
}

bool Sensors::readOutsideThermistor()
{
  // Read temperature
  float new_temperature = _outsideThermistor.readCelsius();

  // Just make sure the temperature value is within reasonable values
  if (new_temperature > -100 && new_temperature < 100) // Between -100 and 100 C
  {
    data.outsideThermistor.temperature = new_temperature;
    return true;
  }
  Serial.println("Outside thermistor reading failed!");
  return false;
}

bool Sensors::readContainerBarometer()
{
  float new_pressure = _containerBaro.readPressure();
  float new_temperature = _containerBaro.readTemperature();

  if ((new_pressure > 0 && new_pressure < 200000) && (new_temperature > -100 && new_temperature < 100)) // Between 100 and 120_000 Pa and -100 and 100 C
  {
    data.containerBaro.pressure = new_pressure;
    data.containerBaro.temperature = new_temperature;
    return true;
  }
  Serial.println("Container barometer reading failed!");
  return false;
}

bool Sensors::readContainerTemperature()
{

  float new_temperature = _containerTemperatureSensor.readTemperature();
  if (new_temperature > -100 && new_temperature < 100) // Between -100 and 100 C
  {
    data.containerTemperature.temperature = new_temperature;
    data.containerTemperature.filtered_temperature = _containerTemperatureFilter.updateEstimate(new_temperature);
    return true;
  }
  Serial.println("Container temperature sensor reading failed!");
  return false;
}