# SmartCup Arduino Firmware 

This repository houses the firmware code for the SmartCup project, a unique cup that informs users about their beverage's temperature and ensures it's maintained at a preferred heat level.

## Overview

The `ARDUINO_CODE.ino` file contains the Arduino firmware developed for the SmartCup. This code is responsible for reading temperature data from the sensors, processing it, and interfacing with other components to relay the information to the user.

## Prerequisites

- Arduino IDE or PlatformIO
- Compatible Arduino board (e.g., Arduino Uno, Nano, Mega)
- Necessary hardware components like temperature sensors, LEDs, etc.

## Installation and Setup

1. Clone this repository:

```bash
git clone https://github.com/jameschen256/SmartCup.git
```

2. Navigate to the project directory:

```bash
cd SmartCup
```

3. Open `ARDUINO_CODE_V4.ino` in Arduino IDE.

4. Connect your Arduino board to your computer.

5. From the Arduino IDE, select the correct board and port.

6. Upload the code to your board.

## Features

- **Temperature Sensing**: The code interfaces with temperature sensors to acquire real-time temperature readings.
  
- **LED Indication**: Depending on the beverage's temperature, the LED color changes to provide a visual indication to the user.
  
- **Safety Features**: Alerts the user if the beverage is too hot, ensuring user safety.

## Hardware Components

- **Arduino Board**: Serves as the microcontroller.
  
- **Temperature Sensors**: Detects the current temperature of the beverage.

- **LEDs**: Provides visual feedback to the user based on temperature readings.

- **Heating Element**: Placed betwen shell of cup and inner container to adjust liquid temperature.

- **Cooling Element**: Placed at base of cup to adjust liquid temperature.
