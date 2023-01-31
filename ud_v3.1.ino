 /*
  * УД нанит, версия 3.1.
  * Рассчитана на 10 адаптеров.
  * Используется матричная клавиатура
  * и светофорные светодиоды.
  * 
  * 
  * PS: в это тестовая версия УД на 10 портов,
  * создана в демонстративных целях,
  * на матричной клавиатуре работают только кнопки
  * '2', '3', '0', 'D' - остальные молчат 
  * (проверил несколько клавиатур).
  * Возможно проблема в самих переходниках,
  * поэтому пока буду использовать пароль 230D + #
  * для блокировки/расблокировки секции охраны.
  */
 

 
 /*
  
  **********ЧТО ГДЕ РАЗМЕСТИТСЯ**********
  
 * на первом гнезде расположится серва и мотор постоянного тока, так как
 * на нем есть драйвер + питание на гнезде идет от батареи, так что
 * можно цеплять мощную нагрузку.
 
 * 2-е гнездо займет физический драйвер ULN2003 для шаговика,
 * там питание идет от батареи -> можно цеплять мощную нагрузку 
 
 * на третьем гнезде не используется пин P3_1, 
 * остальные можно под три отдельных 
 * светофорных светодиода
 
 * на 4-м гнезде есть три ШИМ-пина - как раз под RGB-светодиод

 * на 5-м гнезде будут располагаться базер, датчик звука,
 * датчик линии и инфракрасный датчик движения.
 * Есть 1 порт-прерывание - возможно отдам его PIRу.

 * на 6-м порту будут располагаться информационные датчики:
 * температура-влажность, свет, уровень угарного газа.
 * Информация будет выводится на дисплей нанита
 
 * 7-й и 10-й порт --- матричная клавиатура.

 * 8 порт не буду трогать - там серийный монитор,
 * неразведенный сброс. Что мне один оставшийся 
 * нещасный аналоговый пин?

 * на 9-м гнезде есть шина I2C - нету правда I2C-устройств, 
 * так что оно полностью пойдёт под ультрасоник и семисегментник
  
 * на 11-м гнезде есть UART3, буду общаться с блютуз-модулем через него
  
 * 12-й порт не используется
 
 */


#include <NanitLib.h>

#include <DHT.h>
#include <Servo.h>
#include <TM1637Display.h>
#include <Keypad.h>
#include <EEPROM.h>

#define SERVO_PIN P1_1 
//MOTOR1_A это уже готовый дефайн P1_3 
//MOTOR1_B это уже готовый дефайн P1_4


#define IN1       P2_4
#define IN2       P2_3
#define IN3       P2_2
#define IN4       P2_1


#define TL_RED    P3_2
#define TL_YELLOW P3_3
#define TL_GREEN  P3_4

#define RGB_RED   P4_2
#define RGB_GREEN P4_3
#define RGB_BLUE  P4_4

#define PIR_PIN   P5_1
#define SOUND_PIN P5_2
#define BUZ_PIN   P5_3
#define LINE_PIN  P5_4

#define DHT11_PIN P6_1
#define MQ7_PIN   P6_2
#define LIGHT_PIN P6_3

#define R1        P10_4
#define R2        P10_3
#define R3        P10_2
#define R4        P10_1

#define C1        P7_4
#define C2        P7_3
#define C3        P7_2
#define C4        P7_1

#define TRIG_PIN  P9_3
#define ECHO_PIN  P9_4 
#define CLK       P9_1
#define DIO       P9_2

#define V_BAT     A15 //служебный дефайн, если будет надо глянуть батарею

typedef enum{RED, YELLOW, GREEN}tl_color;                 // для светофорных светодиодов
typedef enum{UP, DOWN}step_pos;                           // для шаговика
typedef enum{OPENED, CLOSE}servo_pos;                       // для сервы
typedef enum{FIRST, SECOND, THIRD, FOURTH}mat_state;      // для матричной клавиатуры


const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {R1, R2, R3, R4};
byte colPins[COLS] = {C1, C2, C3, C4};

char init_pass[4] = {'2', '3', '0', 'D',};
char cur_pass[4];
int count = 0;
int limit_gas=0;
/*
 * Передаваемая по блютузу комманда с телефона:
 * 
 * 'L' - поставить/отключить охрану
 * 'm' - включить/выключить мотор
 * 'w' - открыть/закрыть окно
 */
