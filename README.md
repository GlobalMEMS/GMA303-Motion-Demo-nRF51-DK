nRF51-DK + GMA303 Motion Demo
================================
Demonstrate usage of motion algorithms with GMA303
* Pedometer
* Calories
* Activity
* Fall-down
* Shake
* Hand-raise
* Flip
* Sedentary
* Sleep cycle

Requirements
-----------
- nRF51-DK
- nRF51 SDK: tested with Version 10.0.0
- GCC ARM embedded compiler: tested with gcc-arm-none-eabi-4_9-2015q1-20150306
- GMA303

I2C Connections
---------------
- Use I2C0
  - SCL: P0.07
  - SDA: P0.30
- I2C 7-bit slave address: 0x18

Makefile
--------
- Modify the location of the SDK to your actual directory
```
#
# The base directory for nRF51 SDK
#
nRF51_SDK_ROOT := /C/Data/nRF51_SDK/V10_0_0
```

- Modify the location of the GNU tools to your actual directory
```
GNU_INSTALL_ROOT := C:/Program Files (x86)/GNU Tools ARM Embedded/4.9 2015q1
```

- Execute `make all` to build and flash the program

- Exectute `make help` for available targets

Sensor Layout Pattern
---------------------
Sensor layout pattern is defined by the following macro in the "main.c"
```
#define ACC_LAYOUT_PATTERN          PAT6                 //accelerometer layout pattern
```

Please refer to the "Sensor_Layout_Pattern_Definition.pdf" document for the definition and modify accordingly to fit your actual layout.

Default gma303_initialization function in gma303.c
--------------------------------------------------
Default initialization steps:
 * Turn on offset temperature compensation
 * Set to continuous mode
 * Turn on low pass filter
 * Set data ready INT, ative high, push-pull

You may change the behavior of this initialization function to suit your purpose. Please refer to datasheet for more details on the register settings.

Usage of AutoNil
----------------
 * The program will do an offset AutoNil when executed. Hold the g-sensor steady and maintain in level, then press 'y' after the program prompt for input.
 * You may change the `DATA_AVE_NUM` macro in the gSensor_autoNil.h for the moving averae order for the offset estimation. Defautl is 32.
