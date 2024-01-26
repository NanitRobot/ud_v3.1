<center> 

# Nanit Smart Home 

</center>

![Real Photo](/images/ud_v3.1.png)

Вітаємо. Даний репозиторій представляє собою загальну інформацію про збирання схеми. Оскільки вся інформація по усім модулям знаходиться на самому курсі а також як зібрати готовий екземпляр як на фото. Тут буде лише покрокова інформація про збирання схеми. 

## Загальна схема розумного будинку

![Real Photo](/images/UD_StandAlone_Full.png)

Дана схема - кінцевий результат збирання макету будинку. Але бажано покроково збирати схему. Цим і займемось.

Але для початку треба внести ясність до схеми. Колір дроту який з'єднує пін безпосередньо не важливо, та обрано було можна сказати "на власний розсуд". Але якщо це світлодіод або двигун, то йде сувора послідовність. Тому уважно звертайте увагу на з'єднання подібних компонентів. А також піни живлення: 
- <span style="color: red;">Червоний дріт</span> -> `+5V/VCC`
- **Чорний дріт** ->  &mdash; або `GND`

## Порт 1: Двигуни

![Port 1](/images/UD_StandAlone_Port1.png)

На перший порт має бути підключені <u>**Двигун ПС**</u> для вентиляції та <u>**Сервомотор**</u> для шторки вікна. В даному проекті полярність двигуна буде незначна.

| Пін компонента | Макрос піна в коді |
| ----------- | ----------- |
| `Servo: Signal` | `P1_1` |
| `DC Motor A` | `P1_4 / MOTOR1_A` | 
| `DC Motor B` | `P1_3 / MOTOR1_A` | 

## Порт 2: Кроковий двигун

![Port 2](/images/UD_StandAlone_Port2.png)

Цей порт буде зайнятий цілком **Кроковим двигуном**, який буде підключений через драйвер. Все дуже просто:

| Пін драйвера | Макрос піна в NanitLib |
| ----------- | ----------- | 
| `IN1` | `P2_1` |
| `IN2` | `P2_2` | 
| `IN3` | `P2_3` |
| `IN4` | `P2_4` |

## Порт 3: Світлофор

![Port 3](/images/UD_StandAlone_Port3.png)

На даному порті буде світлофор зроблений з 3-х `світлодіодів`. Бажано перед цим спаяти світлодіоди з <u>резисторами</u> номіналом `220Ом` та дроти. Як було зазначено раніше, колір дрота йде відповідно до кольору світлодіода. Для зручності ось таблиця пінів

| Колір світлодіда | Макрос піна в NanitLib |
| ----------- | ----------- | 
| <font color="red">Red LED</font> | `P3_2` |
| <font color="yellow">Yellow LED</font> | `P3_3` |
| <font color="green">Green LED</font> | `P3_4` |

## Порт 4: Світлофор

![Port 4](/images/UD_StandAlone_Port4.png)

На відміну від `Світлофора` не потрібно нічого паяти, бо вже все було спаяно на модулі виробником. Однак тут хочу застережити що на даній схемі потрібно звертати увагу на примітки модуля як на схемі так і вживу. Але код написано саме таким чином як вказано нижче:

| Колір RGB | Макрос піна в NanitLib |
| ----------- | ----------- | 
| <font color="red">Red LED</font> | `P4_2` |
| <font color="green">Green LED</font> | `P4_3` |
| <font color="blue">Blue LED</font> | `P4_4` |

На малюнку схемі розпіновка йде в такому порядку `B`(синій колір), `G`(зелений колір), `R`(червоний колір) та `-`(маса/мінус). За часту все йде в порядку `R` `G` `B` `-`. Звертайте увагу на маркування ніжок.

## Порт 5: PIR, Датчики звуку та лінії, Buzzer

![Port 5](/images/UD_StandAlone_Port5.png)

Далі йдуть датчики `sensors` разом з базером. Але перед цим рекомендую відрегулювати чутливість компараторів для датчиків звуку та лінії. Бо в першому випадку при звуковій сигналізації датчик може на це зреагувати і сигналізація буде циклічна. у другому випадку датчик лінії не зможе закрити шторку вікна.

Таблиця пінів на порті:

| Піни модулів | Макрос піна в NanitLib |
| ----------- | ----------- | 
| `PIR Motion` | `P5_1` |
| `Sound sensor` | `P5_2` |
| `Buzzer` | `P5_3` |
| `Line sensor` | `P5_4` |

Тут показано в скороченому форматі підключення живлення модулів. У реальності можна спокійно на кожен модуль виводити лінії живлення. Пінів вистачить.

## Порт 6: DHT11, GAS MQ7 and LDR sensors

![Port 6](/images/UD_StandAlone_Port6.png)

На відміну від порта 5 тут калібрування датчиків необов'язкове. Але важливо що датчики аналогові, але не з усіх потрібно читати саме аналогові сигнали. Наприклад, датчик світла може інформувати про рівень освітленості але в цьому проекті нам потрібно лише знати два стани: `темно` та `світло`.

Таблиця пінів на порті:

| Піни модулів | Digital or Analog | Макрос піна в NanitLib |
| ----------- | ----------- | ----------- | 
| `DHT 11 sensor` | Digital | `P6_1` |
| `Gas-MQ7 sensor ` | Analog | `P6_2` |
| `LDR sensor` | Digital | `P6_3` |

## Порт 9: UltraSonic HC-SR04 та 7-сегментний дисплей TM1637

![Port 9](/images/UD_StandAlone_Port9.png) 

Тут нічого складного не буде. Але важливо не переплутати піни для драйвера TM1637 інакше інформації на дисплеї не буде.

Таблиця пінів на порті:

| Піни модулів | Макрос піна в NanitLib |
| ----------- | ----------- |  
| `TM1637 CLK` | `P9_1` |
| `TM1637 DIO` | `P9_2` |
| `UltraSonic Trig` | `P9_3` |
| `UltraSonic Echo` | `P9_4` |

## Порт 7 та 10: Клавіатура

![Port 9](/images/UD_StandAlone_Keypad.png)

Для клавіатури не потрібне живлення однак тут важливо в правильному порядку підключити піни портів. У випадку якщо піни будуть наплутані, то клавіши будуть невірно передані `Nanit`'у. Тому уважно підключайте клавіатуру.

Таблиця підключень клавіатури:

| № | Піни клавіатури | Макрос піна в NanitLib |
| ----------- | ----------- | ----------- |  
| 1 | `R1` | `P7_4` |
| 2 | `R2` | `P7_3` |
| 3 | `R3` | `P7_2` |
| 4 | `R4` | `P7_1` |
| 5 | `C1` | `P10_1` |
| 6 | `C2` | `P10_2` |
| 7 | `C3` | `P10_3` |
| 8 | `C4` | `P10_4` |

---

## Висновок

Після збирання схеми можете спокійно прошити Nanit та протестувати весь функціонал розумного будинку.