char phone_but = 'x'; 
bool lockdown = 0;    //режим блокировки дома
bool lock_change = 0; 
bool manual_motor = 0;
bool manual_window = 0;
bool matrix_lock = 0;
int gas_level = 0;
int light_level = 0;
int temperature = 0;
int humidity = 0;
char key = 0;           //кнопка матрицы
step_pos prev_step = DOWN;   //для того, чтобы шаговик дважды не поднимался/опускался

const uint8_t seg_bloc[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // B
  SEG_E | SEG_D | SEG_F,                                 // L
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         // O
  SEG_A | SEG_D | SEG_E | SEG_F,                         // C
 };

const uint8_t seg_stop[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,         // S
  SEG_D | SEG_E | SEG_F | SEG_G,                 // T
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,         // P
};

const uint8_t seg_go[] = {
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,         // G
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         // O
};

Servo servo;
DHT dht(DHT11_PIN, DHT11);
TM1637Display tm = TM1637Display(CLK, DIO);
Keypad matrix = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/*******инициализаторы гнёзд нанита***********/
void port_1_init();
void port_2_init();
void port_3_init();
void port_4_init();
void port_5_init();
void port_6_init(); 
void port_9_init();


/*******функционал каждого гнезда (разделено пробелом)***********/
void motor_rotate(bool mspeed);
void window(servo_pos which);

void stepper_pos(step_pos which);

void traffic_light(tl_color which);

void rgb_write(byte red, byte green, byte blue); // зеленый с красным перепутались

void buz_pilik(byte times, int duration);

void port_6_info(); // поправить единицы измерения газа

int range_cm();

/*
 * служебные функции, на всякий случай если надо будет 
 * проверить напряжение/процент заряда на наните
 */
float check_bat();
void print_bat();

void setup() 
{
  Nanit_Base_Start();
  delay(7000);
  tft.fillScreen(ST7735_WHITE); //избавляемся от Hello Nanit
  Serial.begin(9600);
  Serial3.begin(9600);

  port_1_init();   
  port_2_init();  
  port_3_init();  
  port_4_init();
  port_5_init();
  port_6_init();  
  port_9_init();  

  motor_rotate(0);
  window(CLOSE);
  rgb_write(0, 0, 0);
  buz_pilik(2, 100); //верещит базер, если не пиликнуть им пару раз
  limit_gas=analogRead(MQ7_PIN)+20;
}


void loop() 
{
  key = matrix.getKey();
  if (key)
  {
    cur_pass[count++] = key;
    buz_pilik(1, 50);
  }
  if (count == 4)
  {
    count = 0;
    if (0 == strncmp(cur_pass, init_pass, 4))
    {
      buz_pilik(2, 50);
      matrix_lock = (matrix_lock == 0)? 1 : 0;
    }
  }
  
  if (Serial3.available())phone_but = Serial3.read();

  if (phone_but == 'L')
  {
    lockdown = (lockdown == 0)? 1 : 0;
    lock_change = 1;
  }
  else if (phone_but == 'm')
  {
    manual_motor = (manual_motor == 0)? 1 : 0;
  }
  else if (phone_but == 'w')
  {
    manual_window = !manual_window;
  }

  if (lockdown)
  {
    if (lock_change)
    {
      buz_pilik(2, 300);
      tm.setSegments(seg_bloc, 4, 0);
      tft.fillScreen(ST7735_RED);
      motor_rotate(0);
      window(CLOSE);
      stepper_pos(DOWN);
      rgb_write(255, 0, 0);
      
      lock_change = 0;
    }
    
  }
  else
  {
    if (lock_change)
    {
      lock_change = 0;
      tft.fillScreen(ST7735_WHITE);
    }
    
    //ПАНЕЛИ F, G (микрофон отдельно)
    port_6_info();
    
   //ПАНЕЛЬ Е   
    if (gas_level > limit_gas || manual_motor) motor_rotate(1);
    else motor_rotate(0);

    //ПАНЕЛЬ D
    if (prev_step==DOWN) {traffic_light(RED);} else {traffic_light(GREEN);}

    
    if (range_cm() < 10)
    {
      tm.clear();
      tm.setSegments(seg_go, 2, 0);
      stepper_pos(UP);
    }
    else
    {
      tm.clear();
      tm.setSegments(seg_stop, 4, 0);
      stepper_pos(DOWN);
    }

    //ПАНЕЛЬ С (+микрофон)

    if (digitalRead(PIR_PIN) && matrix_lock)
    {
      rgb_write(255, 0, 0);
      buz_pilik(3, 100); 
    }
    else rgb_write(0, 255, 0);

    //ПАНЕЛЬ В (+фоторезистор)
    if(manual_window){window(CLOSE);}  ///////////////////////////////////////////////Фоторезистор і вікно
    else {
     if (analogRead(LIGHT_PIN) <= 250){if(!digitalRead(LINE_PIN)){window(CLOSE);} else {window(OPENED);} }
     else window(CLOSE);
    }
    /*Serial.println();
    Serial.print("/b=");
    Serial.print(manual_window);
    Serial.print("/s=");
    Serial.print(analogRead(LIGHT_PIN));
        Serial.print("/l=");
    Serial.println(digitalRead(LINE_PIN));
    delay(1000);*/
    
  }/****КОНЕЦ УСЛОВИЯ "ЕСЛИ НЕ ЛОКДАУН"******/

  
  phone_but = 'x';
}/***КОНЕЦ LOOP****/




