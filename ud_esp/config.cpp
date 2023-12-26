#include "config.hpp"

Servo servo;
Password password = Password("123A");
TinyStepper stepper(HALFSTEPS, IN1, IN2, IN3, IN4);
DHT dht(DHT11_PIN, DHT11);
TM1637D tm(CLK, DIO);

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
  stepper.Enable();
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
  tm.begin();
  tm.setBrightness(4);
  digitalWrite(TRIG_PIN, 0);
  tm.display("STOP");
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

void parkin(void) {
  digitalWrite(TRIG_PIN, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, 0);
  int cm = pulseIn(ECHO_PIN, 1) / 58;

  if (cm < 10 && parkin_flag == 0) {
    // tm.clearScreen();
    tm.display("OPEN");
    stepper.Move(-90);
    delay(2000);
    parkin_flag = 1;
  } else if (cm > 10 && parkin_flag == 1) {
    // tm.clearScreen();
    tm.display("STOP");
    stepper.Move(90);
    delay(2000);
    parkin_flag = 0;
  }
}


void setPassword(char* pass) {
  password.set(pass);
}
