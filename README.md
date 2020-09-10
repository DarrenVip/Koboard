# GamePad [![Build Status](https://combatrobot.org/koboard/gamepad.svg?branch=master)](https://combatrobot.org/koboard/gamepad)

Generate HID Bluetooth devices with ESP32 and Arduino framework.

Base on [ESP32 library for stm32f4 (d2a4a47)](https://github.com/arduino-libraries/).

## Additional Boards Manager URLs:

https://dl.espressif.com/dl/package_esp32_index.json

## Interface

The interface is similar to Arduino/Bluetooth: https://www.arduino.cc/en/Reference/Bluetooth

But the function `attachOnConnect()` is different:

```c
void setup() {
  Serial.begin(115200);
  GP.begin();
  GP.attachOnConnect(onConnection);
}
```

#More information in [source code documentation](https://combatrobot.com/koboard/gamepad/blob/master/src/gamepad.h#L73).

#Example: [01-Simple](examples/01-GamePad/01-GamePad.ino)

## PlatformIO

This library is also available at the [PlatformIO](https://platformio.org) as [Koboard](https://platformio.org/lib/show/1739/Koboard).

## Arduino IDE

This library is available in Arduino IDE Library Manager as `Koboard`.

## Known issues

### Problem with build in Arduino IDE 1.8.10

There was an [issue](https://github.com/arduino/arduino-cli/pull/565) with building this library in Arduino IDE 1.8.10. But this issue should be fixed in Arduino IDE 1.8.11.
