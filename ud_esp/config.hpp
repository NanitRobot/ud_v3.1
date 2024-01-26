#include <NanitLib.h>
#include <DHT.h>
#include <TM1637Display.h>
#include <Keypad.h>  
#include <Password.h>

#define SERVO_PIN P1_1 
//MOTOR1_A вже готовий дефайн P1_3 
//MOTOR1_B вже готовий дефайн P1_4

#define IN1       P2_1    // Пін 1 для крокового двигуна
#define IN2       P2_2    // Пін 2 для крокового двигуна
#define IN3       P2_3    // Пін 3 для крокового двигуна
#define IN4       P2_4    // Пін 4 для крокового двигуна

// #define RGB_LED_3 1     // Макрос для RGB світлофора (Macro for RGB Traffic Light)
#define SEPARATE_LEDS_3 1  // Макрос для звичайного світлофора (Macro for Standard Traffic Light)

#if defined(SEPARATE_LEDS_3)     // Піни звичайного світлофора (Pins of a Standard Traffic Light)

#define TL_RED P3_2          // Пін червоного світлодіода для світлофора (Pin for the Red LED of the Traffic Light)
#define TL_YLW P3_3          // Пін жовтого світлодіода для світлофора (Pin for the Yellow LED of the Traffic Light)
#define TL_GRN P3_4          // Пін зеленого світлодіода для світлофора (Pin for the Green LED of the Traffic Light)

#elif defined(RGB_LED_3)         // Піни RGB світлофора (Pins of an RGB Traffic Light)

#define RGB_RDL P3_2            // Пін червоного кольору RGB світлофора (Pin for the Red Color of the RGB Traffic Light)
#define RGB_GRN P3_3            // Пін зеленого кольору RGB світлофора  (Pin for the Green Color of the RGB Traffic Light)
#define RGB_BLU P3_4            // Пін синього кольору RGB світлофора   (Pin for the Blue Color of the RGB Traffic Light)

#endif

#define RED 1    // Макрос червоного кольору для функції світлофора (Macro for the Red Color in the Traffic Light Function)
#define YELLOW 2 // Макрос жовтого кольору для функції світлофора (Macro for the Yellow Color in the Traffic Light Function)
#define GREEN 3  // Макрос зеленого кольору для функції світлофора (Macro for the Green Color in the Traffic Light Function)

#define OFF 0
#define RED 1    // Макрос червоного кольору для RGB світлодіода з 4 порта (Macro for Red Color of the RGB LED on Port 4)
#define GREEN 2  // Макрос зеленого кольору для RGB світлодіода з 4 порта (Macro for Green Color of the RGB LED on Port 4)
#define BLUE 3   // Макрос синього кольору для RGB світлодіода з 4 порта (Macro for Blue Color of the RGB LED on Port 4)
#define YELLOW 4 // Макрос жовтого кольору для RGB світлодіода з 4 порта (Macro for Yellow Color of the RGB LED on Port 4)
#define CYAN 5   // Макрос голубого кольору для RGB світлодіода з 4 порта (Macro for Cyan Color of the RGB LED on Port 4)
#define PURPLE 6 // Макрос пурпурового кольору для RGB світлодіода з 4 порта (Macro for Purple Color of the RGB LED on Port 4)
#define WHITE 7  // Макрос білого кольору для RGB світлодіода з 4 порта (Macro for White Color of the RGB LED on Port 4)

#define RGB_RED   P4_2    // Пін червоного кольору RGB-світлодіода (Pin for the Red Color of the RGB LED)
#define RGB_GREEN P4_3    // Пін зеленого кольору RGB-світлодіода (Pin for the Green Color of the RGB LED)
#define RGB_BLUE  P4_4    // Пін синього кольору RGB-світлодіода (Pin for the Blue Color of the RGB LED)

#define PIR_PIN   P5_1    // Пін для PIR датчика руху (Pin for PIR Motion Sensor)
#define SOUND_PIN P5_2    // Пін для датчика звуку (Pin for Sound Sensor)
#define BUZ_PIN   P5_3    // Пін для базера (Pin for Buzzer)
#define LINE_PIN  P5_4    // Пін для датчика лінії (Pin for Line Sensor)

#define ACTIVE_BUZZER 1
// #define PASSIVE_BUZZER 1

#define ALERT 1
#define KEYTAP 2 
#define ACS_DNID 3
#define ACS_GRNT 4

#define DHT11_PIN P6_1    // Цифровий пін для DHT 11 (Digital Pin for DHT11 Sensor)
#define MQ7_PIN   P6_2    // Аналоговий пін для датчика газу (Analog Pin for Gas Sensor)
#define LIGHT_PIN P6_3    // Цифровий пін для датчика світла (Digital Pin for Light Sensor)

// #define LIGHT_4pin 1

#define C4        P10_4
#define C3        P10_3
#define C2        P10_2
#define C1        P10_1

#define R1        P7_4
#define R2        P7_3
#define R3        P7_2
#define R4        P7_1

#define TRIG_PIN  P9_3
#define ECHO_PIN  P9_4 
#define CLK       P9_1
#define DIO       P9_2

// #define ESP_CONTROL 1 //Якщо в розумному будинку використовується WiFi то має бути розкоментовано якщо ні - закоментуйте
// #define BT_CONTROL 1  //Якщо в розумному будинку використовується BlTh то має бути розкоментовано якщо ні - закоментуйте

void port_1_init(void);
void port_2_init(void);
void port_3_init(void);
void port_4_init(void);
void port_5_init(void);
void port_6_init(void); 
void port_9_init(void);

void traffic_light(uint8_t color);

void rgb4_set(uint8_t color);

void initdisplay(void);

void displaySensors(void);
void C_O_filter(void);
void window(void);
void parkin(void);

void step_forward(void);
void step_backward(void);
void step_lock(void);

void buzz_sound(uint8_t sound);

void keypad(void);
void processNumberKey(char key);
void checkPassword(void);
void resetPassword(void);

bool locck(void);

void lock_home(void);

