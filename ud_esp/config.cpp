#include "Arduino.h"
#include "config.hpp"

Servo servo;
DHT dht(DHT11_PIN, DHT11);
TM1637Display tm(CLK, DIO);

const byte step_pin[4]{ IN1, IN2, IN3, IN4 };

bool
  parkin_flag = 0,
  light = 0,
  lock_flag = 1,
  window_flag = 0,
  prev_window = 0;

int
  prev_gas = 0,
#ifdef LIGHT_4pin
  prev_ldr = 0,
  last_ldr = 0,
#endif
  last_gas = 0;

float
  prev_tmp = 0,
  last_tmp = 0,
  prev_hum = 0,
  last_hum = 0;

#ifdef LIGHT_4pin
uint16_t light_start = 0;
#endif
uint16_t background_screen = 0;

const uint8_t word_stop[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,          // S
  SEG_D | SEG_E | SEG_F | SEG_G,                  // t
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G           // P
};

const uint8_t word_wait[] = {
  SEG_B | SEG_D | SEG_F | SEG_G,                  // W
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
  SEG_B | SEG_C,                                  // I
  SEG_D | SEG_E | SEG_F | SEG_G,                  // t
};

const uint8_t word_open[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,          // E
  SEG_C | SEG_E | SEG_G,                          // n
};

const uint8_t seg_bloc[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,  // B
  SEG_E | SEG_D | SEG_F,                                  // L
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,          // O
  SEG_A | SEG_D | SEG_E | SEG_F,                          // C
};

const byte ROWS = 4;  // Кількість рядків у клавіатурі (Number of rows in the keypad)
const byte COLS = 4;  // Кількість стовпців у клавіатурі (Number of columns in the keypad)
// Масив символів для клавіш(Array of characters)
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
// Масив пінів для рядків та стовпців (Array of pins for rows and columns)
byte rowPins[ROWS] = { R1, R2, R3, R4 };
byte colPins[COLS] = { C1, C2, C3, C4 };
// Створення об'єкту класу Keypad(Creating a Keypad class object)
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
Password password = Password("123A");
String newPassword;  // зберігає новий пароль (holds the new password)
char passchars[6];   // символи для newPassword (characters of newPassword)
byte maxPasswordLength = 6;
byte currentPasswordLength = 0;


void keypad(void) {
  char Key = customKeypad.getKey();  // Зчитування натиснутої клавіші (Reading the pressed key)
  if (Key) {
    buzz_sound(KEYTAP);
    switch (Key) {
      case '*': resetPassword(); break;
      case '#': checkPassword(); break;
      default: processNumberKey(Key);
    }
    rgb4_set(0, 0, 1);
    delay(50);
  }
}

void processNumberKey(char key) {
  currentPasswordLength++;
  password.append(key);
  if (currentPasswordLength == maxPasswordLength) {
    // Якщо довжина пароля більше 6, то виконати перевірку (If password length is more than 6, perform password check)
    checkPassword();
  }
}

void checkPassword(void) {
  if (password.evaluate()) {
    for (byte i = 0; i <= 6; i++) {
      rgb4_set(1, 1, 0);
      buzz_sound(ACS_GRNT);
      delay(25);
      rgb4_set(0, 0, 0);
      buzz_sound(ACS_GRNT);
    }
    lock_flag = !lock_flag;
    if (lock_flag) {
      tft.fillScreen(background_screen);
      tft.setTextColor(~background_screen);
    } else {
      tm.clear();
      tm.setSegments(seg_bloc);
      tft.fillScreen(ST7735_RED);
      tft.setTextColor(ST7735_WHITE);
    }
  } else {
    tft.fillScreen(ST7735_RED);
    tft.setCursor(25, 20);
    tft.setTextSize(4);
    tft.setTextColor(ST7735_WHITE);
    tft.print("WRONG");
    tft.setCursor(35, 50);
    tft.print("PASS");
    for (byte i = 0; i <= 5; i++) {
      rgb4_set(1, 0, 1);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(1, 0, 0);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(0, 0, 0);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(1, 0, 0);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(1, 0, 1);
      buzz_sound(ACS_DNID);
      delay(10);
      tm.clear();
      tm.setSegments(seg_bloc);
    }
    lock_flag = 0;
  }
  resetPassword();
}

