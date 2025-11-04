# ❤️ ESP32 MAX30102 Heart Rate & SpO₂ Monitor

This project uses the **MAX30102 optical sensor** and an **ESP32** to measure **heart rate (BPM)** and **blood oxygen saturation (SpO₂)** in real time.  
It uses SparkFun’s `MAX3010x` library for sensor control and `spo2_algorithm` for signal analysis.

---

## 🧠 Features
- Real-time heart rate and SpO₂ estimation  
- Automatic finger-contact detection using IR intensity  
- Degraded-signal warning based on IR range variability  
- Configurable sampling window and sensor parameters  
- Serial output with clean, readable formatting  

---

## ⚙️ Hardware Setup
| Component | Connection | ESP32 Pin |
|------------|-------------|-----------|
| **MAX30102 SDA** | I²C Data | GPIO 21 |
| **MAX30102 SCL** | I²C Clock | GPIO 22 |
| **VCC** | 3.3 V |
| **GND** | GND |

---

## 📦 Dependencies
Include these libraries in your Arduino IDE or PlatformIO environment:

```cpp
#include <Wire.h>
#include "MAX30105.h"        // SparkFun MAX3010x Pulse & Proximity Sensor Library
#include "spo2_algorithm.h"  // Include algorithm files in the same project folder
