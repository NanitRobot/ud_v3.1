#include <NanitLib.h>
#include <DHT.h>
#include <TM1637Display.h>
#include <Keypad.h>  
#include <Password.h>

#define SERVO_PIN P1_1 
//MOTOR1_A вже готовий дефайн P1_3 
//MOTOR1_B вже готовий дефайн P1_4

#define IN1       P2_1
#define IN2       P2_2
#define IN3       P2_3
#define IN4       P2_4

// #define rgb_led3 1
#define separate_leds3 1

#ifdef separate_leds3
#define TL_RED    P3_2
#define TL_YELLOW P3_3
#define TL_GREEN  P3_4
#elif defined(rgb_led3)
#define RGB_RDL P3_2
#define RGB_GRN P3_3
#define RGB_BLU P3_4
#endif

#define RGB_RED   P4_2
#define RGB_GREEN P4_3
#define RGB_BLUE  P4_4

#define PIR_PIN   P5_1
#define SOUND_PIN P5_2
#define BUZ_PIN   P5_3
#define LINE_PIN  P5_4

#define active_buz 1
// #define pasive_buz 1

#define DHT11_PIN P6_1
#define MQ7_PIN   P6_2
#define LIGHT_PIN P6_3

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

#if !defined(MEGACORE)
/** @todo  Замінити в коді макровизначення wifi_esp та bluetooth на
 * NANIT_USE_WIFI та NANIT_USE_BlTH відповідно
 *
 * Якщо код використовуватиметься з набором плат Nanit вибір варіанту коду буде
 * доступним через відповідні пункти меню. В інакшому разі потрібно
 * розкоментовувати одне з макровизнечень
 */
// #define NANIT_USE_WIFI ///<  Керуємо будинком через  Wi-Fi

// #define NANIT_USE_BlTH ///<  Управління через блютус

// #define wifi_esp 1   ///< Якщо в розумному будинку використовується WiFi то має бути розкоментовано якщо ні - закоментуйте 

// #define bluetooth 1  ///< Якщо в розумному будинку використовується BlTh то має бути розкоментовано якщо ні - закоментуйте
#endif

#if defined(NANIT_USE_WIFI) && defined(NANIT_USE_BlTH)
#error  Вибір одночасно двох варіацій програми поки не передбачено
#endif
void port_1_init(void);
void port_2_init(void);
void port_3_init(void);
void port_4_init(void);
void port_5_init(void);
void port_6_init(void); 
void port_9_init(void);

void traffic_light(bool r_led,bool y_led,bool g_led);

void rgb4_set(bool r_led,bool g_led, bool b_led);

void initdisplay(void);

void displaySensors(void);
void C_O_filter(void);
void window(void);
void parkin(void);

void step_forward(void);
void step_backward(void);
void step_lock(void);

#ifdef active_buz
void alert_buz(void);
void keytap_buz(void);
void access_grant_buz(void);
void access_denid_buz(void);
#endif

void keypad(void);
void processNumberKey(char key);
void checkPassword(void);
void resetPassword(void);

bool locck(void);

void lock_home(void);

