# Smart Energy-Aware Actuators

This project demonstrates a smart energy-aware actuator implementation that guarantees safe, stable, and passive operation by actively monitoring and controlling the energy injected into a mechanical system. The actuator receives control commands—including both a torque command and an energy budget allocation—from a high‑level controller over USB serial (formatted as JSON). By tracking its energy usage via encoder feedback, the actuator ensures that it never injects more energy than allowed, thereby enforcing passivity and improving overall system safety—even in the face of communication delays or loss.

---

## Overview

Robotic systems operating in dynamic and uncertain environments require robust safety measures. One effective approach is to control the energy injected into the system to ensure passivity, a property that guarantees stability. This project builds on recent research in energy-aware control by implementing an **Embedded Energy-Aware Actuator (E²A²)** that:

- Monitors the energy output at the actuator level.
- Enforces an energy budget so that no more energy is injected than what is allocated.
- Remains safe and stable even under adverse conditions such as communication loss.

The actuator continuously updates its internal energy budget based on encoder feedback and the applied torque. If the energy budget is depleted, the actuator stops supplying power, thereby preventing unsafe energy injection into the system.

---

## Features

- **Embedded Energy Monitoring:**  
  Uses encoder feedback to calculate the energy injected into the system in real time.

- **Energy Budgeting:**  
  Each actuator has an energy budget that limits the amount of energy that can be injected. The energy budget is updated based on motor movement and is replenished when energy is extracted (e.g., during braking).

- **Safety & Passivity Guarantee:**  
  When the energy budget is exhausted, the actuator stops outputting torque to ensure that no additional energy is injected, thereby preserving system passivity and stability.

- **Robust Communication:**  
  The design is inherently robust to communication delays or loss since the actuator enforces energy constraints locally.

- **JSON-Based Control Protocol:**  
  Receives commands in JSON format over USB serial, simplifying integration with high-level controllers.

---

## Hardware Requirements

- **Microcontroller:** ESP32 S3
- **Motor Driver:** Capable of PWM control with two direction pins (configured here as `AIN1` and `AIN2`)
- **Encoder:** A quadrature encoder (e.g., GA12-N20) connected to designated channels

---

## Software Dependencies

- **[ArduinoJson](https://arduinojson.org/):**  
  For parsing incoming JSON commands.

- **[ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder):**  
  For handling quadrature encoder inputs using the ESP32’s PCNT peripheral.

---

## Code Overview

The code implements the core functionality of a smart energy-aware actuator. Below is an outline of its key components.

### Pin Definitions & Global Variables

- **Motor Control Pins:**
  - `PWMA`: PWM output for motor speed control.
  - `AIN1` & `AIN2`: Digital outputs to control motor direction.

- **Encoder Pins:**
  - `ENCODER_C1` and `ENCODER_C2`: Encoder channels used for measuring motor rotation.

- **Global Variables:**
  - `volatile int appliedTorque`: Stores the latest commanded torque.
  - `volatile long energyBudget`: The current energy budget available for injection.
  - `long previousCount`: The last recorded encoder count to compute the difference.
  - `bool reverseEncoder`: Flag to reverse the encoder count if needed.

--- 
## Energy Budgeting Concept

The energy injected by the actuator is estimated by the formula:

```cpp
energyBudget -= diff * appliedTorque;
```

This estimation is based on:
- The assumption that the actuator’s force (or torque) remains constant over a short sampling period.
- The measured position change (via encoder ticks) gives an approximation of the work done.

When the energy budget is depleted (i.e., when `energyBudget` is zero or negative), the actuator stops applying any torque, ensuring that no additional energy is injected and the system remains passive.

---

## JSON Command Protocol

Commands are sent to the actuator over USB serial using a JSON format. The expected structure is as follows:

```json
{
  "torque": <integer>,
  "budget": <integer>
}
```

- **torque:**  
  The desired torque command. A positive value indicates forward motion, while a negative value indicates reverse motion.

- **budget:**  
  The amount of energy (in arbitrary units) to be added to the actuator’s energy budget. This ensures the actuator has sufficient energy to perform the desired action.

**Example Command:**

```json
{"torque": 150, "budget": 1000}
```

This command sets the desired torque to 150 (PWM units) and adds 1000 units to the energy budget.

---

## Usage Instructions

1. **Hardware Setup:**
   - Connect your motor driver to the ESP32 using the designated PWM and direction pins.
   - Connect the encoder channels to the ESP32 pins defined in the code.
   - Ensure proper power supply and wiring as per your hardware specifications.

2. **Running the System:**
   - Open the Serial Monitor (set the baud rate to 115200).
   - Send JSON commands in the specified format to control the motor and manage the energy budget.
   - Observe the actuator behavior and debug outputs (encoder count and energy budget) on the serial monitor.

---

## Project Roadmap

This project is part of a larger initiative to develop smart energy-aware actuators based on energy-based control principles. Future work will include:

- **Open-Source Library:**  
  Developing an extensive library for modeling and controlling energy-aware actuators.

- **Enhanced Communication Protocol:**  
  Improving the protocol for more complex control scenarios.

- **Prototype Development:**  
  Building and validating a reproducible hardware prototype.

- **Tutorials & Documentation:**  
  Providing comprehensive tutorials and theoretical background materials on [energyawarerobotics.eu](http://energyawarerobotics.eu).

---

## Contact

For further questions, suggestions, or contributions, please contact the project maintainers or visit our website at [energyawarerobotics.eu](http://energyawarerobotics.eu).

