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

// Piece bitmap constants
#define PIECE_BITMAP_SIZE  16       // 16×16 pixel bitmaps
#define PIECE_OFFSET       0        // 16×16 fills the 16×16 square exactly

// King bitmap and mask (16×16, extracted from pieces.png via image2cpp)
// Bitmap: black pixels of the piece (0=piece pixel, 1=background)
// Mask: transparency mask (1=opaque/draw, 0=transparent/skip)
const uint8_t PROGMEM bitmap_king[] = {
  0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x02, 0x40, 0x1d, 0xb8, 0x22, 0x44, 0x41, 0x82, 0x41, 0x82, 
  0x41, 0x82, 0x40, 0x02, 0x2f, 0xf4, 0x10, 0x08, 0x1f, 0xf8, 0x10, 0x08, 0x0f, 0xf0, 0x00, 0x00
};

const uint8_t PROGMEM mask_king[] = {
  0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x1f, 0xf8
};

// Queen bitmap and mask (frame-02)
const uint8_t PROGMEM bitmap_queen[] = {
  0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x42, 0x42, 0x74, 0x2e, 0x48, 0x12, 0x20, 0x04, 
  0x20, 0x04, 0x2f, 0xf4, 0x10, 0x08, 0x1f, 0xf8, 0x30, 0x0c, 0x20, 0x04, 0x1f, 0xf8, 0x00, 0x00
};

const uint8_t PROGMEM mask_queen[] = {
  0x01, 0x80, 0x03, 0xc0, 0x83, 0xc1, 0xc7, 0xe3, 0xf7, 0xef, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 
  0x7f, 0xfe, 0x7f, 0xfe, 0x3f, 0xfc, 0x3f, 0xfc, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x1f, 0xf8
};

// Bishop bitmap and mask (frame-03)
const uint8_t PROGMEM bitmap_bishop[] = {
  0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x50, 0x08, 0x90, 
  0x08, 0x10, 0x04, 0x30, 0x07, 0xe0, 0x04, 0x20, 0x02, 0x40, 0x04, 0x20, 0x0f, 0xf0, 0x00, 0x00
};

const uint8_t PROGMEM mask_bishop[] = {
  0x01, 0x80, 0x03, 0xc0, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 
  0x1f, 0xf8, 0x0f, 0xf8, 0x0f, 0xf0, 0x0f, 0xf0, 0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8
};

// Knight bitmap and mask (frame-04)
const uint8_t PROGMEM bitmap_knight[] = {
  0x04, 0x80, 0x04, 0x80, 0x08, 0x20, 0x14, 0x10, 0x28, 0x90, 0x43, 0x10, 0x4d, 0x08, 0x31, 0x08, 
  0x01, 0x08, 0x02, 0x10, 0x04, 0x10, 0x08, 0x10, 0x08, 0x10, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00
};

const uint8_t PROGMEM mask_knight[] = {
  0x06, 0xc0, 0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8, 0x7f, 0xf8, 0xff, 0xfc, 0xff, 0xfc, 0x7f, 0xfc, 
  0x33, 0xfc, 0x07, 0xf8, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc
};

// Rook bitmap and mask (frame-05)
const uint8_t PROGMEM bitmap_rook[] = {
  0x00, 0x00, 0x1d, 0xb8, 0x16, 0x68, 0x10, 0x08, 0x1f, 0xf8, 0x08, 0x10, 0x04, 0x20, 0x04, 0x20, 
  0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x08, 0x10, 0x0f, 0xf0, 0x10, 0x08, 0x1f, 0xf8, 0x00, 0x00
};

const uint8_t PROGMEM mask_rook[] = {
  0x3d, 0xbc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x0f, 0xf0, 
  0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc
};

// Pawn bitmap and mask (frame-06)
const uint8_t PROGMEM bitmap_pawn[] = {
  0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x02, 0x40, 0x01, 0x80, 0x02, 0x40, 0x02, 0x40, 
  0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x0f, 0xf0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00
};

const uint8_t PROGMEM mask_pawn[] = {
  0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0, 
  0x07, 0xe0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x00, 0x00
};

