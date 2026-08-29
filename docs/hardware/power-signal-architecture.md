# Preliminary Power & Signal Architecture

## Power Architecture
```text
Laptop USB-C
    |
    v
ESP32-S3 DevKit
    |
    +---- 3.3 V ----> BME280
    |
    +---- 3.3 V ----> INMP441
    |
    +---- GND -------+
                     |
External 5 V --------+----> SN74AHCT125
                     |
                     +----> WS2812B

All Device grounds share a common ground reference.
```
## Signal Architecture
```text
BME280
  SDA <----------> ESP32-S3
  SCL <----------- ESP32-S3

INMP441
  SCK <----------- ESP32-S3
  WS  <----------- ESP32-S3
  SD  ------------> ESP32-S3

ESP32-S3 GPIO
      |
      v
SN74AHCT125
      |
      v
WS2812B DIN

Mic Mute Switch --------> ESP32-S3 GPIO
Cloud Enable Switch ----> ESP32-S3 GPIO

Future:
ESP32-S3 Native USB-C <------> Raspberry Pi 5 USB Host
```

### Notes
- BME280 & INMP441 operate from ESP32-S3 3.3V rail
- WS2812B & SN74AHCT125 operate from 5V supply
- All grounds must be common
- Exact GPIO assignments documented separately in 'pin-map.md'
