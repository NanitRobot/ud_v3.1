#include "config.hpp"

bool flag =0;

void setup() 
{
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
  displaySensors();
  C_O_filter();
  window();
  parkin();

}
