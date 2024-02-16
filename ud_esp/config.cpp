#include "NanitColors.hpp"
#include "Arduino.h"
#include "config.hpp"

Servo servo;
DHT dht(DHT11_PIN, DHT11);
TM1637Display tm(CLK, DIO);

// Масив пінів крокового двигуна для зручності у програмуванні
// Array of Pins for the Stepper Motor for Programming Convenience
const byte step_pin[4]{IN1, IN2, IN3, IN4};

bool                        //
    manual_control = false, /**< Ознака що ми керуємо в ручному режимі
                                A sign that we are driving in manual mode */

    fan_manual_on = false, /**< Вентилятор увімкнено мануально
                                The fan is turned on manually */
    window_manual_open = false, /**< Вікно відриємо по вланому бажанню
                                     We open the window as desired */
    parkin_flag = false, 
    light = false,
    lock_flag = true,
    window_flag = false,
    prev_window = false;

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

// Масив для налаштування сегментів у слово STOP
// Array for Configuring Segments to Form the Word 'STOP'
const uint8_t word_stop[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,          // S
  SEG_D | SEG_E | SEG_F | SEG_G,                  // t
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G           // P
};

// Масив для налаштування сегментів у слово WAIT
// Array for Configuring Segments to Form the Word 'WAIT'
const uint8_t word_wait[] = {
  SEG_B | SEG_D | SEG_F | SEG_G,                  // W
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,  // A
  SEG_B | SEG_C,                                  // I
  SEG_D | SEG_E | SEG_F | SEG_G,                  // t
};

// Масив для налаштування сегментів у слово OPEN
// Array for Configuring Segments to Form the Word 'OPEN'
const uint8_t word_open[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,          // P
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,          // E
  SEG_C | SEG_E | SEG_G,                          // n
};

// Масив для налаштування сегментів у слово BLOC
// Array for Configuring Segments to Form the Word 'BLOC'
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

// Масиви пінів для рядків та стовпців (Arrays of pins for rows and columns)
byte rowPins[ROWS] = { R1, R2, R3, R4 };
byte colPins[COLS] = { C1, C2, C3, C4 };
// Створення об'єкту класу Keypad(Creating a Keypad class object)
Keypad customKeypad = Keypad(makeKeymap(hexaKeys),rowPins, colPins, ROWS, COLS);
// Створення об'єкту Password з паролем (Creating an Object of Password with a password)
Password password = Password("123A"); 
// Змінні для довжин буфера введених символів (Variables for the Length of the Input Character Buffer)
byte maxPasswordLength = 6, currentPasswordLength = 0;

// String newPassword;  // зберігає новий пароль (holds the new password)
// char passchars[6];   // символи для newPassword (characters of newPassword)

void keypad(void) {
  // Зчитування натиснутої клавіші (Reading the pressed key)
  char Key = customKeypad.getKey();
  // Коли кнопка натиснута (When the button is pressed...)
  if (Key) {
    buzz_sound(KEYTAP);
    switch (Key) { // Розгалуження для призначення дій на кнопки (Branching for Assigning Actions to Buttons)
// Кнопка ʼ*ʼ натиснута - скидається буфер паролю (If the '*' button is pressed, the password buffer is cleared.)
      case '*': resetPassword(); break;
// Кнопка ʼ#ʼ натиснута - вмикається алгоритм перевірки введеного паролю
// If the '#' button is pressed, the password validation algorithm is activated.
      case '#': checkPassword(); break;
// При інших кнопках йде запис зчитанної клавіши у буфер паролю
// For other buttons, the pressed key is recorded in the password buffer.
      default: processNumberKey(Key);
    }
// Під час натискання кнопки вмикається на короткий період синій колір RGB-4
    rgb4_set(BLUE);
    delay(50);
    rgb4_set(OFF);
// "During button press, the RGB-4 LED briefly turns on in blue color."
  }
}

void processNumberKey(char key) {
  currentPasswordLength++;
  // Запис отриманного символа в буфер паролю
  // Recording the received character in the password buffer
  password.append(key); 
  if (currentPasswordLength == maxPasswordLength) {
    // Якщо довжина буферу більше 6 символів, то виконати перевірку 
    // If buffer length is more than 6, perform password check
    checkPassword();
  }
}

