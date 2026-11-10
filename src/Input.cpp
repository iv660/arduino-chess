/**
 * Input.cpp
 * Implementation of input hardware polling
 */

#include "Input.h"
#include <Arduino.h>

// Hardware pin definitions
constexpr uint8_t JOYSTICK_X_PIN = A0;
constexpr uint8_t JOYSTICK_Y_PIN = A1;
constexpr uint8_t JOYSTICK_BTN_PIN = 2;

// Joystick calibration and thresholds
constexpr int16_t JOYSTICK_CENTER = 512;
constexpr int16_t JOYSTICK_DEADZONE = 200;
constexpr int16_t JOYSTICK_THRESHOLD = 100;

// Movement repeat delay in milliseconds
constexpr unsigned long MOVE_REPEAT_DELAY_MS = 150;

// Track last movement time for repeat delay
static unsigned long lastMoveTime = 0;

void initInput() {
  // Set joystick button pin to INPUT_PULLUP (active LOW)
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP);
}

int16_t readJoystickX() {
  return analogRead(JOYSTICK_X_PIN) - JOYSTICK_CENTER;
}

int16_t readJoystickY() {
  return analogRead(JOYSTICK_Y_PIN) - JOYSTICK_CENTER;
}

bool isJoystickButtonPressed() {
  return digitalRead(JOYSTICK_BTN_PIN) == LOW;
}

void updateCursor(GameState* state) {
  if (!state) return;
  
  // Read joystick axes
  int16_t x = readJoystickX();
  int16_t y = readJoystickY();
  
  // Check if joystick is within deadzone
  if (abs(x) < JOYSTICK_DEADZONE && abs(y) < JOYSTICK_DEADZONE) {
    // Reset movement timer when joystick returns to center
    lastMoveTime = 0;
    return;
  }
  
  // Check movement repeat delay
  unsigned long now = millis();
  if (lastMoveTime != 0 && (now - lastMoveTime) < MOVE_REPEAT_DELAY_MS) {
    return;
  }
  
  // Map deflection to direction
  int8_t dx = 0;
  int8_t dy = 0;
  
  if (x < -JOYSTICK_THRESHOLD) {
    dx = -1;  // Left
  } else if (x > JOYSTICK_THRESHOLD) {
    dx = 1;   // Right
  }
  
  if (y < -JOYSTICK_THRESHOLD) {
    dy = -1;  // Down (may need inversion based on hardware)
  } else if (y > JOYSTICK_THRESHOLD) {
    dy = 1;   // Up
  }
  
  // No movement if both deltas are zero
  if (dx == 0 && dy == 0) {
    return;
  }
  
  // Update cursor with edge clamping
  int8_t newRow = state->cursorRow + dy;
  int8_t newCol = state->cursorCol + dx;
  
  // Clamp to board boundaries (0-7)
  if (newRow < 0) newRow = 0;
  if (newRow > 7) newRow = 7;
  if (newCol < 0) newCol = 0;
  if (newCol > 7) newCol = 7;
  
  state->cursorRow = newRow;
  state->cursorCol = newCol;
  
  // Update movement time
  lastMoveTime = now;
}
