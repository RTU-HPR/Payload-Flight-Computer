#include <Config.h>

Config config;
AdcVoltage batteryVoltageReader;
AdcVoltage::AdcVoltage_Data battery;
Adafruit_INA219 ina219;

const int heater_pin = 22;

int heater_pwm = 0;

float current_mA = 0;

void setup()
{
  Serial.begin(config.PC_BAUDRATE);
  while (!Serial)
  {
    delay(100);
  }

  Wire.setSCL(config.WIRE0_SCL);
  Wire.setSDA(config.WIRE0_SDA);
  Wire.begin();

  // Enable sensor power
  pinMode(config.SENSOR_POWER_ENABLE_PIN, OUTPUT_12MA);
  digitalWrite(config.SENSOR_POWER_ENABLE_PIN, HIGH);

  Serial.println("Sensor power enabled");

  // Set the heater pin to output and pull it low
  pinMode(heater_pin, OUTPUT_12MA);
  digitalWrite(heater_pin, LOW);

  // The analog write range and frequency has to be changed for heater PWM to work properly
  analogWriteRange(10000); // Don't change this value
  analogWriteFreq(100);    // Don't change this value

  analogReadResolution(12);
  batteryVoltageReader.begin(config.battery_voltage_reader_config);

  if (!ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
    while (1)
    {
      delay(10);
    }
  }

  Serial.println("Heater PWM: " + String(heater_pwm));
  for (int i = 0; i < 10; i++)
  {
    batteryVoltageReader.read(battery);
    delay(1);
  }
  Serial.println("Initial battery voltage: " + String(battery.voltage) + "V");
  Serial.println();

  for (int i = 0; i <= 2000; i += 10)
  {
    analogWrite(heater_pin, i);
    delay(10);

    current_mA = 0;
    for (int x = 0; x < 1000; x++)
    {
      batteryVoltageReader.read(battery);
      current_mA += ina219.getCurrent_mA();
    }
    current_mA /= 1000;

    Serial.println(String(i) + "," + String(current_mA, 2) + ","+ String(battery.voltage, 2));
  }
  digitalWrite(heater_pin, LOW);
}

void loop()
{
}
