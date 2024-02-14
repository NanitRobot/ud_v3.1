#include "config.hpp"
// Директива включення файлів конфігурації розумного будинку (Directive for Including Smart Home Configuration Files)

void setup()  // Стартова функція скетчу (Starting Function of the Sketch)
{
  // Serial.begin(9600);
  initdisplay();  // Функція ініціалізація дисплею Nanit (Function for Nanit Display Initialization)
  port_1_init();  // Функція ініціалізації 1-го порту (Function for Initializing Port 1)
  port_2_init();  // Функція ініціалізації 2-го порту (Function for Initializing Port 2)
  port_3_init();  // Функція ініціалізації 3-го порту (Function for Initializing Port 3)
  port_4_init();  // Функція ініціалізації 4-го порту (Function for Initializing Port 4)
  port_5_init();  // Функція ініціалізації 5-го порту (Function for Initializing Port 5)
  port_6_init();  // Функція ініціалізації 6-го порту (Function for Initializing Port 6)
  port_9_init();  // Функція ініціалізації 9-го порту (Function for Initializing Port 9)
}

void loop()  // Основний цикл роботи алгоритму розумного будинку (Main Loop of the Smart Home Algorithm)
{
  if (wireless_ctrl()) {
    parkin_ctrl();
    window_ctrl();
    fan_ctrl();
    light_ctrl();
    if (millis() % 3000 == 0) {
      send_data();
    }
  } else {
    keypad();            // Основна функція роботи клавіатури (Main Function for Keyboard Operation)
    if (locck())         // Розгалуження режимів роботи будинку (Branching for Smart Home Operation Modes)
    {                    // Блок режиму роботи, блокування зняте (Operation Mode Block, Locking Removed)
      displaySensors();  // Функція виведення показників з датчиків газу, температури та вологості
      // Function for Displaying Gas, Temperature, and Humidity Sensor Readings
      AirQuality_Fan();  // Функція роботи системи вентиляції (Function for Ventilation System Operation)
      window();          // Функція роботи вікна (Function for Window Operation)
      parkin();          // Функція роботи паркінгу (Function for Parking Operation)
      rgb4_set(GREEN);   // Колір RGB-світлодіода - зелений (The RGB LED color is green.)
    } else {             // Блок режиму роботи, блокування включене (Operation Mode Block, Locking Enabled)
      lock_home();       // Функція блокування розумного будинку (Function for Smart Home Locking)
      rgb4_set(RED);     // Колір RGB-світлодіода - червоний (The RGB LED color is red.)
    }
  }
}
