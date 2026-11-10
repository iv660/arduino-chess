/**
 * Input.h
 * Hardware input abstraction for joystick and buttons
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "GameState.h"

// Initialize input hardware (joystick and buttons)
void initInput();

// Read joystick X-axis as signed offset from center
int16_t readJoystickX();

// Read joystick Y-axis as signed offset from center
int16_t readJoystickY();

// Check if joystick button is currently pressed
bool isJoystickButtonPressed();

// Update cursor position based on joystick input
void updateCursor(GameState* state);

#endif // INPUT_H
