/**
 * Arduino Chess - XC Slim Pad
 * Main program entry point
 */

#include <Arduino.h>
#include <mcu-max.h>
#include "GameState.h"
#include "Display.h"
#include "Input.h"

// Global game state
GameState gameState;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {
    ; // Wait for serial port
  }

  Serial.println(F("\n=== Arduino Chess - Starting ==="));
  
  // Initialize display
  initDisplay();
  
  // Initialize input hardware
  initInput();
  
  // Initialize game state
  initGameState(&gameState);
  
  Serial.println(F("GameState initialized"));
  Serial.print(F("Memory: "));
  Serial.print(sizeof(GameState));
  Serial.println(F(" bytes"));
  
  // Draw board and turn indicator
  drawBoard(&gameState);
  Serial.println(F("Board drawn"));
  
  drawTurnIndicator(&gameState);
  Serial.println(F("Turn indicator shown"));
  
  Serial.println(F("\nDisplay ready"));
}

void loop() {
  // Store previous cursor position
  uint8_t oldRow = gameState.cursorRow;
  uint8_t oldCol = gameState.cursorCol;
  
  // Update cursor based on joystick input
  updateCursor(&gameState);
  
  // Redraw board if cursor moved
  if (gameState.cursorRow != oldRow || gameState.cursorCol != oldCol) {
    drawBoard(&gameState);
  }
}
