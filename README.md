[Читати мене Українською](README_uk.md)

<center>

# Nanit Smart Home

</center>

![Real Photo](/images/ud_v3.1.png)

Welcome!  
This repository provides general information about assembling the circuit.  
Since all detailed information about each module and the complete assembly (as shown in the photo) is provided in the course itself, this document contains only a **step-by-step guide** for assembling the circuit.

---

## 🏠 General Smart Home Circuit

![Real Photo](/images/UD_StandAlone_Full.png)

This diagram represents the **final assembled version** of the house model.  
However, it’s recommended to build the circuit step by step — which is exactly what we’ll do here.

Before starting, let’s clarify a few things:  
The color of the connecting wires is not important unless specified — it was chosen mostly for clarity.  
However, for **LEDs** or **motors**, the wiring order **must** be followed strictly.  
Also, remember the standard power pin colors:

- 🔴 **Red wire** → `+5V / VCC`  
- ⚫ **Black wire** → `—` or `GND`

---

## ⚙️ Port 1: Motors

![Port 1](/images/UD_StandAlone_Port1.png)

Port 1 should be connected to a **DC motor** for ventilation and a **servo motor** for the window curtain.  
In this project, the DC motor polarity is not critical.

| Component Pin | Code Macro |
| -------------- | ----------- |
| `Servo: Signal` | `P1_1` |
| `DC Motor A` | `P1_4 / MOTOR1_A` |
| `DC Motor B` | `P1_3 / MOTOR1_B` |

---

## ⚙️ Port 2: Stepper Motor

![Port 2](/images/UD_StandAlone_Port2.png)

This entire port is dedicated to the **stepper motor**, which connects via a driver.  
Simple and straightforward:

| Driver Pin | NanitLib Macro |
| ----------- | -------------- |
| `IN1` | `P2_1` |
| `IN2` | `P2_2` |
| `IN3` | `P2_3` |
| `IN4` | `P2_4` |

---

## 🚦 Port 3: Traffic Light

![Port 3](/images/UD_StandAlone_Port3.png)

This port is used for the **traffic light**, made from 3 LEDs.  
It’s recommended to **solder the LEDs** together with **220Ω resistors** and wires beforehand.  
As mentioned earlier, wire color should match the LED color.  
For convenience:

| LED Color | NanitLib Macro |
| ---------- | -------------- |
| 🔴 Red LED | `P3_2` |
| 🟡 Yellow LED | `P3_3` |
| 🟢 Green LED | `P3_4` |

---

## 🌈 Port 4: RGB Module

![Port 4](/images/UD_StandAlone_Port4.png)

Unlike the traffic light, no soldering is required here — everything is already preassembled on the module.  
However, pay attention to the module pin markings both on the schematic and on the physical board.  
The code is written assuming the following pin mapping:

| RGB Color | NanitLib Macro |
| ---------- | -------------- |
| 🔴 Red LED | `P4_2` |
| 🟢 Green LED | `P4_3` |
| 🔵 Blue LED | `P4_4` |

In the schematic, the pin order is `B` (blue), `G` (green), `R` (red), and `-` (ground).  
Usually, the order is `R`, `G`, `B`, `-`. Always check the pin labels carefully.

---

## 🎯 Port 5: PIR, Sound & Line Sensors, Buzzer

![Port 5](/images/UD_StandAlone_Port5.png)

This port connects multiple **sensors** along with the **buzzer**.  
Before using them, adjust the sensitivity of the **sound** and **line** sensor comparators.  
Otherwise, the sound sensor may repeatedly trigger the alarm, or the line sensor may fail to close the curtain properly.

| Module Pin | NanitLib Macro |
| ----------- | -------------- |
| `PIR Motion` | `P5_1` |
| `Sound Sensor` | `P5_2` |
| `Buzzer` | `P5_3` |
| `Line Sensor` | `P5_4` |

Power connections are shown in a simplified format here.  
In practice, you can provide separate power lines for each module — there are enough pins for that.

---

## 🌡️ Port 6: DHT11, Gas MQ7, and LDR Sensors

![Port 6](/images/UD_StandAlone_Port6.png)

Unlike Port 5, sensor calibration here is **not required**.  
Most of these sensors are **analog**, but not all need analog readings.  
For example, the light sensor only needs to detect two states: **dark** or **bright**.

| Module Pin | Digital/Analog | NanitLib Macro |
| ----------- | -------------- | -------------- |
| `DHT11 Sensor` | Digital | `P6_1` |
| `Gas MQ7 Sensor` | Analog | `P6_2` |
| `LDR Sensor` | Digital | `P6_3` |

---

## 📏 Port 9: UltraSonic HC-SR04 and 7-Segment Display TM1637

![Port 9](/images/UD_StandAlone_Port9.png)

This setup is simple, but make sure not to **swap the TM1637 driver pins**, otherwise nothing will display.

| Module Pin | NanitLib Macro |
| ----------- | -------------- |
| `TM1637 CLK` | `P9_1` |
| `TM1637 DIO` | `P9_2` |
| `UltraSonic Trig` | `P9_3` |
| `UltraSonic Echo` | `P9_4` |

---

## ⌨️ Ports 7 and 10: Keypad

![Keypad](/images/UD_StandAlone_Keypad.png)

The keypad does not require power, but the **pin order** is very important.  
If the pins are connected incorrectly, the keys will send the wrong values to the Nanit board.  
Connect carefully according to the table below:

| № | Keypad Pin | NanitLib Macro |
| - | ----------- | -------------- |
| 1 | `R1` | `P7_4` |
| 2 | `R2` | `P7_3` |
| 3 | `R3` | `P7_2` |
| 4 | `R4` | `P7_1` |
| 5 | `C1` | `P10_1` |
| 6 | `C2` | `P10_2` |
| 7 | `C3` | `P10_3` |
| 8 | `C4` | `P10_4` |

---

## ✅ Conclusion

Once the wiring is complete, you can safely **flash the Nanit board** and **test all smart home functions**.
