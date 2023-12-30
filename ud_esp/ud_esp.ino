#include "config.hpp"

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
}

void loop() 
{
  keypad();
  if(locck())
  {
    displaySensors();
    C_O_filter();
    window();
    parkin();
    rgb4_set(0, 1, 0);
  }
  else{
    lock_home();
    rgb4_set(1, 0, 0);
  }
}
