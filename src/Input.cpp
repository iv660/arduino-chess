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

// Movement timing in milliseconds
constexpr unsigned long MOVE_INITIAL_DELAY_MS = 800;   // Delay before first move
constexpr unsigned long MOVE_REPEAT_DELAY_MS = 150;    // Delay between repeated moves

// Track last movement time for repeat delay
static unsigned long lastMoveTime = 0;

// Track joystick deflection start time for initial delay
static unsigned long deflectionStartTime = 0;

// Track whether joystick is currently active (outside deadzone)
// This prevents false triggers until joystick returns to neutral
static bool joystickActive = false;

// Button debouncing state
static bool lastButtonState = HIGH;      // Last stable state (after debounce)
static bool lastButtonReading = HIGH;    // Last raw reading
static unsigned long lastDebounceTime = 0;
constexpr unsigned long DEBOUNCE_DELAY_MS = 50;

/**
 * Find the index of current cursor position in legal moves array
 * Returns the index if found, or 0 if not found (defaults to first legal move)
 */
static uint8_t findCurrentLegalMoveIndex(const GameState* state) {
  if (!state || state->legalMoveCount == 0) return 0;
  
  for (uint8_t i = 0; i < state->legalMoveCount; i++) {
    if (state->legalMoves[i][0] == state->cursorRow && 
        state->legalMoves[i][1] == state->cursorCol) {
      return i;
    }
  }
  
  return 0;  // Default to first legal move
}

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

bool isButtonNewlyPressed() {
  bool currentReading = digitalRead(JOYSTICK_BTN_PIN);
  unsigned long now = millis();
  
  // Detect state change in raw reading
  if (currentReading != lastButtonReading) {
    lastDebounceTime = now;
    lastButtonReading = currentReading;
  }
  
  // Check if debounce period has passed
  if ((now - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    // If stable reading differs from last stable state
    if (currentReading != lastButtonState) {
      lastButtonState = currentReading;
      // Return true only on falling edge (HIGH -> LOW, button pressed)
      if (currentReading == LOW) {
        return true;
      }
    }
  }
  
  return false;
}

void updateCursor(GameState* state) {
  if (!state) return;
  
  // Check if piece is selected - use different navigation logic
  if (isPieceSelected(state)) {
    // Cursor navigation restricted to legal moves
    if (state->legalMoveCount == 0) return;  // Safety check
    
    // Read joystick axes
    int16_t x = readJoystickX();
    int16_t y = readJoystickY();
    
    // Check if joystick is within deadzone
    bool inDeadzone = (abs(x) < JOYSTICK_DEADZONE && abs(y) < JOYSTICK_DEADZONE);
    
    if (inDeadzone) {
      // Joystick returned to neutral - reset state for next movement
      joystickActive = false;
      deflectionStartTime = 0;
      lastMoveTime = 0;
      return;
    }
    
    unsigned long now = millis();
    
    // If this is the first time joystick left deadzone, allow immediate first move
    if (!joystickActive) {
      joystickActive = true;
      deflectionStartTime = now;
      // Fall through to execute first move immediately
    } else {
      // Subsequent moves: check appropriate delay
      unsigned long timeSinceLastMove = now - lastMoveTime;
      
      if (lastMoveTime == deflectionStartTime) {
        // Second move: use initial delay
        if (timeSinceLastMove < MOVE_INITIAL_DELAY_MS) {
          return;  // Too soon for second move
        }
      } else {
        // Third and later moves: use repeat delay
        if (timeSinceLastMove < MOVE_REPEAT_DELAY_MS) {
          return;  // Too soon for repeat
        }
      }
    }
    
    // Get current index in legal moves
    uint8_t currentIndex = findCurrentLegalMoveIndex(state);
    
    // Determine direction (any joystick movement cycles through legal moves)
    int8_t direction = 0;
    if (x < -JOYSTICK_THRESHOLD || y < -JOYSTICK_THRESHOLD) {
      direction = -1;  // Previous move
    } else if (x > JOYSTICK_THRESHOLD || y > JOYSTICK_THRESHOLD) {
      direction = 1;   // Next move
    }
    
    if (direction != 0) {
      // Cycle through legal moves with wrapping
      int8_t newIndex = currentIndex + direction;
      if (newIndex < 0) {
        newIndex = state->legalMoveCount - 1;  // Wrap to end
      } else if (newIndex >= state->legalMoveCount) {
        newIndex = 0;  // Wrap to start
      }
      
      // Update cursor to new legal move position
      state->cursorRow = state->legalMoves[newIndex][0];
      state->cursorCol = state->legalMoves[newIndex][1];
      
      // Update movement time
      lastMoveTime = now;
    }
    
    return;
  }
  
  // Normal free-roaming cursor navigation when no piece selected
  // Read joystick axes
  int16_t x = readJoystickX();
  int16_t y = readJoystickY();
  
  // Check if joystick is within deadzone
  bool inDeadzone = (abs(x) < JOYSTICK_DEADZONE && abs(y) < JOYSTICK_DEADZONE);
  
  if (inDeadzone) {
    // Joystick returned to neutral - reset state for next movement
    joystickActive = false;
    deflectionStartTime = 0;
    lastMoveTime = 0;
    return;
  }
  
  unsigned long now = millis();
  
  // If this is the first time joystick left deadzone, allow immediate first move
  if (!joystickActive) {
    joystickActive = true;
    deflectionStartTime = now;
    // Fall through to execute first move immediately
  } else {
    // Subsequent moves: check appropriate delay
    unsigned long timeSinceLastMove = now - lastMoveTime;
    
    if (lastMoveTime == deflectionStartTime) {
      // Second move: use initial delay
      if (timeSinceLastMove < MOVE_INITIAL_DELAY_MS) {
        return;  // Too soon for second move
      }
    } else {
      // Third and later moves: use repeat delay
      if (timeSinceLastMove < MOVE_REPEAT_DELAY_MS) {
        return;  // Too soon for repeat
      }
    }
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
