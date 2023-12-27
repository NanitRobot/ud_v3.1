#include "config.hpp"

bool flag =0;

void setup() 
{
  Serial.begin(9600);
  initdisplay();
  port_1_init();
  port_2_init();
  port_3_init();
  port_4_init();
  port_5_init();
  port_6_init();
  port_9_init();
  delay(1000); // Затримка 1 секунда (Delay for 1 second)
}

void loop() 
{
  displaySensors();
  C_O_filter();
  window();
  parkin();
}