/*****************определения функций*****************/

void port_1_init()
{
  servo.attach(SERVO_PIN);
  servo.write(0);
  
  pinMode(MOTOR_ENABLE, OUTPUT);
  pinMode(MOTOR1_A, OUTPUT);
  pinMode(MOTOR1_B, OUTPUT);
  
  digitalWrite(MOTOR_ENABLE, HIGH);
  digitalWrite(MOTOR1_A, LOW);
  digitalWrite(MOTOR1_B, LOW);
  
}

void port_2_init()
{
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void port_3_init()
{
  pinMode(TL_RED, OUTPUT); 
  pinMode(TL_YELLOW, OUTPUT); 
  pinMode(TL_GREEN, OUTPUT); 
}

void port_4_init()
{
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);

  digitalWrite(RGB_RED, LOW);
  digitalWrite(RGB_GREEN, LOW);
  digitalWrite(RGB_BLUE, LOW);
}

void port_5_init()
{
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(BUZ_PIN, OUTPUT);
  pinMode(LINE_PIN, INPUT_PULLUP);
}

void port_6_init()
{
  pinMode(MQ7_PIN, INPUT);
  dht.begin();
  pinMode(LIGHT_PIN, INPUT);
}

void port_9_init()
{  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  tm.setBrightness(2);
  tm.clear();
}

void window(servo_pos which)
{
  if (which == OPENED)servo.write(100);  ///Серво відкривається
  else if (which == CLOSE)servo.write(0); ///Серво закривається
}

void motor_rotate(boolean mspeed)
{
  if (mspeed == 1)
  {
     digitalWrite(MOTOR1_A, HIGH);
     digitalWrite(MOTOR1_B, LOW);
  }
  else if (mspeed == 0)
  {
    digitalWrite(MOTOR1_A, LOW);
    digitalWrite(MOTOR1_B, LOW);
  }

}

/*
 * пока что руками приходится выставлять начальное 
 * положение шаговика (стыд-позор), хотя он чётко откалиброван
 * по времени на угол поворота в 90 градусов.
 * Хоть delay'ев нет, шаговик все равно
 * немного торомозит всю систему. В идеале
 * привязать бы его к другому таймеру, а не к millis'у.
*/
void stepper_pos(step_pos which)
{
  long lil_delay = millis();
  long step_delay = 0;
  
  int step_speed = 5;
  int step_total_time = 625;
  
  if (which == UP && prev_step == DOWN)
  {
    prev_step = UP;
    
    while (millis() - lil_delay < step_total_time)
    {    
      traffic_light(YELLOW);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);

      step_delay = millis();
      while (millis() - step_delay < step_speed);
      
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);

      step_delay = millis();
      while (millis() - step_delay < step_speed);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);

      step_delay = millis();
      while (millis() - step_delay < step_speed);

      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);

      step_delay = millis();
      while (millis() - step_delay < step_speed);
    }
  }
  else if (which == DOWN && prev_step == UP)
  {
    prev_step = DOWN;
    
    while (millis() - lil_delay < step_total_time)
    {
      traffic_light(YELLOW);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, HIGH);

      step_delay = millis();
      while (millis() - step_delay < step_speed);
      
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);

      step_delay = millis();
      while (millis() - step_delay < step_speed);

      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);

      step_delay = millis();
      while (millis() - step_delay < step_speed);

      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);

      step_delay = millis();
      while (millis() - step_delay < step_speed);
    }
  }
}


