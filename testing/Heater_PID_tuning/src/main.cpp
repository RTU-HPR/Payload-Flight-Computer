#include <Payload.h>

Payload payload;

// Performance monitoring
unsigned long last_total_loop_millis = 0;
unsigned long total_loop_time = 0;

void setup()
{
  // INCREASES SPI CLOCK SPEED TO BE THE SAME AS CPU SPEED
  // MUST BE AT THE START OF SETUP
  // Get the processor sys_clk frequency in Hz
  uint32_t freq = clock_get_hz(clk_sys);

  // clk_peri does not have a divider, so input and output frequencies will be the same
  clock_configure(clk_peri,
                  0,
                  CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                  freq,
                  freq);

  payload.begin();
  Serial.println("Payload setup complete");
  Serial.println("CPU Speed: " + String(rp2040.f_cpu() / 1000000) + " MHz");
  Serial.println();
}

void loop()
{
  last_total_loop_millis = millis();
  payload.actions.runAllActions(payload.sensors, payload.communication, payload.heater, payload.config);
  total_loop_time = millis() - last_total_loop_millis;
}
