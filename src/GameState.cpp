/**
 * GameState.cpp
 * Implementation of core game state management
 */

#include "GameState.h"
#include <Arduino.h>
#include <string.h>
#include <mcu-max.h>

/**
 * Map mcu-max piece constants to our board representation
 *
 * IMPORTANT: mcu-max coordinate system and encoding
 * ==================================================
 * The mcu-max engine's ranks are arranged for a top-down display:
 * - Rank 0 = black's back rank (top of display, row 0)
 * - Rank 7 = white's back rank (bottom of display, row 7)
 *
 * mcumax_get_piece() color encoding:
 * - Bit 0x8 clear (0x1-0x7) = white pieces
 * - Bit 0x8 set (0x9-0xF) = black pieces
 *
 * mcumax_get_current_side() exposes different internal side values:
 * - 0x08 = white to move
 * - 0x10 = black to move
 */
static uint8_t mapMcumaxPiece(mcumax_piece mcumaxPiece) {
  // Treat 0x8 (which engine returns for empty) as EMPTY_SQUARE (0)
  if (mcumaxPiece == 0x8) {
    return EMPTY_SQUARE;
  }
  // Use mcu-max encoding directly for pieces
  return mcumaxPiece;
}

/**
 * Convert our [row][col] coordinates to mcu-max square encoding
 * Our board: row 0 = black's back rank (top of board)
 *            row 7 = white's back rank (bottom of board)
 *            col 0 = a-file (left), col 7 = h-file (right)
 * mcu-max: square encoded as 0xRF where R=display row (0-7), F=file (0-7, 0=a-file)
 */
static uint8_t rowColToMcumaxSquare(uint8_t row, uint8_t col) {
  // mcu-max has inverted board: rank 0 = black back rank (top), rank 7 = white back rank (bottom)
  // So display row maps directly to engine rank
  uint8_t rank = row;
  return (rank << 4) | col;
}

void initGameState(GameState* state) {
  if (!state) return;

  // Initialize mcu-max engine
  mcumax_init();

  // Sync board from engine
  syncBoardFromEngine(state);

  // Initialize game state
  mcumax_piece engineSide = mcumax_get_current_side();
  state->currentPlayer = currentPlayerFromMcumaxSide(engineSide);

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

void selectPieceAt(GameState* state, uint8_t row, uint8_t col) {
  if (!state || !isValidSquare(row, col)) return;
  
  // Get piece at cursor position
  uint8_t piece = getPiece(state, row, col);
  
  // Check if square is empty
  if (piece == EMPTY_SQUARE) return;
  
  // Check piece ownership: bit 0x8 set = black piece, clear = white piece (per mcu-max docs)
  bool pieceIsBlack = (piece & 0x8) != 0;
  bool currentPlayerIsBlack = (state->currentPlayer == 1);
  
  if (pieceIsBlack != currentPlayerIsBlack) {
    return;  // Wrong player's piece
  }
  
  // Query all legal moves from engine
  #define MAX_MOVES 50
  mcumax_move allMoves[MAX_MOVES];
  uint32_t moveCount = mcumax_search_valid_moves(allMoves, MAX_MOVES);
  
  // Filter moves for this piece
  uint8_t selectedSquare = rowColToMcumaxSquare(row, col);
  state->legalMoveCount = 0;
  
  for (uint32_t i = 0; i < moveCount && state->legalMoveCount < MAX_LEGAL_MOVES; i++) {
    if (allMoves[i].from == selectedSquare) {
      // Convert destination square to row/col
      uint8_t destSquare = allMoves[i].to;
      uint8_t destRank = (destSquare >> 4) & 0x7;  // Extract rank
      uint8_t destFile = destSquare & 0x7;         // Extract file
      uint8_t destRow = destRank;                  // Direct mapping now
      uint8_t destCol = destFile;
      
      state->legalMoves[state->legalMoveCount][0] = destRow;
      state->legalMoves[state->legalMoveCount][1] = destCol;
      state->legalMoveCount++;
    }
  }
  
  // Only set selection if piece has legal moves
  if (state->legalMoveCount > 0) {
    state->selectedRow = row;
    state->selectedCol = col;
  }
}

void clearSelection(GameState* state) {
  if (!state) return;
  
  state->selectedRow = NO_SELECTION;
  state->selectedCol = NO_SELECTION;
  state->legalMoveCount = 0;
  memset(state->legalMoves, 0, sizeof(state->legalMoves));
}

bool executeMove(GameState* state, uint8_t fromRow, uint8_t fromCol, uint8_t toRow, uint8_t toCol) {
  if (!state) return false;
  
  // Construct move
  mcumax_move move;
  move.from = rowColToMcumaxSquare(fromRow, fromCol);
  move.to = rowColToMcumaxSquare(toRow, toCol);
  
  // Execute move through engine
  if (mcumax_play_move(move)) {
    // Sync board from engine
    syncBoardFromEngine(state);
    
    // Sync current player from engine (engine switches side automatically).
    state->currentPlayer = currentPlayerFromMcumaxSide(mcumax_get_current_side());
    
    // Clear selection
    clearSelection(state);
    
    return true;
  }
  
  return false;
}
