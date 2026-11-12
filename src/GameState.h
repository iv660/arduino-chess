/**
 * GameState.h
 * Core game state and board representation for Arduino Chess
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdint.h>

// Game phase tracking
enum GamePhase : uint8_t {
  PLAYER_MOVE,
  COMPUTER_THINKING,
  GAME_OVER
};

// Game result
enum GameResult : uint8_t {
  IN_PROGRESS,
  WHITE_WINS,
  BLACK_WINS,
  DRAW
};

// Special marker values
constexpr uint8_t EMPTY_SQUARE = 0;
constexpr uint8_t NO_SELECTION = 255;
constexpr uint8_t NO_EN_PASSANT = 255;

// mcu-max exposes its internal side values through mcumax_get_current_side().
constexpr uint8_t MCUMAX_ENGINE_WHITE_SIDE = 0x08;
constexpr uint8_t MCUMAX_ENGINE_BLACK_SIDE = 0x10;

constexpr uint8_t currentPlayerFromMcumaxSide(uint8_t engineSide) {
  return engineSide == MCUMAX_ENGINE_BLACK_SIDE ? 1 : 0;
}

// Maximum legal moves for any position (conservative estimate)
constexpr uint8_t MAX_LEGAL_MOVES = 64;

/**
 * GameState structure
 * Memory layout: 64 (board) + 10 (tracking) + 128 (legal moves) + 3 (flags) = ~205 bytes
 */
struct GameState {
  uint8_t board[8][8];              // Piece positions (64 bytes)
  uint8_t currentPlayer;            // 0=White, 1=Black
  uint8_t cursorRow;                // UI cursor row (0-7)
  uint8_t cursorCol;                // UI cursor column (0-7)
  uint8_t selectedRow;              // Selected piece row (255 = NO_SELECTION)
  uint8_t selectedCol;              // Selected piece column (255 = NO_SELECTION)
  uint8_t legalMoves[MAX_LEGAL_MOVES][2];  // Legal destination squares (128 bytes)
  uint8_t legalMoveCount;           // Number of legal moves available
  GamePhase phase;                  // Current game phase
  GameResult result;                // Game outcome
  uint8_t castlingRights;           // Bitfield: bit 0=WK, 1=WQ, 2=BK, 3=BQ
  uint8_t enPassantTarget;          // Square index or NO_EN_PASSANT
};

// Function declarations
void initGameState(GameState* state);
void resetGameState(GameState* state);
void syncBoardFromEngine(GameState* state);
uint8_t getPiece(const GameState* state, uint8_t row, uint8_t col);
bool isValidSquare(uint8_t row, uint8_t col);
bool isPieceSelected(const GameState* state);
const char* getPhaseString(GamePhase phase);
const char* getResultString(GameResult result);

// Move selection helpers
void selectPieceAt(GameState* state, uint8_t row, uint8_t col);
void clearSelection(GameState* state);
bool executeMove(GameState* state, uint8_t fromRow, uint8_t fromCol, uint8_t toRow, uint8_t toCol);

#endif // GAMESTATE_H
