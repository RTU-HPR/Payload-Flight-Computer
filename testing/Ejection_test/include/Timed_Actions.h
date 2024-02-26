#include <Actions.h>

void Actions::runBuzzerAction(Config &config)
{
  // If the descent flag is set, or the payload has been on for more than 2 hours, start the beeping
  if (config.config_file_values.descent_flag == 1 or millis() > config.BUZZER_ACTION_START_TIME)
  {
    if (millis() - buzzerLastStateTime >= config.BUZZER_BEEP_TIME)
    {
      buzzerLastStateTime = millis();
      digitalWrite(config.BUZZER_PIN, !digitalRead(config.BUZZER_PIN));
    }
  }
}