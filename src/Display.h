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

// Draw a single board square with its piece and optional cursor highlight
void drawSquare(const GameState* state, uint8_t row, uint8_t col, bool drawCursor);

// Draw a single piece at the specified board position
void drawPiece(uint8_t piece, uint8_t row, uint8_t col);

// Draw the current turn indicator text at screen top
void drawTurnIndicator(const GameState* state);

#endif // DISPLAY_H
