#include "config.hpp"

Servo servo;
Password password = Password("123A");
TinyStepper stepper(HALFSTEPS, IN1, IN2, IN3, IN4);
DHT dht(DHT11_PIN,DHT11);

uint16_t gas_start = 0;

#ifdef LIGHT_4pin
uint16_t light_start = 0;
#endif

void port_1_init()
{
    servo.attach(SERVO_PIN);
    pinMode(MOTOR1_A, OUTPUT);
    pinMode(MOTOR1_B, OUTPUT);
    pinMode(MOTOR_ENABLE, OUTPUT);
    digitalWrite(MOTOR_ENABLE, 1);
    digitalWrite(MOTOR1_A,0);
    digitalWrite(MOTOR1_B,0);
    servo.write(0);
}

void port_2_init()
{
    stepper.Enable();
}

void port_3_init()
{
    #ifdef separate_leds3
    pinMode(TL_RED, OUTPUT);
    pinMode(TL_YELLOW, OUTPUT);
    pinMode(TL_GREEN, OUTPUT);
    digitalWrite(TL_RED|TL_YELLOW | TL_GREEN, 0);
    #elif rgb_led3
    pinMode(RGB_RDL, OUTPUT);
    pinMode(RGB_GRN, OUTPUT);
    pinMode(RGB_BLU, OUTPUT);
    digitalWrite(RGB_RDL| RGB_GRN | RGB_BLU, 0);
    #endif
}

void port_4_init()
{
    pinMode(RGB_RED, OUTPUT);
    pinMode(RGB_GREEN, OUTPUT);
    pinMode(RGB_BLUE, OUTPUT);
    digitalWrite(RGB_GREEN,1);
    digitalWrite(RGB_RED| RGB_BLUE, 0);
}

void port_5_init()
{
    pinMode(PIR_PIN, INPUT);
    pinMode(SOUND_PIN, INPUT);
    pinMode(BUZ_PIN, OUTPUT);
    pinMode(LINE_PIN, INPUT);
}

void port_6_init()
{
    dht.begin();
    pinMode(MQ7_PIN,INPUT);
    pinMode(LIGHT_PIN, INPUT);
    gas_start = analogRead(MQ7_PIN);
    #ifdef LIGHT_4pin
    light_start = analogRead(LIGHT_PIN);
    #endif
}

void port_9_init()
{
    
}

void setPassword(char* pass)
{
    password.set(pass);
}
