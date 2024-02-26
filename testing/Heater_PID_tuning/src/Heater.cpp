#include <Heater.h>

void Heater::begin(const Config::Heater_Config &heater_config)
{
  // Save the config
  _heater_config = heater_config;

  // Reset all values to default
  reset();

  // By default, the heater is disabled
  _heater_enabled = false;
}

void Heater::enableHeater(const float &container_temp)
{
  // Reset all values to default
  reset();

  // Set the current temperature step to the container temperature
  _current_temperature_step = _heater_config.target_temp - 1;

  // Enable the heater
  _heater_enabled = true;
}

bool Heater::update(const float &container_temp)
{
  // If the heater is enabled, calculate the PID values and set the heater PWM
  if (!_heater_enabled)
  {
    return false;
  }

  calculatePidValues(container_temp);
  calculateHeaterPwm(container_temp);
  setHeaterPwm();
  return true;
}

void Heater::calculatePidValues(const float &container_temp)
{
  // Time since last pid update
  float dt = (millis() - _last_pid_calculation_time);

  // If the temperature is 0.05 degree below or above the safe temp, reset the time at temp step
  if ((_current_temperature_step - 0.05) > container_temp || container_temp > (_current_temperature_step + 0.05))
  {
    _time_at_temperature_step_start = millis();
  }
  
  // Update temperature step temp value to nearest rounded down integer
  // if the container temp exceededs current temp step temp
  if (container_temp >= _current_temperature_step + 1)
  {
    _current_temperature_step = constrain(floor(container_temp), 0, _heater_config.target_temp);
    _integral_term += _proportional_term + _derivative_term - 50;
  }
  else if (container_temp >= _current_temperature_step + 0.5)
  {
    _current_temperature_step = constrain(floor(container_temp) + 0.5, 0, _heater_config.target_temp);
    _integral_term += _proportional_term + _derivative_term - 50;
  }

  // Calculate proportional term based on the state
  if (_heater_config.target_temp == _current_temperature_step)
  {
    _proportional_term = _heater_config.target_temp - container_temp;
  }
  else
  {
    int step_to_increase_by = 1;

    // If the temperature has stayed at the safe temp for 10 seconds, increase the safe temperature
    if ((millis() - _time_at_temperature_step_start) > 10000)
    {
      _current_temperature_step += step_to_increase_by;
      _time_at_temperature_step_start = millis();
    }
    _proportional_term = _current_temperature_step - container_temp;
  }

  // Integral term
  _integral_term += _proportional_term * dt;

  // Derivative term
  _derivative_term = (_proportional_term - _last_proportional_term) / (float)dt;

  // Make sure the the PID values are within the limits
  _proportional_term = constrain(_proportional_term, -_heater_config.Kp_limit, _heater_config.Kp_limit);
  _integral_term = constrain(_integral_term, 0, _heater_config.Ki_limit / (float)_heater_config.Ki); // Integral term can't be negative in our case
  _derivative_term = constrain(_derivative_term, -_heater_config.Kd_limit, _heater_config.Kd_limit);

  // Save last proportional term for future derivative term calculations
  _last_proportional_term = _proportional_term;

  // Save last PID calculation time for time delta calculations
  _last_pid_calculation_time = millis();
}

void Heater::calculateHeaterPwm(const float &container_temp)
{
  // Heater power is the sum of all the individual PID values multiplied by their coefficients
  _heater_pwm = _heater_config.Kp * _proportional_term;
  _heater_pwm += _heater_config.Ki * _integral_term;
  _heater_pwm += _heater_config.Kd * _derivative_term;

  // Constrain the final sum to PWM output range
  if (container_temp >= _heater_config.target_temp + 0.05)
  {
    _heater_pwm /= 1.1;
  }
  else if (container_temp >= _heater_config.target_temp + 0.1)
  {
    _heater_pwm /= 1.4;
  }
  else if (container_temp >= _heater_config.target_temp + 0.2)
  {
    _heater_pwm /= 3.5;
  }
  else if (container_temp >= _heater_config.target_temp + 0.5)
  {
    _heater_pwm = 0;
  }
  else
  {
    _heater_pwm = constrain(_heater_pwm, _heater_config.pwm_min, _heater_config.pwm_max);
  }
}

// heater PWM value
void Heater::setHeaterPwm()
{
  analogWrite(_heater_config.heater_pin, (int)_heater_pwm);
}

void Heater::reset()
{
  _last_pid_calculation_time = millis();
  _time_at_temperature_step_start = millis();

  float _proportional_term = 0;
  float _last_proportional_term = 0;
  float _derivative_term = 0;
  float _integral_term = 0;

  float _current_temperature_step = 0;
  float _heater_pwm = 0;
  setHeaterPwm();
}