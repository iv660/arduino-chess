/**
 * Arduino Chess - XC Slim Pad
 * Main program entry point
 */

#include <Arduino.h>
#include <mcu-max.h>
#include "GameState.h"

// Global game state
GameState gameState;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    ; // Wait for serial port
  }

  Serial.println(F("\n=== Arduino Chess - Starting ==="));
  
  // Initialize game state
  initGameState(&gameState);
  
  Serial.println(F("GameState initialized"));
  Serial.print(F("Memory: "));
  Serial.print(sizeof(GameState));
  Serial.println(F(" bytes"));
  
  Serial.println(F("\nReady for Feature 3: Display rendering"));
}

void loop() {
  // Game loop will be implemented in later features
}
