/**
 * Arduino Chess - Basic LED Blink Test
 * 
 * Hardware: XC Slim Pad
 * Board: Arduino Nano (ATmega328P)
 * 
 * This is a basic test sketch to verify that the board can be programmed
 * successfully. The built-in LED (pin 13) will blink at 1 Hz.
 * 
 * Hardware Components:
 * - Arduino Nano
 * - 1.8" TFT Display with ST7735
 * - Analog Joystick with momentary switch
 * - Pause button (normally high)
 * - Start button (normally high)
 */

#include <Arduino.h>

// Built-in LED pin on Arduino Nano
#define LED_PIN 13

// Pin definitions for future use (based on XC Slim Pad schematic)
// TFT Display (ST7735)
#define TFT_CS    10
#define TFT_DC    8
// Note: TFT_RST is connected to Arduino RST pin (hardware reset, not controllable)
// SPI pins: MOSI=11, MISO=12, SCK=13 (hardware SPI)

// Joystick
#define JOY_X     A0
#define JOY_Y     A1
#define JOY_BTN   2

// Control buttons
#define BTN_PAUSE 3
#define BTN_START 4

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB)
  }
  
  Serial.println(F("Arduino Chess - XC Slim Pad"));
  Serial.println(F("LED Blink Test"));
  Serial.println(F("Board: Arduino Nano (ATmega328P)"));
  Serial.println();
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println(F("Setup complete. LED should be blinking..."));
}

void loop() {
  // Turn LED on
  digitalWrite(LED_PIN, HIGH);
  Serial.println(F("LED: ON"));
  delay(1000);  // Wait 1 second
  
  // Turn LED off
  digitalWrite(LED_PIN, LOW);
  Serial.println(F("LED: OFF"));
  delay(1000);  // Wait 1 second
}
