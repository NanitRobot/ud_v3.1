#include "config.hpp"

Servo servo;
Password password = Password("123A");
DHT dht(DHT11_PIN, DHT11);
TM1637Display tm(CLK, DIO);

const byte step_pin[4]{IN1,IN2,IN3,IN4};

bool
  parkin_flag = 0;

int
  background_screen = 0,
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

const uint8_t word_stop[] ={
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,            // S
  SEG_D | SEG_E | SEG_F | SEG_G,                    // t
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G             // P
};

const uint8_t word_wait[] ={
  SEG_B | SEG_D | SEG_F | SEG_G,                    // W
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,    // A
  SEG_B | SEG_C,                                    // I
  SEG_D | SEG_E | SEG_F | SEG_G,                    // t
};

const uint8_t word_open[] ={
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,            // P
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,            // E
  SEG_C | SEG_E | SEG_G,                            // n
};

void port_1_init(void) {
  servo.attach(SERVO_PIN);
  pinMode(MOTOR1_A, OUTPUT);
  pinMode(MOTOR1_B, OUTPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);
  digitalWrite(MOTOR_ENABLE, 1);
  digitalWrite(MOTOR1_A, 0);
  digitalWrite(MOTOR1_B, 0);
  servo.write(0);
}

void port_2_init(void) {
  for(short k =0; k<4; k++){
    pinMode(step_pin[k],OUTPUT);
    digitalWrite(step_pin[k], 0);
  }
}

void port_3_init(void) {
#ifdef separate_leds3
  pinMode(TL_RED, OUTPUT);
  pinMode(TL_YELLOW, OUTPUT);
  pinMode(TL_GREEN, OUTPUT);
  digitalWrite(TL_RED | TL_YELLOW | TL_GREEN, 0);
#elif rgb_led3
  pinMode(RGB_RDL, OUTPUT);
  pinMode(RGB_GRN, OUTPUT);
  pinMode(RGB_BLU, OUTPUT);
  digitalWrite(RGB_RDL | RGB_GRN | RGB_BLU, 0);
#endif
  traffic_light(1, 0, 0);
}

void port_4_init(void) {
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  digitalWrite(RGB_GREEN, 1);
  digitalWrite(RGB_RED | RGB_BLUE, 0);
}

void port_5_init(void) {
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(BUZ_PIN, OUTPUT);
  pinMode(LINE_PIN, INPUT);
  digitalWrite(BUZ_PIN, 1);
}

void port_6_init(void) {
  dht.begin();
  pinMode(MQ7_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);
#ifdef LIGHT_4pin
  light_start = analogRead(LIGHT_PIN);
#endif
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
  if (digitalRead(LIGHT_PIN)) {
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
  tft.drawCircle(139, 51, 2, ST7735_WHITE);  // print degree symbol ( ° )
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
    if (digitalRead(LIGHT_PIN)) {
    background_screen =!background_screen;
    delay(10);
    tft.setTextColor(!background_screen);
    tft.fillScreen(background_screen);
  } 
  else 
  {
    background_screen = ST7735_WHITE;
    tft.setTextColor(ST7735_BLACK);
  }
}

void C_O_filter(void) {
  if (prev_gas > 120 || prev_tmp > 33 || prev_hum > 66) {
    digitalWrite(MOTOR1_A, 1);
  } else {
    digitalWrite(MOTOR1_A, 0);
  }
}

void window(void) {
  servo.write((digitalRead(LINE_PIN) ? 0 : 90));
}

void traffic_light(bool r_led, bool y_led, bool g_led) {
  digitalWrite(TL_RED, r_led);
  digitalWrite(TL_YELLOW, y_led);
  digitalWrite(TL_GREEN, g_led);
}

void step_forward(void){
  for(byte k = 0; k < 32; k++)
  {
    for(short i = 0; i<4; i++)
    {
      digitalWrite(step_pin[i], 1);
      delay(5);
      digitalWrite(step_pin[i],0);
    }
  }
}
void step_backward(void){
  for(byte k = 0; k < 32; k++)
  {
    for(short i = 3; i>=0; i--)
    {
      digitalWrite(step_pin[i], 1);
      delay(5);
      digitalWrite(step_pin[i],0);
    }
  }
}
void parkin(void) {
  digitalWrite(TRIG_PIN, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, 0);
  int cm = pulseIn(ECHO_PIN, 1) / 58;

  if (cm < 5 && parkin_flag == 0) {
    tm.clear();
    tm.setSegments(word_wait);
    traffic_light(0, 1, 0);
    step_backward();
    tm.clear();
    tm.setSegments(word_open);
    traffic_light(0, 0, 1);
    parkin_flag = 1;
  } else if (cm > 5 && parkin_flag == 1) {
    tm.clear();
    tm.setSegments(word_wait);
    traffic_light(0, 1, 0);
    step_forward();
    traffic_light(1, 0, 0);
    tm.clear();
    tm.setSegments(word_stop);
    parkin_flag = 0;
  }
}


void setPassword(char* pass) {
  password.set(pass);
}