void traffic_light(tl_color which)
{
  if (which == RED)
  {
    digitalWrite(TL_RED, HIGH);
    digitalWrite(TL_YELLOW, LOW);
    digitalWrite(TL_GREEN, LOW);
  }
  else if (which == YELLOW)
  {
    digitalWrite(TL_RED, LOW);
    digitalWrite(TL_YELLOW, HIGH);
    digitalWrite(TL_GREEN, LOW);
  }
  else if (which == GREEN)
  {
    digitalWrite(TL_RED, LOW);
    digitalWrite(TL_YELLOW, LOW);
    digitalWrite(TL_GREEN, HIGH);
  }
}

void rgb_write(byte red, byte green, byte blue)
{
  analogWrite(RGB_RED, red);
  analogWrite(RGB_GREEN, green);
  analogWrite(RGB_BLUE, blue);
}

void buz_pilik(byte times, int duration)
{
  times = (times > 0)? times : 1;

  long lil;
  
  for (int i = 0; i < times; i++)
  {
    digitalWrite(BUZ_PIN, LOW);
    
    lil = millis();
    while (millis() - lil < duration);

    digitalWrite(BUZ_PIN, HIGH);

    lil = millis();
    while (millis() - lil < duration);
  }
}

void port_6_info()
{
  gas_level = analogRead(MQ7_PIN);
  light_level = analogRead(LIGHT_PIN);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  
  tft.setTextWrap(true);
  tft.setRotation(1);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.fillRect(55, 5, 60, 25, ST7735_WHITE);
  tft.print("CO2: ");
  tft.print(gas_level);
  tft.println(" ppm");

  tft.setCursor(10, 40);
  tft.fillRect(75, 35, 75, 25, ST7735_WHITE);
  tft.print("light: ");
  tft.println(light_level);
  
  tft.setCursor(10, 70);
  tft.fillRect(70, 65, 85, 25, ST7735_WHITE);
  tft.print("temp: ");
  tft.print(temperature);
  tft.println(" C");

  tft.setCursor(10, 100);
  tft.fillRect(60, 95, 90, 25, ST7735_WHITE);
  tft.print("hum: ");
  tft.print(humidity);
  tft.println(" %");
}

int range_cm()
{
  long duration = 0;
  int distance = 0;
  
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.034) / 2;

  return distance;
}


/***********служебные функции***************/
float check_bat()
{
  float bat_volt = 0;
  bat_volt = analogRead(V_BAT) * 5.0 / 1024.0;
  
  return bat_volt;
}

void print_bat()
{
  int bat_perc = 0;
  float bat_volt = check_bat();
  
  if (bat_volt <= 3.05) bat_perc = 0;
  else if (bat_volt > 3.05 && bat_volt <= 3.2) bat_perc = 10;
  else if (bat_volt > 3.2 && bat_volt <= 3.4) bat_perc = 20;
  else if (bat_volt > 3.4 && bat_volt <= 3.55) bat_perc = 30;
  else if (bat_volt > 3.55 && bat_volt <= 3.7) bat_perc = 40;
  else if (bat_volt > 3.7 && bat_volt <= 3.8) bat_perc = 50;
  else if (bat_volt > 3.8 && bat_volt <= 3.9) bat_perc = 60;
  else if (bat_volt > 3.9 && bat_volt <= 3.95) bat_perc = 70;
  else if (bat_volt > 3.95 && bat_volt <= 4.0) bat_perc = 80;
  else if (bat_volt > 4.0 && bat_volt <= 4.1) bat_perc = 90;
  else if (bat_volt > 4.1) bat_perc = 100;
  
  tft.setTextSize(1);
  tft.setCursor(0, 14);        
  tft.print(bat_volt); 
  tft.print('V');

    /*
    tft.setCursor(120, 14);
    tft.print(bat_perc);
    tft.print('%');
    */
}