void checkPassword(void) {
  // Функція перевірки буферу паролю (Function for Checking the Password Buffer)
  if (password.evaluate()) {
  // Дії, якщо пароль вірно введений (Actions to take if the password is correctly entered)
    for (byte i = 0; i <= 6; i++) {
    // Цикл повідомлень перемикання режиму блокування (Loop of Messages for Toggling the Locking Mode)
      rgb4_set(YELLOW);
      buzz_sound(ACS_GRNT);
      delay(25);
      rgb4_set(OFF);
      buzz_sound(ACS_GRNT);
    }
  // Перемикання стану режиму блокування (Toggling the Locking Mode State)
    lock_flag = !lock_flag;
  // Наступні дії при обох станах режиму блокування (Subsequent actions in both locking mode states)
    if (lock_flag) { 
  // Якщо стан режиму блокування "блокування знято" (If the locking mode state is 'lock released'...)
      tft.fillScreen(background_screen);
      tft.setTextColor(~background_screen);
    } 
    else {
  // Якщо стан режиму блокування "блокування включено" ("If the locking mode state is 'lock enabled'...")
      tm.clear();
      tm.setSegments(seg_bloc);
      tft.fillScreen(ST7735_RED);
      tft.setTextColor(ST7735_WHITE);
    }
  }
  // Дії, якщо пароль невірно введений
  // Actions to take if the password is incorrectly entered
  else {
    tft.fillScreen(ST7735_RED);
    tft.setCursor(25, 20);
    tft.setTextSize(4);
    tft.setTextColor(ST7735_WHITE);
    tft.print("WRONG");
    tft.setCursor(35, 50);
    tft.print("PASS");
    for (byte i = 0; i <= 5; i++) {
  // Цикл повідомлень "невірно введений пароль" ("Loop of Messages 'Incorrect Password'")
      rgb4_set(PURPLE);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(RED);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(OFF);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(RED);
      buzz_sound(ACS_DNID);
      delay(10);
      rgb4_set(PURPLE);
      buzz_sound(ACS_DNID);
      delay(10);
      tm.clear();
      tm.setSegments(seg_bloc);
    }
    lock_flag = 0;
  }
  // Скидання буферу паролю після перевірки його (Clearing the password buffer after its verification)
  resetPassword();
}

// Функція скидання буферу пароля (Function for Clearing the Password Buffer)
void resetPassword() {
  password.reset();  // Метод скидання буфера для пароля (Method for Clearing the Password Buffer)
  currentPasswordLength = 0; // Скидання змінної для розміру буфера (Resetting the Variable for Buffer Size)
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
  for (short k = 0; k < 4; k++)   // Налаштування пінів другого порта для крокового мотора через цикл та масив
  {                               // Setting up the pins of the port 2 for a stepper motor using a "for" and an array
    pinMode(step_pin[k], OUTPUT); // Налаштування пінів на режим "вихід" (Setting Pins to Output Mode)
    digitalWrite(step_pin[k], 0); 
    // Вимикання пінів для базового положення шлагбаума (Turning Off Pins for the Barrier Gate to set base position)
  }
}

void port_3_init(void) {  // Функція ініціалізації 3-го порта (Initialization Function for Port 3)
  byte pin_port[3] = { 0 }; // Масив для пінів світлофора (Array for Traffic Light Pins)
#ifdef SEPARATE_LEDS_3   // Якщо світлофор складається з 3-х світлодіодів (If the traffic light consists of 3 LEDs...)
  pin_port[0] = TL_RED;  // Йде заповнення масиву макросами пінів (Populating the Array with Pin Macros)
  pin_port[1] = TL_YLW;
  pin_port[2] = TL_GRN;
#elif defined(RGB_LED_3) // Якщо світлофор це RGB-світлодіод (If the traffic light is an RGB LED...)
  pin_port[0] = RGB_RDL; // Йде заповнення масиву макросами пінів (Populating the Array with Pin Macros)
  pin_port[1] = RGB_GRN;
  pin_port[2] = RGB_BLU;
#endif
// Налаштування режиму пінів через цикл for на вихід (Setting Pin Modes to Output Using a 'for' Loop)
  for (short i = 0; i < 3; i++) {pinMode(pin_port[i], OUTPUT);}
  traffic_light(RED); // Світлофор вмикає червоний колір (The traffic light turns on the red color)
}