void drawPiece(uint8_t piece, uint8_t row, uint8_t col) {
  // Extract piece type (bits 0-2) and color (bit 3)
  uint8_t pieceType = piece & 0x07;
  bool isBlack = (piece & 0x08) != 0;  // 0x08 bit set = black piece
  
  // Skip empty squares
  if (pieceType == 0) {  // MCUMAX_EMPTY
    return;
  }
  
  // Select bitmap and mask based on piece type
  // Map mcu-max constants: 1=PAWN_UP, 2=PAWN_DOWN, 3=KNIGHT, 4=KING, 5=BISHOP, 6=ROOK, 7=QUEEN
  const uint8_t* bitmap = nullptr;
  const uint8_t* mask = nullptr;
  
  switch (pieceType) {
    case 1:  // MCUMAX_PAWN_UPSTREAM
    case 2:  // MCUMAX_PAWN_DOWNSTREAM
      bitmap = bitmap_pawn;
      mask = mask_pawn;
      break;
    case 3:  // MCUMAX_KNIGHT
      bitmap = bitmap_knight;
      mask = mask_knight;
      break;
    case 4:  // MCUMAX_KING
      bitmap = bitmap_king;
      mask = mask_king;
      break;
    case 5:  // MCUMAX_BISHOP
      bitmap = bitmap_bishop;
      mask = mask_bishop;
      break;
    case 6:  // MCUMAX_ROOK
      bitmap = bitmap_rook;
      mask = mask_rook;
      break;
    case 7:  // MCUMAX_QUEEN
      bitmap = bitmap_queen;
      mask = mask_queen;
      break;
    default:
      return;  // Unknown piece type
  }
  
  // Calculate screen position
  int16_t screenX = BOARD_OFFSET_X + col * SQUARE_SIZE + PIECE_OFFSET;
  int16_t screenY = BOARD_OFFSET_Y + row * SQUARE_SIZE + PIECE_OFFSET;
  
  // Determine piece colors
  // Bitmaps are designed for black pieces (bitmap bit 0 = black pixel)
  // For black pieces: bitmap=0 -> black, bitmap=1 -> white
  // For white pieces: invert colors: bitmap=0 -> white, bitmap=1 -> black
  uint16_t primaryColor = isBlack ? ST77XX_WHITE : ST77XX_BLACK;
  uint16_t secondaryColor = isBlack ? ST77XX_BLACK : ST77XX_WHITE;
  
  // Render sprite pixel-by-pixel with transparency
  // Bitmap format: 16×16 pixels, 2 bytes per row (16 bits), MSB first
  // Bitmap: 0=primary color, 1=secondary color
  // Mask: 1=opaque (draw), 0=transparent (skip)
  for (uint8_t py = 0; py < PIECE_BITMAP_SIZE; py++) {
    for (uint8_t px = 0; px < PIECE_BITMAP_SIZE; px++) {
      // Calculate byte index and bit position (MSB first)
      uint16_t bitIndex = py * PIECE_BITMAP_SIZE + px;
      uint8_t byteIndex = bitIndex / 8;
      uint8_t bitPosition = 7 - (bitIndex % 8);  // MSB = bit 7
      
      // Read mask bit from PROGMEM
      uint8_t maskByte = pgm_read_byte(&mask[byteIndex]);
      bool isMaskSet = (maskByte & (1 << bitPosition)) != 0;
      
      // Skip transparent pixels (where mask=0)
      if (!isMaskSet) {
        continue;
      }
      
      // Read bitmap bit from PROGMEM
      uint8_t bitmapByte = pgm_read_byte(&bitmap[byteIndex]);
      bool isBitmapSet = (bitmapByte & (1 << bitPosition)) != 0;
      
      // Draw all pixels inside mask with two colors
      // bitmap=0 -> primary color, bitmap=1 -> secondary color
      uint16_t pixelColor = isBitmapSet ? secondaryColor : primaryColor;
      tft.drawPixel(screenX + px, screenY + py, pixelColor);
    }
  }
}

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
  
  // Draw pieces on the board
  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t piece = state->board[row][col];
      if (piece != EMPTY_SQUARE) {
        drawPiece(piece, row, col);
      }
    }
  }
  
  // Draw cursor highlight
  int16_t cursorX = BOARD_OFFSET_X + state->cursorCol * SQUARE_SIZE;
  int16_t cursorY = BOARD_OFFSET_Y + state->cursorRow * SQUARE_SIZE;
  
  // Draw double-line yellow border for visibility
  tft.drawRect(cursorX, cursorY, SQUARE_SIZE, SQUARE_SIZE, ST77XX_YELLOW);
  tft.drawRect(cursorX + 1, cursorY + 1, SQUARE_SIZE - 2, SQUARE_SIZE - 2, ST77XX_YELLOW);
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
