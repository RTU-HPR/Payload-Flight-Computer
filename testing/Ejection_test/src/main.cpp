#include <Payload.h>

Payload payload;

// Performance monitoring
unsigned long last_total_loop_millis = 0;

// Declare as global variables
extern int total_loop_time;

// Set initial values
int total_loop_time = 0;

void setup()
{
  payload.begin();
  Serial.println("Test setup complete");
}

void loop()
{
  last_total_loop_millis = millis();
  payload.actions.runAllActions(payload.config);
  total_loop_time = millis() - last_total_loop_millis;
}