void port_4_init(void) {       // Функція ініціалізації 4-го порта (Initialization Function for Port 4)
  pinMode(RGB_RED, OUTPUT);    // Налаштування піна червоного кольору RGB (Configuration of the Red Color Pin for RGB)
  pinMode(RGB_GREEN, OUTPUT);  // Налаштування піна зеленого кольору RGB (Configuration of the Green Color Pin for RGB)
  pinMode(RGB_BLUE, OUTPUT);   // Налаштування піна синього кольору RGB (Configuration of the Blue Color Pin for RGB)
  rgb4_set(GREEN);
}

void port_5_init(void) {      // Функція ініціалізації 5-го порта (Initialization Function for Port 5)
  pinMode(PIR_PIN, INPUT);    // Налаштування піна для PIR датчика руху (Setting up the Pin for the PIR Motion Sensor)
  pinMode(SOUND_PIN, INPUT);  // Налаштування піна для датчика звуку (Setting up the Pin for the Sound Sensor)
  pinMode(BUZ_PIN, OUTPUT);   // Налаштування піна для базера (Setting up the Pin for the Buzzer)
  pinMode(LINE_PIN, INPUT);   // Налаштування піна для датчика лінії (Setting up the Pin for the Line Sensor)
  digitalWrite(BUZ_PIN, HIGH);   // Вимикання базера (Turning Off the Buzzer)
}