void resetPassword() {
  password.reset();  // Скидання пароля (Reset password)
  currentPasswordLength = 0;
}

bool locck(void) {
  return lock_flag;
}

void port_1_init(void) {
  servo.attach(SERVO_PIN);        // Приєднання сервоприводу до піна P1_1 під назвою SERVO_PIN мікроконтролера.
  pinMode(MOTOR1_A, OUTPUT);      // Налаштування піна MOTOR1_A як вихідний (Setting pin MOTOR1_A as output)
  pinMode(MOTOR1_B, OUTPUT);      // Налаштування піна MOTOR1_B як вихідний (Setting pin MOTOR1_B as output)
  pinMode(MOTOR_ENABLE, OUTPUT);  // Налаштування піна MOTOR_ENABLE як вихідний (Setting pin MOTOR_ENABLE as output)
  digitalWrite(MOTOR_ENABLE, 1);  // Включення вбудованого драйвера (Turning on the built-in driver)
  digitalWrite(MOTOR1_A, 0);      // Зупинка двигунa (Stop Motor)
  digitalWrite(MOTOR1_B, 0);      // На два канали  (On both channels)
  servo.write(0);                 // Початкове положення сервоприводу (Initial position of the Servo)
}

void port_2_init(void) {
  for (short k = 0; k < 4; k++)  // Налаштування пінів другого порта для крокового мотора через цикл та масив
  {                              // (Setting up the pins of the port 2 for a stepper motor using a "for" and an array)
    pinMode(step_pin[k], OUTPUT);
    digitalWrite(step_pin[k], 0);
  }
}

void port_3_init(void) {  // Функція ініціалізації 3-го порта (Initialization Function for Port 3)
  byte pin_port[3] = { 0 };
#ifdef separate_leds3
  pin_port[0] = TL_RED;
  pin_port[1] = TL_YELLOW;
  pin_port[2] = TL_GREEN;
#elif defined(rgb_led3)
  pin_port[0] = RGB_RDL;
  pin_port[1] = RGB_GRN;
  pin_port[2] = RGB_BLU;
#endif
  for (short i = 0; i < 3; i++) {
    pinMode(pin_port[i], OUTPUT);
    digitalWrite(pin_port[i], 0);
  }
  traffic_light(RED);
}

void port_4_init(void) {       // Функція ініціалізації 4-го порта (Initialization Function for Port 4)
  pinMode(RGB_RED, OUTPUT);    // Налаштування піна червоного кольору RGB (Configuration of the Red Color Pin for RGB)
  pinMode(RGB_GREEN, OUTPUT);  // Налаштування піна зеленого кольору RGB (Configuration of the Green Color Pin for RGB)
  pinMode(RGB_BLUE, OUTPUT);   // Налаштування піна синього кольору RGB (Configuration of the Blue Color Pin for RGB)
  rgb4_set(0, 0, 0);
}

void port_5_init(void) {      // Функція ініціалізації 5-го порта (Initialization Function for Port 5)
  pinMode(PIR_PIN, INPUT);    // Налаштування піна для PIR датчика руху (Setting up the Pin for the PIR Motion Sensor)
  pinMode(SOUND_PIN, INPUT);  // Налаштування піна для датчика звуку (Setting up the Pin for the Sound Sensor)
  pinMode(BUZ_PIN, OUTPUT);   // Налаштування піна для базера (Setting up the Pin for the Buzzer)
  pinMode(LINE_PIN, INPUT);   // Налаштування піна для датчика лінії (Setting up the Pin for the Line Sensor)
  digitalWrite(BUZ_PIN, 1);   // Вимикання базера (Turning Off the Buzzer)
}

