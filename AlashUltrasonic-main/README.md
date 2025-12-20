# AlashUltrasonic Library

This library allows you to interface with the RCWL-9610 ultrasonic distance sensor using GPIO, I2C, UART, and 1-Wire interfaces. It provides a simple way to measure distances in centimeters.

## Features

- Support for multiple communication interfaces: GPIO, I2C, UART, and 1-Wire.
- Easy-to-use API for measuring distance.
- Compatible with Arduino.
- **NEW**: Resolved constructor ambiguity issues for better type safety.

## Installation

1. Download the latest release from the [GitHub repository](https://github.com/Alash-electronics/AlashUltrasonic).
2. Unzip the downloaded file.
3. Move the unzipped folder to your Arduino libraries directory (usually located in `Documents/Arduino/libraries`).
4. Restart the Arduino IDE.

## Configuration

### Configuring the Module

- **Default Configuration (GPIO)**:
  - The default configuration of the board is to use GPIO.
  - If you have previously used one of the other modes, ensure that the M1 and M2 jumpers are both open (no solder blob).

- **UART Configuration**:
  - The solder jumper "M1" (back of board top right) must be closed by putting a blob of solder on it (M2 must be open, no blob).

- **I2C Configuration**:
  - The solder jumper "M2" (back of board bottom right) must be closed by putting a blob of solder on it (M1 must be open, no blob).

## Usage

### Example for GPIO

```cpp
#include <AlashUltrasonic.h>

// GPIO pins
const uint8_t TRIGGER_PIN = 7;
const uint8_t ECHO_PIN = 8;

AlashUltrasonic sensorGPIO(TRIGGER_PIN, ECHO_PIN);

void setup() {
  Serial.begin(9600);
  sensorGPIO.begin();
}

void loop() {
  float distance = sensorGPIO.getDistance();
  Serial.print("Distance (GPIO): ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
}
```

### Example for I2C

```cpp
#include <AlashUltrasonic.h>

// I2C address
const uint8_t I2C_ADDRESS = 0x57;

AlashUltrasonic sensorI2C(I2C_ADDRESS);

void setup() {
  Serial.begin(9600);
  sensorI2C.begin();
}

void loop() {
  float distance = sensorI2C.getDistance();
  Serial.print("Distance (I2C): ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
}
```

### Example for UART

```cpp
#include <AlashUltrasonic.h>

// UART pins
const uint8_t RX_PIN = 3; // Подключите Echo_TX_SDA
const uint8_t TX_PIN = 2; // Подключите Trig_RX_SCL_I/O

AlashUltrasonic sensorUART(RX_PIN, TX_PIN, UART_MODE);

void setup() {
  Serial.begin(9600);
  sensorUART.begin();
}

void loop() {  
  float distance = sensorUART.getDistance();
  Serial.print("Distance (UART): ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
}
```

### Example for 1-Wire

```cpp
#include <AlashUltrasonic.h>

// 1-Wire pin
const uint8_t ONE_WIRE_PIN = A5;

AlashUltrasonic sensorOneWire(ONE_WIRE_PIN, ONEWIRE_MODE);

void setup() {
  Serial.begin(9600);
  sensorOneWire.begin();
}

void loop() {
  float distance = sensorOneWire.getDistance();
  Serial.print("Distance (1-Wire): ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
}
```

### Example with All Modes

```cpp
#include <AlashUltrasonic.h>

// Параметры для разных типов подключения
const uint8_t TRIGGER_PIN = 7;
const uint8_t ECHO_PIN = 8;
const uint8_t I2C_ADDRESS = 0x57;
const uint8_t RX_PIN = 3;
const uint8_t TX_PIN = 2;
const uint8_t ONE_WIRE_PIN = A5;

// Создание объектов для разных типов подключения
AlashUltrasonic sensorGPIO(TRIGGER_PIN, ECHO_PIN);           // GPIO режим
AlashUltrasonic sensorI2C(I2C_ADDRESS);                      // I2C режим
AlashUltrasonic sensorUART(RX_PIN, TX_PIN, UART_MODE);       // UART режим
AlashUltrasonic sensorOneWire(ONE_WIRE_PIN, ONEWIRE_MODE);   // 1-Wire режим

void setup() {
  Serial.begin(9600);
  
  // Инициализация всех датчиков
  sensorGPIO.begin();
  sensorI2C.begin();
  sensorUART.begin();
  sensorOneWire.begin();
}

void loop() {
  // Измерения с разных датчиков
  float distanceGPIO = sensorGPIO.getDistance();
  float distanceI2C = sensorI2C.getDistance();
  float distanceUART = sensorUART.getDistance();
  float distanceOneWire = sensorOneWire.getDistance();
  
  // Вывод результатов...
  delay(1000);
}
```

## API Reference

### `AlashUltrasonic`

#### Constructor

- `AlashUltrasonic(uint8_t triggerPin, uint8_t echoPin)`: Initialize the sensor with GPIO pins.
- `AlashUltrasonic(uint8_t i2cAddress)`: Initialize the sensor with an I2C address.
- `AlashUltrasonic(uint8_t rxPin, uint8_t txPin, ConnectionType type)`: Initialize the sensor with UART pins and connection type.
- `AlashUltrasonic(uint8_t oneWirePin, ConnectionType type)`: Initialize the sensor with a 1-Wire pin and connection type.

#### ConnectionType Enum

```cpp
enum ConnectionType {
  GPIO_MODE,
  I2C_MODE,
  UART_MODE,
  ONEWIRE_MODE
};
```

#### Methods

- `void begin()`: Initialize the sensor.
- `float getDistance()`: Get the distance in centimeters.

## Breaking Changes in v2.0

- **UART Constructor**: Changed from `AlashUltrasonic(rxPin, txPin, bool useUART)` to `AlashUltrasonic(rxPin, txPin, UART_MODE)`.
- **1-Wire Constructor**: Changed from `AlashUltrasonic(oneWirePin, true)` to `AlashUltrasonic(oneWirePin, ONEWIRE_MODE)`.
- **Better Type Safety**: All constructors now have unique signatures, preventing ambiguity when using similar pin numbers.

## License

This library is licensed under the MIT License. See the [LICENSE](LICENSE) file for more information.

## Acknowledgments

Special thanks to all contributors and the open-source community for their support and contributions.

## Contact

For any questions or suggestions, feel free to open an issue on the [GitHub repository](https://github.com/Alash-electronics/AlashUltrasonic) or contact me at alash.electronics@gmail.com
