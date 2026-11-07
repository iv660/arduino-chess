/**
 * GameState.cpp
 * Implementation of core game state management
 */

#include "GameState.h"
#include <string.h>
#include <mcu-max.h>

/**
 * Map mcu-max piece constants to our board representation
 * mcu-max encoding:
 *   - MCUMAX_EMPTY = 0
 *   - MCUMAX_PAWN_UPSTREAM = 1, MCUMAX_PAWN_DOWNSTREAM = 2
 *   - MCUMAX_KNIGHT = 3, MCUMAX_KING = 4, MCUMAX_BISHOP = 5
 *   - MCUMAX_ROOK = 6, MCUMAX_QUEEN = 7
 *   - MCUMAX_BLACK = 0x8 (bit 3 set for black pieces)
 * 
 * Our encoding: We use mcu-max values directly for compatibility
 */
static uint8_t mapMcumaxPiece(mcumax_piece mcumaxPiece) {
  // Use mcu-max encoding directly
  return mcumaxPiece;
}

/**
 * Convert our [row][col] coordinates to mcu-max square encoding
 * Our board: row 0 = rank 8 (Black's back rank, top of board)
 *            row 7 = rank 1 (White's back rank, bottom of board)
 *            col 0 = a-file (left), col 7 = h-file (right)
 * mcu-max: square encoded as 0xRF where R=rank (0-7, 0=rank 1), F=file (0-7, 0=a-file)
 */
static uint8_t rowColToMcumaxSquare(uint8_t row, uint8_t col) {
  uint8_t rank = 7 - row;  // Convert row to rank (row 0 -> rank 7, row 7 -> rank 0)
  return (rank << 4) | col;
}

void initGameState(GameState* state) {
  if (!state) return;
  
  // Initialize mcu-max engine
  mcumax_init();
  
  // Sync board from engine
  syncBoardFromEngine(state);
  
  // Initialize game state
  state->currentPlayer = 0;  // White starts
  state->cursorRow = 0;
  state->cursorCol = 0;
  state->selectedRow = NO_SELECTION;
  state->selectedCol = NO_SELECTION;
  state->legalMoveCount = 0;
  state->phase = PLAYER_MOVE;
  state->result = IN_PROGRESS;
  state->castlingRights = 0x0F;  // All castling rights available
  state->enPassantTarget = NO_EN_PASSANT;
  
  // Clear legal moves buffer
  memset(state->legalMoves, 0, sizeof(state->legalMoves));
}

void resetGameState(GameState* state) {
  initGameState(state);
}

void syncBoardFromEngine(GameState* state) {
  if (!state) return;
  
  // Read all 64 squares from mcu-max engine
  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t mcumaxSquare = rowColToMcumaxSquare(row, col);
      mcumax_piece enginePiece = mcumax_get_piece(mcumaxSquare);
      state->board[row][col] = mapMcumaxPiece(enginePiece);
    }
  }
}

uint8_t getPiece(const GameState* state, uint8_t row, uint8_t col) {
  if (!state || row >= 8 || col >= 8) {
    return EMPTY_SQUARE;
  }
  return state->board[row][col];
}

bool isValidSquare(uint8_t row, uint8_t col) {
  return (row < 8) && (col < 8);
}

bool isPieceSelected(const GameState* state) {
  if (!state) return false;
  return state->selectedRow != NO_SELECTION;
}

const char* getPhaseString(GamePhase phase) {
  switch (phase) {
    case PLAYER_MOVE: return "Player";
    case COMPUTER_THINKING: return "Computer";
    case GAME_OVER: return "Game Over";
    default: return "Unknown";
  }
}

const char* getResultString(GameResult result) {
  switch (result) {
    case IN_PROGRESS: return "In Progress";
    case WHITE_WINS: return "White Wins";
    case BLACK_WINS: return "Black Wins";
    case DRAW: return "Draw";
    default: return "Unknown";
  }
}