void buzz_sound(uint8_t sound) {
  switch (sound) {
    case ALERT:
#ifdef active_buz
      for (int i = 0; i < 600; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(400);
      }
#elif defined(pasive_buz)
      tone(BUZ_PIN, 250);
      delay(240);
      noTone(BUZ_PIN);
#endif
      break;
    case KEYTAP:
#ifdef active_buz
      for (int i = 0; i < 400; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(100);
      }
#elif defined(pasive_buz)
      tone(BUZ_PIN, 1200);
      delay(40);
      noTone(BUZ_PIN);
      break;
#endif
    case ACS_DNID:
#ifdef active_buz
      for (int i = 0; i < 700; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(350);
      }
#elif defined(pasive_buz)
      tone(BUZ_PIN, 180);
      delay(290);
      noTone(BUZ_PIN);
#endif
      break;
    case ACS_GRNT:
#ifdef active_buz
      for (int i = 0; i < 2000; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(75);
      }
#elif defined(pasive_buz)
      tone(BUZ_PIN, 800);
      delay(150);
      noTone(BUZ_PIN);
#endif
      break;
  }
}

void port_6_init(void) {           // Функція ініціалізації 6-го порта (Initialization Function for Port 6)
  dht.begin();                     // Метод ініціалізації датчика DHT11 (Method for Initializing the DHT11 Sensor)
  pinMode(MQ7_PIN, INPUT);         // Налаштування піна для датчика газу (Configuration of the Pin for the Gas Sensor)
  pinMode(LIGHT_PIN, INPUT);       // Налаштування піна для датчика світла (Configuration of the Pin for the Light Sensor)
  light = digitalRead(LIGHT_PIN);  // Зчитування початкового стану світла (Reading the Initial State of the Light)
}

void port_9_init(void) {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, 0);
  tm.setBrightness(0x0f);
  tm.setSegments(word_stop);
}

void initdisplay(void) {
  Nanit_Base_Start();
  if (light) {
    background_screen = ST7735_BLACK;
    tft.setTextColor(ST7735_WHITE);
  } else {
    background_screen = ST7735_WHITE;
    tft.setTextColor(ST7735_BLACK);
  }
  tft.fillScreen(background_screen);
  tft.setCursor(10, 10);
  tft.print("Hello Nanit");
  delay(2000);
  tft.fillScreen(background_screen);
}

void displaySensors(void) {
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  last_gas = analogRead(MQ7_PIN);
  tft.print("CO2: ");
  tft.print(prev_gas);
  tft.print(" ppm");
#ifdef LIGHT_4pin
  tft.setCursor(10, 30);
  last_gas = analogRead(MQ7_PIN);
  tft.print("LDR: ");
  tft.print(map(prev_ldr, 0, 1023, 100, 0));
#endif
  tft.setCursor(10, 50);
  last_tmp = dht.readTemperature();
  tft.print("Temp:");
  tft.print(prev_tmp);
  tft.drawCircle(139, 51, 2, ~background_screen);  // print degree symbol ( ° )
  tft.print(" C");
  tft.setCursor(10, 70);
  last_hum = dht.readHumidity();
  tft.print("Hmdt:");
  tft.print(prev_hum);
  tft.print(" %");
  delay(100);

  if (prev_gas != last_gas) {
    tft.fillRect(70, 10, 55, 15, background_screen);
    prev_gas = last_gas;
  }
#ifdef LIGHT_4pin
  if (prev_ldr != last_ldr) {
    tft.fillRect(70, 30, 55, 15, background_screen);
    prev_ldr = last_ldr;
  }
#endif
  if (prev_tmp != last_tmp) {
    tft.fillRect(70, 50, 70, 15, background_screen);
    prev_tmp = last_tmp;
  }
  if (prev_hum != last_hum) {
    tft.fillRect(70, 70, 70, 15, background_screen);
    prev_hum = last_hum;
  }
  if (digitalRead(LIGHT_PIN) != light) {
    light = digitalRead(LIGHT_PIN);
    background_screen = ~background_screen;
    tft.setTextColor(~background_screen);
    tft.fillScreen(background_screen);
  }
}

