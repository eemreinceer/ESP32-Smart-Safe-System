Secure ESP32 Smart Safe System

![Project Status](https://img.shields.io/badge/Status-Prototype-orange) ![Platform](https://img.shields.io/badge/Platform-ESP32-blue) ![License](https://img.shields.io/badge/License-MIT-green)

A high-performance, non-blocking embedded security system engineered using the ESP32 architecture. This project features a robust **Finite State Machine (FSM)** implementation, utilizing asynchronous `millis()` timing to ensure responsive multitasking and real-time input processing.

Key Features

* **Asynchronous Architecture:** Eliminates blocking delays by leveraging `millis()` for time management, allowing the system to process keypad interrupts, manage LED indicators, and drive servo motors concurrently without latency.
* **Robust State Machine:** Implements a modular FSM architecture (Locked, Open, Error) ensuring predictable system behavior and ease of maintenance.
* **Hardware Abstraction:** Clean separation of concerns between hardware drivers and application logic for enhanced security and portability.
* **Simulation-Ready:** Fully optimized and pre-configured for seamless deployment on the Wokwi simulation platform.

Technical Specifications

* **Microcontroller:** ESP32 DevKit V1
* **Language:** C++ (Arduino Framework)
* **Hardware Components:**
    * 4x4 Matrix Keypad (Input Interface)
    * Micro Servo Motor (Locking Mechanism)
    * I2C LCD 16x2 (User Feedback Display)
    * Status Indicators (LEDs)

Circuit Topology

![Circuit Diagram](https://github.com/eemreinceer/ESP32-Smart-Safe-System/blob/main/circuit_diagram.png?raw=true)


System Architecture (FSM)

The system logic is governed by three primary states:

1.  **STATE_LOCKED:** The system remains in a low-power idle mode, polling for user authentication via the keypad.
2.  **STATE_OPEN:** Upon successful authentication, the actuator (Servo) disengages for a calibrated 3-second interval before automatically re-locking.
3.  **STATE_ERROR:** Triggers visual and logical feedback loops in response to unauthorized access attempts.

Simulation & Deployment Guide

1.  Navigate to the source code on [Wokwi](https://wokwi.com/).
2.  Replicate the `main.cpp` logic into the simulation environment.
3.  Install the required dependencies: `LiquidCrystal_I2C`, `Keypad`, and `ESP32Servo`.
4.  Initialize the simulation to observe the FSM in action.

---
*Engineered by Emre İnceer*