void buzz_sound(uint8_t sound) { // Функція звуку для базера (Sound Function for the Buzzer)
  switch (sound) { // Розгалуження для звуків базера (Branching for Buzzer Sounds)
    case ALERT: // Звук тривоги для базера (Alert Sound for the Buzzer)
#ifdef ACTIVE_BUZZER // Якщо базер активний (If the buzzer is active...)
      for (int i = 0; i < 600; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(400);
      }
#elif defined(PASSIVE_BUZZER) // Якщо базер пасивний (If the buzzer is passive...)
      tone(BUZ_PIN, 250);
      delay(240);
      noTone(BUZ_PIN);
#endif
      break;
    case KEYTAP: // Звук клавіатури для базера (Keytap Sound for the Buzzer)
#ifdef ACTIVE_BUZZER // Якщо базер активний (If the buzzer is active...)
      for (int i = 0; i < 400; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(100);
      }
#elif defined(PASSIVE_BUZZER) // Якщо базер пасивний (If the buzzer is passive...)
      tone(BUZ_PIN, 1200);
      delay(40);
      noTone(BUZ_PIN);
#endif
      break;
    case ACS_DNID: // Звук "доступ заборонено" для базера (Sound 'Access Denied' for the Buzzer)
#ifdef ACTIVE_BUZZER // Якщо базер активний (If the buzzer is active...)
      for (int i = 0; i < 700; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(350);
      }
#elif defined(PASSIVE_BUZZER) // Якщо базер пасивний (If the buzzer is passive...)
      tone(BUZ_PIN, 180);
      delay(290);
      noTone(BUZ_PIN);
#endif
      break;
    case ACS_GRNT: // Звук "доступ дозволено" для базера (Sound 'Access Granted' for the Buzzer)
#ifdef ACTIVE_BUZZER // Якщо базер активний (If the buzzer is active...)
      for (int i = 0; i < 2000; i++) {
        digitalWrite(BUZ_PIN, !digitalRead(BUZ_PIN));
        delayMicroseconds(75);
      }
#elif defined(PASSIVE_BUZZER) // Якщо базер пасивний (If the buzzer is passive...)
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


void port_9_init(void) { // Функція ініціалізації 9-го порта (Initialization Function for Port 9)
// Налаштування піна TRIG для датчика відстані на вхід (Setting the TRIG Pin of the Distance Sensor as Input)
  pinMode(TRIG_PIN, OUTPUT);
// Налаштування піна ECHO для датчика відстані на вихід (Setting the ECHO Pin of the Distance Sensor as Output)
  pinMode(ECHO_PIN, INPUT);
// Вимкнення TRIG піна для датчика відстані (Turning Off the TRIG Pin for the Distance Sensor)
  digitalWrite(TRIG_PIN, LOW);

// Метод для задачі яскравості сегментного дисплею (Method for Setting the Brightness of the Segment Display) 
  tm.setBrightness(0x0f);
// Метод для виведення новоутворенного слова на дисплеї (Method for Displaying custom Word on the Display)      
  tm.setSegments(word_stop);   
}

// Функція ініціалізації та основних налаштувань дисплею Nanit'у
// Function for Initialization and Basic Settings of the Nanit Display
void initdisplay(void) {
// Стартова ініціалізація Nanit разом з усіма налаштуваннями
// Initial Setup and Initialization of Nanit Display with All Settings
  Nanit_Base_Start();
// Розгалуження налаштувань темної або світлої теми на екрані
// Branching for Dark or Light Theme Settings on the Screen
  if (light) {
    background_screen = ST7735_BLACK;
    tft.setTextColor(ST7735_WHITE);
  } else {
    background_screen = ST7735_WHITE;
    tft.setTextColor(ST7735_BLACK);
  }
// Привітання текстов на дві секунди та очищенння екрану
// Displaying a Text Greeting for Two Seconds and Clearing the Screen
  tft.fillScreen(background_screen);
  tft.setCursor(10, 10);
  tft.print("Hello Nanit");
  delay(2000);
  tft.fillScreen(background_screen);
}
/**
 * @brief Send sensors values to serial port
 * @param sender Serial stream
 */
void sentSensors(HardwareSerial &sender) {
  /* Опорне значення для АЦП 5 вольт передавати ми будемо у мілівольтах 5000
   * (примаючий скрипт приймає тільки цілі значення).
   *
   * АЦП який ви використовуємо 10 бітний. Для визначення максимального значення
   * яке може видати перетворювач використаємо операцію побітого зсуву 1 на 10
   * розрядів у гору і віднімемо 1 в результаті отримаємо максимальне значення
   * яке може видати перетворювач (1023)
   *
   * Ключове слово constexpr вимагає щоб компілятор спробував обчислити
   * значення, щоб мікроконтролер не займався обчисленням числа
   */
  /* We will transmit the reference value for the ADC of 5 volts in 5000
   * millivolts (the receiving script accepts only integer values).
   *
   * The ADC that you use is 10 bit. To determine the maximum value
   * which can be issued by the converter, we will use the broken shift
   * operation 1 by 10 digits uphill and subtract 1, resulting in the maximum
   * value which can be issued by the converter (1023)
   *
   * The constexpr keyword requires the compiler to attempt to calculate
   * value so that the microcontroller does not calculate the number
   */
  constexpr int mVperBit =
      5000 / ((1 << 10) - 1); /** < роздільна здатність аналогово- цифрового
                               перетворювача у мілівольтах. Після обчислення
                               компілятором значення константи буде 4

                               analog-digital resolution
                               converter in millivolts. After calculation
                               the compiler value of the constant will be 4*/

  /** Послідовно передаємо значення */
  /** We pass values sequentially */
  sender.print((int)prev_tmp); /**< Надсилаємо температуру 
                                    We send the temperature */
  /** Розділяючи їх комою з пробілом
      Separating them with a comma and a space
  */
  const char _sep_[] = ", ";
  sender.print(_sep_);
  sender.print((int)prev_hum); /**< Надсилаємо вологість
                                    We send humidity */
  sender.print(_sep_);
  sender.print((int)prev_gas); /** < Надсилаємо рівень газу
                                     We send the gas level */
  sender.print(_sep_);
  sender.print((int)NanitRobot::Nanit::getNanit()
                   .getBatteryPower()); /**< Надсилаємо рівень зяряду
                                             We send the charge level */
  sender.print(_sep_);
  sender.print((int)(analogRead(BATTERY_PIN) *
                     mVperBit)); /**< Надсилаємо напругу на батареї
                                      We send voltage to the battery*/
  sender.println();
}

void getControl(HardwareSerial &sender) {
  /**  @brief Перелік параметрів які передаються в порт  */
  enum {
    MAN_CTRL = 0,    //
    FAN_MAN_CTRL,    //
    WINDOW_MAN_CTRL, //
    RESERVED,        //
    GATE_MAN_CTRL,   //
    LAST_VALUE //
  };

  /**
   * Якщо в порт щось прилітає починаємо аналузвати
   *
   */
  if (0 != sender.available()) {
    String input_string = Serial3.readStringUntil(
        '\n'); /** < Читаємо до символу завершення рядка */
    char *str = strdup(input_string.c_str());
    char *token = strtok(str, ","); /** < виділяємо перший елемент що
                                    закінчується комою або до кінця рядка */
    byte index = 0; /** < лічильник елементів */
    int values[LAST_VALUE]; /** < оргінзуємо масив для зберігання чисел */
    while ((NULL != token) &&
           (index < LAST_VALUE)) /**  < Повторюватимемо дії поки не закінчиться
                                    рядок або місце у масиві */
    {
      values[index++] = atoi(
          token); /** < шукаємо числа у рядку, якщо знаходимо занписуємо у першу
                     комірку і одразу перводимо вказівник на наступну комірку */
      token = strtok(NULL, ","); /** < шукаємо цифри у до наступної коми */
    }
    /** Якщо ми щось записали в масив присвоюємо ці значення змінним для подальшого*/
    manual_control = values[MAN_CTRL];
    fan_manual_on = values[FAN_MAN_CTRL];
    window_manual_open = values[WINDOW_MAN_CTRL];
  }
  // manual_control
  // fan_manual_on
  // window_manual_open
}

void displaySensors(void) 
{ // Функція виведення інформації з датчиків (Function for Displaying Sensor Information)
// Метод для задання розміру тексту на екрані (Method for Setting Text Size on the Screen)
  tft.setTextSize(2);
// Блок дій виведення строчки показника газу (Block of Actions for Displaying the Gas Gauge Indicator)
  tft.setCursor(10, 10);
  last_gas = analogRead(MQ7_PIN);
  tft.print("CO2: ");
  tft.print(prev_gas);
  tft.print(" ppm");
// Кінець блоку (End of block)
#ifdef LIGHT_4pin
  tft.setCursor(10, 30);
  last_gas = analogRead(MQ7_PIN);
  tft.print("LDR: ");
  tft.print(map(prev_ldr, 0, 1023, 100, 0));
#endif
// Блок дій виведення строчки показника температури ("Block of Actions for Displaying the Temperature Gauge Indicator")
  tft.setCursor(10, 50);
  last_tmp = dht.readTemperature();
  tft.print("Temp:");
  tft.print(prev_tmp);
  tft.drawCircle(139, 51, 2, ~background_screen);  // print degree symbol ( ° )
  tft.print(" C");
// Кінець блоку (End of block)
// Блок дій виведення строчки показника вологості (Block of Actions for Displaying the Humidity Gauge Indicator)
  tft.setCursor(10, 70);
  last_hum = dht.readHumidity();
  tft.print("Hmdt:");
  tft.print(prev_hum);
  tft.print(" %");
// Кінець блоку

// Блоки дій оновлення показників на екрані
// Blocks of Actions for Updating Indicators on the Screen
  delay(100);
// Блок для показника газу (Block for the Gas Indicator)
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
// Блок для показника температури (Block for the Temperature Indicator)
  if (prev_tmp != last_tmp) {
    tft.fillRect(70, 50, 70, 15, background_screen);
    prev_tmp = last_tmp;
  }
// Блок для показника вологості (Block for the Humidity Indicator)
  if (prev_hum != last_hum) {
    tft.fillRect(70, 70, 70, 15, background_screen);
    prev_hum = last_hum;
  }
// Блок для перемикання режиму темної теми в залежності показника датчика світла
// Block for Switching the Dark Mode based on the Light Sensor Reading
  if (digitalRead(LIGHT_PIN) != light) {
    light = digitalRead(LIGHT_PIN);
    background_screen = ~background_screen;
    tft.setTextColor(~background_screen);
    tft.fillScreen(background_screen);
  }

  sentSensors(Serial3); /**< відправляємо дані
                             we send data */
}

// Функція для двигуна ПС, який виступає в якості системи вентиляції
// Function for a DC Motor Acting as a Ventilation System
void AirQuality_Fan(void) {
  bool fan_is_on; /**< Вентилятор увімкнений
                       The fan is on */
  if (manual_control) 
  /* Якщо ручне керування то 
     If manual control then */
  {
    fan_is_on = fan_manual_on;
  } else
  /* Якщо працює автоматика
     If the automation works */ 
  {
    // Коли показники якості повітря поза норми двигун вмикається
    // When Air Quality Readings are Out of Normal Range, the Motor Turns On
    if (prev_gas > 200 || prev_tmp > 33 || prev_hum > 66)
      fan_is_on = true;
    else
      // Інакше, коли показники якості в рамках норми двигун вимкнений
      // Otherwise, When Air Quality Readings are Within Normal Range, the
      // Motor is Turned Off
      fan_is_on = false;
  }
  /*
    Посилаємо визначений сигнал на пін
    We send the specified signal to the pin
  */
  digitalWrite(MOTOR1_A, fan_is_on);
}

// Функція для серво, який використовуються для механізму шторки вікна
// Function for Servo Used in Window Curtain Mechanism
void window(void) {
  if (manual_control) {
  /* Якщо ручне керування то 
     If manual control then */
    window_flag = window_manual_open;
  } 
  /* Якщо працює автоматика
    If the automation works */ 
  else {
    // Якщо датчик світла подає сигнал "світло", то вікно відкрите
    // If the Light Sensor Signals 'Light', Then the Window is Opened
    if (!digitalRead(LIGHT_PIN)) {
      window_flag = true;
    }

    // Якщо датчик лінії бачить перед собою об'єкт, то вікно закривається
    // If the Line Sensor Detects an Object in Front, Then the Window is Closed
    if (!digitalRead(LINE_PIN)) {
      window_flag = false;
    }
  }
  // Положення сервомотора відносно отриманної інформації з датчиків світла та
  // лінії Position of the Servo Motor Relative to the Information Received from
  // Light and Line Sensors
  servo.write((window_flag) ? 90 : 0);
}

// Функція вмикання кольору світлофора (Function for Turning On the Traffic Light Color)
void traffic_light(uint8_t color) {

#if defined(SEPARATE_LEDS_3) // Дії при 3-х світлодіодах (Actions for 3 LEDs)
  switch (color) { // Розгалуження дій для кольорів (Branching of Actions for Colors)
    case RED: // Червоний колір світлофора (Red Color of the Traffic Light)
      digitalWrite(TL_RED, HIGH);
      digitalWrite(TL_YLW, LOW);
      digitalWrite(TL_GRN, LOW);
      break;
    case YELLOW: // Жовтий колір світлофора (Yellow Color of the Traffic Light)
      digitalWrite(TL_RED, LOW);
      digitalWrite(TL_YLW, HIGH);
      digitalWrite(TL_GRN, LOW);
      break;
    case GREEN: // Зелений колір світлофора (Green Color of the Traffic Light)
      digitalWrite(TL_RED, LOW);
      digitalWrite(TL_YLW, LOW);
      digitalWrite(TL_GRN, HIGH);
      break;
  }

#elif defined(RGB_LED_3) // Дії при RGB світлодіоді (Actions for an RGB LED)
  switch (mode) { // Розгалуження дій для кольорів (Branching of Actions for Colors)
    case RED: // Червоний колір світлофора (Red Color of the Traffic Light)
      digitalWrite(RGB_RDL, HIGH);
      digitalWrite(RGB_GRN, LOW);
      digitalWrite(RGB_BLU, LOW);
      break;
    case YELLOW: // Жовтий колір світлофора (Yellow Color of the Traffic Light)
      digitalWrite(RGB_RDL, HIGH);
      digitalWrite(RGB_GRN, HIGH);
      digitalWrite(RGB_BLU, LOW);
      break;
    case GREEN: // Зелений колір світлофора (Green Color of the Traffic Light)
      digitalWrite(RGB_RDL, LOW);
      digitalWrite(RGB_GRN, HIGH);
      digitalWrite(RGB_BLU, LOW);
      break;
  }
#endif
}

void rgb4_set(uint8_t color){ // Функція вмикання кольорів для RGB світлодіода (Function for Turning On Colors for an RGB LED)
  switch (color) { // Розгалуження для кольорів (Branching for Colors)
    case OFF: // Вимкнути RGB світлодіод (Turn off the RGB LED)
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case RED: // Червоний колір RGB світлодіода (Red Color of the RGB LED)
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case GREEN: // Зелений колір RGB світлодіода (Green Color of the RGB LED)
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, HIGH);
      digitalWrite(RGB_BLUE, LOW);
      break;
    case BLUE: // Синій колір RGB світлодіода (Blue Color of the RGB LED)
      digitalWrite(RGB_RED, LOW);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, HIGH);
      break;
    case YELLOW: // Жовтий колір RGB світлодіода (Yellow Color of the RGB LED)
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, 0);
      break;
    case CYAN: // Голубий колір RGB світлодіода (Cyan Color of the RGB LED)
      digitalWrite(RGB_RED, 0);
      digitalWrite(RGB_GREEN, HIGH);
      digitalWrite(RGB_BLUE, HIGH);
      break;
    case PURPLE: // Пурпоровий колір RGB світлодіода (Purple Color of the RGB LED)
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, LOW);
      digitalWrite(RGB_BLUE, HIGH);
      break;
    case WHITE: // Білий колір RGB світлодіода (WHITE Color of the RGB LED)
      digitalWrite(RGB_RED, HIGH);
      digitalWrite(RGB_GREEN, HIGH);
      digitalWrite(RGB_BLUE, HIGH);
      break;
  }
}

