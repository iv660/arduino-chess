/**
 * Display.h
 * Display initialization and rendering for ST7735 TFT
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "GameState.h"

// Initialize ST7735 display hardware
void initDisplay();

// Draw the 8×8 chess board grid with alternating colors
void drawBoard(const GameState* state);

// Draw the current turn indicator text at screen top
void drawTurnIndicator(const GameState* state);

#endif // DISPLAY_H
