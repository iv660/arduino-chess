/**
 * Display.cpp
 * Display initialization and rendering for ST7735 TFT
 */

#include "Display.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ST7735 TFT display object
// Pins: CS=10, DC=8, RST=-1 (shared with Arduino reset)
#define TFT_CS   10
#define TFT_DC   8
#define TFT_RST  -1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Board layout constants
#define SQUARE_SIZE    16    // 128 / 8 = 16 pixels per square
#define BOARD_OFFSET_X 0     // Start at left edge
#define BOARD_OFFSET_Y 0     // Start at top edge

// Colors (RGB565)
#define COLOR_LIGHT_SQUARE  0xCE79  // Light gray
#define COLOR_DARK_SQUARE   0x4208  // Dark gray

void initDisplay() {
  Serial.print(F("Initializing ST7735 display... "));
  
  // Initialize TFT with INITR_BLACKTAB (common for 1.8" 128x160 displays)
  tft.initR(INITR_BLACKTAB);
  
  // Set rotation: 0=portrait, 1=landscape, 2=portrait180, 3=landscape180
  // Using landscape (1) for 160x128 display
  tft.setRotation(1);
  
  // Clear screen to black
  tft.fillScreen(ST77XX_BLACK);
  
  Serial.println(F("Display OK"));
}

void drawBoard(const GameState* state) {
  // Draw 8×8 chess board with alternating colors
  // Standard chess: a1 (bottom-left) is dark square
  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      // Calculate screen position
      int16_t x = BOARD_OFFSET_X + col * SQUARE_SIZE;
      int16_t y = BOARD_OFFSET_Y + row * SQUARE_SIZE;
      
      // Determine square color
      // Row 0 is rank 8 (top of screen), row 7 is rank 1 (bottom)
      // Col 0 is file a, col 7 is file h
      // a1 (col=0, row=7) should be dark: (0+7) % 2 == 1 -> dark
      bool isDark = ((col + row) % 2) == 1;
      uint16_t color = isDark ? COLOR_DARK_SQUARE : COLOR_LIGHT_SQUARE;
      
      // Draw filled rectangle for this square
      tft.fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);
    }
  }
  
  // Draw file labels (a-h) inside bottom squares, lower-left corner
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  for (uint8_t col = 0; col < 8; col++) {
    char file = 'a' + col;
    int16_t x = BOARD_OFFSET_X + col * SQUARE_SIZE + 1; // Left edge + 1px
    int16_t y = BOARD_OFFSET_Y + 7 * SQUARE_SIZE + 8;   // Bottom row, lower part
    tft.setCursor(x, y);
    tft.print(file);
  }
  
  // Draw rank labels (1-8) on left side
  for (uint8_t row = 0; row < 8; row++) {
    char rank = '8' - row; // Row 0 = rank 8, row 7 = rank 1
    int16_t x = BOARD_OFFSET_X + 8 * SQUARE_SIZE + 2;   // Right of board
    int16_t y = BOARD_OFFSET_Y + row * SQUARE_SIZE + 4; // Vertically centered
    tft.setCursor(x, y);
    tft.print(rank);
  }
}

void drawTurnIndicator(const GameState* state) {
  // Position in top-right corner
  int16_t x = 140;  // Right edge - margin
  int16_t y = 2;    // Top edge + small margin
  int16_t boxSize = 18;
  
  if (state->currentPlayer == 0) {
    // White turn: white background, black text
    tft.fillRect(x, y, boxSize, boxSize, ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(x + 4, y + 2);
    tft.print(F("W"));
  } else {
    // Black turn: black background with white border, white text
    tft.fillRect(x, y, boxSize, boxSize, ST77XX_WHITE);  // White border
    tft.fillRect(x + 1, y + 1, boxSize - 2, boxSize - 2, ST77XX_BLACK);  // Black inside
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(x + 4, y + 2);
    tft.print(F("B"));
  }
}
