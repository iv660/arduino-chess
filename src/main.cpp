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

/**
 * Execute computer move using mcu-max engine
 * Called when it's the computer's turn (assumes computer is black for Feature 7)
 */
void executeComputerMove(GameState* state) {
  if (!state) return;

  // Set phase to thinking
  state->phase = COMPUTER_THINKING;

  // Display thinking indicator
  drawThinkingIndicator();

  // Calculate best move with the minimum node budget that fits Nano stack limits.
  // mcu-max depth 1 still recurses to its internal minimum depth, so keep this
  // budget very small on the 2 KB ATmega328P.
  mcumax_move move = mcumax_search_best_move(1, 1);

  // Check for invalid move (no legal moves - checkmate or stalemate)
  if (move.from == MCUMAX_SQUARE_INVALID) {
    // No legal move available - game over
    state->phase = GAME_OVER;
    return;
  }

  // Execute the computer's move through the common state-sync path.
  executeMove(state, move.from >> 4, move.from & 0x07,
              move.to >> 4, move.to & 0x07);

  // Return to player move phase
  state->phase = PLAYER_MOVE;
}

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
  
  // Handle button press for piece selection and move execution
  if (isButtonNewlyPressed()) {
    if (isPieceSelected(&gameState)) {
      // Piece is already selected
      if (gameState.cursorRow == gameState.selectedRow && 
          gameState.cursorCol == gameState.selectedCol) {
        // Cancel selection - button pressed on same square
        clearSelection(&gameState);
        // Redraw board to clear selection highlights
        drawBoard(&gameState);
        drawTurnIndicator(&gameState);
      } else {
        // Execute move to cursor position
        uint8_t fromRow = gameState.selectedRow;
        uint8_t fromCol = gameState.selectedCol;
        uint8_t toRow = gameState.cursorRow;
        uint8_t toCol = gameState.cursorCol;
        
        if (executeMove(&gameState, fromRow, fromCol, toRow, toCol)) {
          // Move successful - redraw entire board and update turn indicator
          drawBoard(&gameState);
          drawTurnIndicator(&gameState);

          // Check if it's the computer's turn (assume player is white, computer is black)
          if (gameState.currentPlayer == 1) {
            executeComputerMove(&gameState);
            // Redraw board and turn indicator after computer move
            drawBoard(&gameState);
            drawTurnIndicator(&gameState);
          }
        }
      }
    } else {
      // No piece selected - try to select piece at cursor
      selectPieceAt(&gameState, gameState.cursorRow, gameState.cursorCol);
      if (isPieceSelected(&gameState)) {
        // Piece selected successfully - move cursor to first legal move
        if (gameState.legalMoveCount > 0) {
          gameState.cursorRow = gameState.legalMoves[0][0];
          gameState.cursorCol = gameState.legalMoves[0][1];
        }
        // Redraw board to show selection highlights
        drawBoard(&gameState);
      }
    }
  }
  
  // Optimized cursor redraw: only update old and new squares to avoid flicker
  if (gameState.cursorRow != oldRow || gameState.cursorCol != oldCol) {
    // Redraw old square without cursor
    drawSquare(&gameState, oldRow, oldCol, false);
    // Draw new square with cursor
    drawSquare(&gameState, gameState.cursorRow, gameState.cursorCol, true);
  }
}