void C_O_filter(void) {
  if (prev_gas > 200 || prev_tmp > 33 || prev_hum > 66) {
    digitalWrite(MOTOR1_A, 1);
  } else {
    digitalWrite(MOTOR1_A, 0);
  }
}

void window(void) {
  if (!digitalRead(LIGHT_PIN)) { window_flag = 1; }
  if (!digitalRead(LINE_PIN)) { window_flag = 0; }
  servo.write((window_flag) ? 90 : 0);
}

void traffic_light(uint8_t mode) {
#if defined(separate_leds3)
  switch (mode) {
    case RED:
      digitalWrite(TL_RED, 1);
      digitalWrite(TL_YELLOW, 0);
      digitalWrite(TL_GREEN, 0);
      break;
    case YELLOW:
      digitalWrite(TL_RED, 0);
      digitalWrite(TL_YELLOW, 1);
      digitalWrite(TL_GREEN, 0);
      break;
    case GREEN:
      digitalWrite(TL_RED, 0);
      digitalWrite(TL_YELLOW, 0);
      digitalWrite(TL_GREEN, 1);
      break;
  }
#elif defined(rgb_led3)
  switch (mode) {
    case RED:
      digitalWrite(RGB_RDL, 1);
      digitalWrite(RGB_GRN, 0);
      digitalWrite(RGB_BLU, 0);
      break;
    case YELLOW:
      digitalWrite(RGB_RDL, 1);
      digitalWrite(RGB_GRN, 1);
      digitalWrite(RGB_BLU, 0);
      break;
    case GREEN:
      digitalWrite(RGB_RDL, 0);
      digitalWrite(RGB_GRN, 1);
      digitalWrite(RGB_BLU, 0);
      break;
  }
#endif
}

void rgb4_set(bool r_led, bool g_led, bool b_led) {
  digitalWrite(RGB_RED, r_led);
  digitalWrite(RGB_GREEN, g_led);
  digitalWrite(RGB_BLUE, b_led);
}

void step_forward(void) {
  for (byte k = 0; k < 128; k++) {
    for (short i = 0; i <= 3; i++) {
      digitalWrite(step_pin[i], 1);
      delay(3);
      digitalWrite(step_pin[i], 0);
    }
  }
}

void step_backward(void) {
  for (byte k = 0; k < 128; k++) {
    for (short i = 3; i >= 0; i--) {
      digitalWrite(step_pin[i], 1);
      delay(3);
      digitalWrite(step_pin[i], 0);
    }
  }
}

void step_lock(void) {
  for (short k = 0; k < 4; k++) { digitalWrite(step_pin[k], 0); }
}

void parkin(void) {
  digitalWrite(TRIG_PIN, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, 0);
  int cm = pulseIn(ECHO_PIN, 1) / 58;

  if (cm < 5 && parkin_flag == 0) {
    tm.clear();
    tm.setSegments(word_wait);
    traffic_light(YELLOW);
    step_backward();
    tm.clear();
    tm.setSegments(word_open);
    traffic_light(GREEN);
    parkin_flag = 1;
  } else if (cm > 5 && parkin_flag == 1) {
    tm.clear();
    tm.setSegments(word_wait);
    traffic_light(YELLOW);
    step_forward();
    traffic_light(RED);
    tm.clear();
    tm.setSegments(word_stop);
    parkin_flag = 0;
  }
}

void lock_home(void) {
  servo.write(0);
  tm.setSegments(seg_bloc);
  step_lock();
  if (digitalRead(PIR_PIN) || !digitalRead(SOUND_PIN) || !digitalRead(LINE_PIN)) {
    for (byte i = 0; i < 2; i++) { buzz_sound(ALERT); }
    tft.setCursor(10, 40);
    tft.setTextSize(3);
    tft.setTextColor(ST7735_WHITE);
    tft.print("ALARM!!!");
  }
  delay(100);
  tft.fillScreen(ST7735_RED);
}
