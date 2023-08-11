# ESP32 Applications

This repository contains various Applications for the ESP32-WROOM32 that can be compiled and used with the Arduino IDE.
It is part of a master-thesis with the goal to develop an Orchestration and Deployment Manager.

For a complete understanding of the project, please refer to the following repositories:

- [Orchestration and Deployment Manager](https://github.com/pixelboehm/longevity): Main application that handles the orchestration of LDTs.
- [Longevity Digital Twins](https://github.com/pixelboehm/ldt): Stores various LDTs.
- [LDT Meta Repository](https://github.com/pixelboehm/meta-ldt): Stores a file with links to repositories containing LDTs.
- (Optional) [Homebrew-LDT](https://github.com/pixelboehm/homebrew-ldt): Contains Homebrew (outdated) formulas for the ODM and LDTs. The formulas are not up-to-date anymore, but can be enabled through the `.goreleaser.yml` again.

## Applications

- Lightbulb
- Switch

## Setup Variables

The following variables need to be set at the beginning of each file, in order to function properly:

```c
const char* ssid = "";
const char* password = "";
const char* bootstrapper_address = ""; // IP:PORT
```

## Building

I build each target with via the Arduino IDE, as it makes the usage of libraries easier. The device is called "DOIT ESP32 DEVKIT V1". For help regarding the general setup, please look [here](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)