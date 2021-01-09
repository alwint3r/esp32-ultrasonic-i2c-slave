ESP32 + Ultrasonic Sensor (HY-SRF05) as I2C Slave
=================================================

This project demonstrates how one can use ESP32 as an I2C slave that reads and send distance measurement (in cm) upon request by the master device.
This project is based on the official development platform ESP-IDF and can be compiled, debugged, and uploaded by PlatformIO.

## Wiring Information

Pin | Function
----|-----------------
IO4 | Ultrasonic Echo
IO5 | Ultrasonic Trigger
IO21| I2C SDA
IO22| I2C SCL

## I2C Details

#### Device Address

The default device address is `0x70`. The master device can use either `0x70` or `(0x70 << 1)` as the device address when communicating with this device, depending on the platform of the master device.

The 10 bit addressing is not supported.

#### Registers

#### `DISTANCE (0x01) - readonly`

This register is used to read the distance measurement (in cm) which is splitted into 2 bytes of unsigned integer.

## Building the Project

You can use the PlatformIO IDE (VS Code extensions) or if you have PlatformIO CLI installed, you can run the following command to build the project:

```sh
pio run
```

Then you can use the following command to flash the firmware

```sh
pio run -t upload
```

By default, the upload port is `/dev/ttyUSB0` and the upload speed is `921600` you can change both of these parameters by editing the `platformio.ini` file.


## Known Issues

This project is tested by connecting a NUCLEO F446RE board as the master device. You can find the firmware that is used to test this firmware on [alwint3r/stm32f446re-i2c-master-test](https://github.com/alwint3r/stm32f446re-i2c-master-test). Below are the list of issues found during testing:

* The slave device always sends the previous measurement or (n-1)th measurement and not the latest measurement as the response to reading `DISTANCE` register.


## Acknowledgement

The code that reads the distance measurement from the HY-SRF05 ultrasonic sensor is basically a modified version of the `ultrasonic` component from [esp-idf-lib](https://github.com/UncleRus/esp-idf-lib) written by [Ruslan V. Uss](github.com/UncleRus).
