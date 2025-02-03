#include <ArduinoJson.h>
#include <ESP32Encoder.h>

// ------------------------------------------------
// Pin definitions
// ------------------------------------------------
#define PWMA        48    // PWM output for Motor A
#define AIN1        21    // Motor A direction control
#define AIN2        47    // Motor A direction control

// Encoder (GA12-N20) connections:
#define ENCODER_C1  35    // Encoder channel A
#define ENCODER_C2  36    // Encoder channel B

// ------------------------------------------------
// Global variables
// ------------------------------------------------
volatile int appliedTorque = 0;   // Last commanded torque (positive = forward, negative = reverse)
volatile long energyBudget = 0;  // Global energy budget

// Create an ESP32Encoder instance (using the PCNT peripheral behind the scenes)
ESP32Encoder encoder;

// To detect tick differences, store the previous encoder count
long previousCount = 0;

// Reverse the encoder count direction
bool reverseEncoder = true;

// ------------------------------------------------
// Motor control function
// ------------------------------------------------
void moveMotor(int torque, bool forward) {
  if (forward) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  }
  ledcWrite(0, torque);  // Set PWM duty cycle (0–255)
}

// ------------------------------------------------
// Setup
// ------------------------------------------------
void setup() {
  // Initialize the built‑in USB serial port using USBSerial.
  USBSerial.begin(115200);
  while (!USBSerial) {  // Wait until the USBSerial port is ready.
    delay(10);
  }
  USBSerial.println("System Initialized. Waiting for JSON commands...");

  // Set up motor driver pins.
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  // Configure PWM on channel 0 at 5 kHz with 8-bit resolution.
  ledcAttachPin(PWMA, 0);
  ledcSetup(0, 5000, 8);

  // ----- Initialize the hardware encoder -----
  // Enable internal weak pull-ups.
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  // Attach the encoder pins in half-quadrature mode.
  // (If you prefer, you could swap the pin order here.
  //  In this example we reverse the encoder by inverting its count later.)
  encoder.attachHalfQuad(ENCODER_C1, ENCODER_C2);
  encoder.clearCount();

  // Initialize previousCount using the reversed encoder reading.
  previousCount = -encoder.getCount();
}

// ------------------------------------------------
// Main loop
// ------------------------------------------------
void loop() {
  // --- Process any incoming JSON command over USB ---
  if (USBSerial.available() > 0) {
    // Read incoming data until a newline character.
    String input = USBSerial.readStringUntil('\n');
    if (input.length() > 0) {
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, input);
      if (!error) {
        // Extract the "torque" and "budget" values from the JSON command.
        int cmdTorque = doc["torque"] | 0;
        int cmdBudget = doc["budget"] | 0;

        // Update the global commanded torque and add the incoming budget.
        appliedTorque = cmdTorque;
        energyBudget += cmdBudget;

        USBSerial.print("Received command: torque = ");
        USBSerial.print(cmdTorque);
        USBSerial.print(", budget += ");
        USBSerial.println(cmdBudget);
      } else {
        USBSerial.print("JSON parse error: ");
        USBSerial.println(error.c_str());
      }
    }
  }

  // --- Update energy budget based on encoder ticks ---
  //
  long currentCount = reverseEncoder ? -encoder.getCount() : encoder.getCount();  
  long diff = currentCount - previousCount;

  // Update energy budget according to the tick difference and commanded motor torque.
  energyBudget -= diff * appliedTorque;
  previousCount = currentCount;  // Save current count for the next loop

  // --- Update motor drive ---
  //
  // The motor is driven using the absolute value of the commanded torque,
  // but only if the energy budget is positive. Otherwise, the motor is stopped.
  int currentCmd = appliedTorque;
  long currentBudget = energyBudget;
  int motorTorque = (currentBudget > 0) ? abs(currentCmd) : 0;

  if (currentCmd > 0) {
    moveMotor(motorTorque, true);
  } else if (currentCmd < 0) {
    moveMotor(motorTorque, false);
  } else {
    moveMotor(0, true);  // Stop the motor if no torque is commanded.
  }

  // Print encoder count and energy budget every second ---
  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 1000) {
    USBSerial.print("Encoder Count: ");
    USBSerial.print(currentCount);
    USBSerial.print(" | Energy Budget: ");
    USBSerial.println(currentBudget);
    lastPrintTime = millis();
  }

  delay(10);  
}
