ESP32 Smart Safe System (Simulation Ready)

![Project Status](https://img.shields.io/badge/Status-Prototype-orange) ![Platform](https://img.shields.io/badge/Platform-ESP32-blue) ![License](https://img.shields.io/badge/License-MIT-green)

A secure, non-blocking embedded system design for a smart safe, developed using ESP32 architecture. This project demonstrates a robust **Finite State Machine (FSM)** implementation, replacing traditional blocking delays with `millis()` based timing for responsive multitasking.

Features

* **Non-Blocking Architecture:** Using `millis()` for time management allows the system to handle keypad inputs while managing LEDs and servo motors simultaneously.
* **State Machine Logic:** Clean and maintainable code structure (Locked, Open, Error states).
* **Secure Logic:** Hardware abstraction and clear separation of concerns.
* **Simulation Ready:** Designed to run seamlessly on Wokwi.

Tech Stack

* **Microcontroller:** ESP32 DevKit V1
* **Language:** C++ (Arduino Framework)
* **Modules:**
    * Keypad (4x4 Matrix)
    * Servo Motor (Lock Mechanism)
    * I2C LCD 16x2 Display
    * Status LEDs

Circuit Diagram
`[https://github.com/eemreinceer/ESP32-Smart-Safe-System/blob/main/circuit_diagram.png?raw=true]`

Finite State Machine (FSM)

The system operates in three main states:
1.  **STATE_LOCKED:** Waiting for user input.
2.  **STATE_OPEN:** Solenoid/Servo active for 3 seconds.
3.  **STATE_ERROR:** Visual feedback for incorrect password attempts.

How to Run (Simulation)

1.  Open the source code in [Wokwi](https://wokwi.com/).
2.  Copy the `main.cpp` content.
3.  Add the required libraries: `LiquidCrystal_I2C`, `Keypad`, `ESP32Servo`.
4.  Start the simulation.

---
*Developed by Emre İnceer*