// Функція оберту крокового двигуна на 90 градусів за годинниковою стрілкою
// Function for Rotating the Stepper Motor 90 Degrees Clockwise
void step_forward(void) {
// Для оберту на 90 градусів кроковий двигун має пройти 128 кроків
// For a 90-degree rotation, the stepper motor needs to complete 128 steps.
  for (byte k = 0; k < 128; k++) 
  { // Один крок для двигуна - подача сигналу на кожен пін на 3мс по черзі
    // One step for the motor is the 3ms activation of each pin by setting it to HIGH in sequence.
    for (short i = 0; i <= 3; i++) {
      digitalWrite(step_pin[i], HIGH);
      delay(3);
      digitalWrite(step_pin[i], LOW);
    }
  }
}

// Функція оберту крокового двигуна на 90 градусів проти годинникової стрілки
// Function for a 90-Degree Counterclockwise Rotation of the Stepper Motor
void step_backward(void) {
// Для оберту на 90 градусів кроковий двигун має пройти 128 кроків
// For a 90-degree rotation, the stepper motor needs to complete 128 steps.
  for (byte k = 0; k < 128; k++) 
  { // Один крок для двигуна - подача сигналу на кожен пін на 3мс по черзі
    // One step for the motor is the 3ms activation of each pin by setting it to HIGH in sequence.
    for (short i = 3; i >= 0; i--) {
      digitalWrite(step_pin[i], HIGH);
      delay(3);
      digitalWrite(step_pin[i], LOW);
    }
  }
}

