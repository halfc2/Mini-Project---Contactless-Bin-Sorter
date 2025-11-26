# Digital Bin – Contactless Smart Trash Sorter

A quick and fun, pandemic-friendly, no-touch trash bin that:
- Automatically opens the correct lid when you hover your hand over **PLASTIC** or **PAPER**
- Detects when each bin is full using ultrasonic sensors
- Sends an SMS alert to the janitor when a bin is full
- Shows cute random Filipino/Gen-Z thank-you messages after every use
- Has a reset button for testing/demo

Perfect school project, makerspace demo, or anything you can think to make out if it.

## Features
- 100% contactless operation
- Two separate bins: **Plastic** and **Paper**
- SG90 servo flips the lid to the correct side
- 4× HC-SR04 ultrasonic sensors
- 16×2 I2C LCD with backlight
- GSM module (SIM800L/SIM900) sends SMS when full
- 15+ thank-you messages (feel free to replace or add with your own!)

## Hardware Required
| Component                  | Quantity | Notes                                      |
|----------------------------|----------|--------------------------------------------|
| Arduino Uno or Nano        | 1        |                                            |
| HC-SR04 Ultrasonic Sensor  | 4        | 2 for full detection, 2 for hand detection |
| SG90 Micro Servo           | 1        | Controls the lid                           |
| 16×2 I2C LCD (0x27)        | 1        |                                            |
| SIM800L or SIM900 GSM      | 1        | With valid SIM card and antenna            |
| Push button                | 1        | Connected to A2 (reset)                    |
| 10kΩ resistor (optional)   | 1        | Pull-down for button if not using INPUT_PULLUP |
| Power supply (5V 2A recommended) | 1  | GSM module is power hungry                 |

## Wiring Diagram
Sensor 1 (Plastic Full)   → Trig: D5   Echo: D6
Sensor 2 (Paper Full)     → Trig: D7   Echo: D8
Sensor 3 (Hand → Plastic) → Trig: D9   Echo: D10
Sensor 4 (Hand → Paper)   → Trig: D11  Echo: D12
Servo signal            → D3
Reset Button              → A2 (other side to GND)
LCD I2C                   → SDA → A4, SCL → A5
GSM Module TX/RX          → Arduino RX/TX (SoftwareSerial possible)

text## Pin Summary (Arduino Uno/Nano)
| Function                 | Pin  |
|--------------------------|------|
| Servo                    | D3   |
| Sensor 1 Trig/Echo       | D5/D6|
| Sensor 2 Trig/Echo       | D7/D8|
| Sensor 3 Trig/Echo       | D9/D10|
| Sensor 4 Trig/Echo       | D11/D12|
| Reset Button             | A2   |
| LCD I2C SDA/SCL          | A4/A5|
| GSM TX → Arduino RX      | D0   |
| GSM RX → Arduino TX      | D1   |

## How to Use
1. Power it up → "Starting..." appears in the display  
2. Hover hand over **left side** → lid opens to **PLASTIC**  
3. Hover hand over **right side** → lid opens to **PAPER**  
4. When a either bin gets full → LCD flashes warning + SMS sent  
5. Press the button on A2 to reset everything (for demo)

## Important Notes
- Change the phone number in `sendSMS()` function:
  ```cpp
  Serial.print(F("AT+CMGS=\"+639123456789\"\r"));  // ← put your number here
