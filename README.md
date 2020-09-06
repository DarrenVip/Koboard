
Generate HID Bluetooth devices with ESP32 and Arduino framework.

Base on [ESP32 library for stm32f4 (d2a4a47)](https://github.com/arduino-libraries/).

## Interface

function `attachOnConnect()` :

```c
void setup() {
  Serial.begin(115200);
  GP.begin();
  GP.attachOnConnect(onConnection);
}
```


#Example: [01-Simple](examples/01-GamePad/01-GamePad.ino)


## Arduino IDE

This library is available in Arduino IDE Library Manager as `Koboard`.

## Known issues

### Problem with build in Arduino IDE 1.8.10

There was an [issue](https://github.com/arduino/arduino-cli/pull/565) with building this library in Arduino IDE 1.8.10. But this issue should be fixed in Arduino IDE 1.8.11.
#koboard

