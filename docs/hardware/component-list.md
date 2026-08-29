# Hardware Components

### ESP32-S3
- ESP32-S3-DevKitC-1 compatible development board
- ESP32-S3-WROOM-1-N8R8
- 8 MB Flash
- 8 MB PSRAM

### Environmental Sensor
- BME280 3.3V breakout
- I2C/SPI
- Temperature, humidity, pressure

### Microphone
- INMP441 MEMS microphone
- I2S digital output

### Lighting
- Ws2812B
- 5V
- 60 individually addressable RGB LEDs/m

### Logic Interface
- SN74AHCT125N
- 3.3V to 5V digital buffering for WS2812B

### Supporting Components
- Electrolytic capacitor assortment
- Resistors
- Breadboards
- Jumper wires
- Toggle switches

# Electrical & Interface Requirements

## ESP32-S3 DevKitC-1 N8R8
**Purpose:**
- Primary embedded controller for sensing, local AI, and hardware control.
- Main hardware authority and FreeRTOS host.

**Electrical:**
- USB powered during development
- GPIO logic: 3.3V
- Memory: 8MB Flash, 8MB PSRAM

**Planned Interfaces:**
- I2C
- I2S
- GPIO
- Wi-Fi
- USB/UART


## BME280
**Purpose:**
- Temperature, humidity, and pressure sensing.

**Electrical:**
- Supply: 3.3V
- Logic: 3.3V
- Power Source: ESP32-S3 3.3V Rail

**Interface:**
- I2C: SDA (Bidirectional Data), SCL (Clock)

**ESP32-S3 Requirements**
- 2 GPIOs
- Common GND

## INMP441
**Purpose:**
- Digital MEMS microphone for voice/audio input.

**Electrical:**
- Supply: 1.8V to 3.3V
- Planned Operating Voltage: 3.3V
- Typical Current: ~2.2mA

**Interface:**
- I2S: SCK (Serial Clock), WS (Word-select), SD (Audio data output), L/R (Selects left or right I2S channel)

**Audio Format:**
- 24-bit I2S
- Two's-complement, MSB-first

**ESP32-S3 Requirements:**
- 3 GPIOs
- Common GND

## WS2812B
**Purpose:**
- Individually addressable RGB LED output for system status & lighting effects.

**Electrical:**
- Supply: 5V
- Logic Input: 5V
- Planned Power Source: external 5V supply

**Interface:**
- Signal-wire digital data
- DIN (data input)
- DOUT (data output to next LED)

**ESP32-S3 Requirements:**
- 1 GPIO
- Common GND
- 3.3V to 5V logic shifting through SN74AHCT125
- Series resistor on data line
- Capacitor (~1kF) across 5V and GND near strip input

## SN74AHCT125N
**Purpose:**
- Buffer & shift 3.3V signal to 5V logic signal for the WS2812B

**Electrical:**
- Supply: 4.5V to 5.5V
- Planned Operating Voltage: 5V
- Logic Input: Accepts 3.3V logic high

**Interface:**
- 4 independent non-inverting buffer channels
- A (logic input)
- Y (Buffered logic output)
- /OE (Active-low output enable)

**ESP32-S3 Requirements:**
- Uses same GPIO assigned to the WS2812B data signal
- Common GND
- /OE tied LOW for an always-enabled channel