// Функція для блокування шлагбаума (Function for Barrier Gate Locking)
void step_lock(void) {for (short k = 0; k < 4; k++) { digitalWrite(step_pin[k], LOW); }}

///!todo Не закінчене керування вототами

// Основна функція роботи шлагбаума в парі з датчиком відстані, світлофором та 7-сегментним екраном
// Main Function for Barrier Gate Operation in Conjunction with Distance Sensor, Traffic Light, and 7-Segment Display
void parkin(void) {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  int cm = pulseIn(ECHO_PIN, 1) / 58;

  if (cm < 5 && parkin_flag == false) {
    tm.clear();
    tm.setSegments(word_wait);
    traffic_light(YELLOW);
    step_backward();
    tm.clear();
    tm.setSegments(word_open);
    traffic_light(GREEN);
    parkin_flag = true;
  } else if (cm > 5 && parkin_flag == true) {
    tm.clear();
    tm.setSegments(word_wait);
    traffic_light(YELLOW);
    step_forward();
    traffic_light(RED);
    tm.clear();
    tm.setSegments(word_stop);
    parkin_flag = false;
  }
}
// Функція режиму блокування будинку (Function for Home Locking Mode)
void lock_home(void) {
  servo.write(0); // Скидання положення серво для вікна (Resetting the Servo Position for the Window)
  tm.setSegments(seg_bloc); // Виведення слова "BLOC" на 7-сегментному екрані (Displaying the Word 'BLOC' on the 7-Segment Display)
  step_lock(); // Функція блокування шлагбауму для гаражу (Function for Garage Barrier Gate Locking)
  // Блок дій при виявленні посторонній шумів та рухів (Actions Block upon Detection of Foreign Noises and Movements)
  if (digitalRead(PIR_PIN) || !digitalRead(SOUND_PIN) || !digitalRead(LINE_PIN)) {
    for (byte i = 0; i < 2; i++) { buzz_sound(ALERT); }
    tft.setCursor(10, 40);
    tft.setTextSize(3);
    tft.setTextColor(ST7735_WHITE);
    tft.print("ALARM!!!");
  }
  delay(100); // Час затримки для опитування датчиків звуку, руху та ліній (Delay Time for Polling Sound, Motion, and Line Sensors)
  tft.fillScreen(ST7735_RED); // Очищення екрану червоним фоном (Clearing the Screen with a Red Background)
}

