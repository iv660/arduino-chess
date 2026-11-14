/**
 * GameState.cpp
 * Implementation of core game state management
 */

#include "GameState.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <string.h>
#include <mcu-max.h>

static bool isOwnedByPlayer(uint8_t piece, uint8_t player) {
  return piece != EMPTY_SQUARE && ((piece & 0x8) != 0) == (player == 1);
}

static uint8_t squareIndex(uint8_t row, uint8_t col) {
  return row * 8 + col;
}

static bool isSquareAttacked(const GameState* state, uint8_t row, uint8_t col,
                             uint8_t attacker) {
  for (uint8_t sourceRow = 0; sourceRow < 8; sourceRow++) {
    for (uint8_t sourceCol = 0; sourceCol < 8; sourceCol++) {
      uint8_t piece = getPiece(state, sourceRow, sourceCol);
      if (!isOwnedByPlayer(piece, attacker)) {
        continue;
      }

      uint8_t pieceType = piece & 0x07;
      int8_t rowDelta = row - sourceRow;
      int8_t colDelta = col - sourceCol;
      int8_t absRowDelta = rowDelta < 0 ? -rowDelta : rowDelta;
      int8_t absColDelta = colDelta < 0 ? -colDelta : colDelta;

      if ((pieceType <= 2 && rowDelta == (attacker == 0 ? -1 : 1) &&
           absColDelta == 1) ||
          (pieceType == 3 && (absRowDelta * absRowDelta) +
           (absColDelta * absColDelta) == 5) ||
          (pieceType == 4 && absRowDelta <= 1 && absColDelta <= 1)) {
        return true;
      }

      bool diagonal = absRowDelta == absColDelta && absRowDelta > 0;
      bool straight = (rowDelta == 0) != (colDelta == 0);
      if ((diagonal && (pieceType == 5 || pieceType == 7)) ||
          (straight && (pieceType == 6 || pieceType == 7))) {
        int8_t rowStep = rowDelta == 0 ? 0 : (rowDelta < 0 ? -1 : 1);
        int8_t colStep = colDelta == 0 ? 0 : (colDelta < 0 ? -1 : 1);
        int8_t targetRow = sourceRow + rowStep;
        int8_t targetCol = sourceCol + colStep;
        bool clear = true;

        while (targetRow != row || targetCol != col) {
          if (getPiece(state, targetRow, targetCol) != EMPTY_SQUARE) {
            clear = false;
            break;
          }
          targetRow += rowStep;
          targetCol += colStep;
        }

        if (clear) {
          return true;
        }
      }
    }
  }

  return false;
}

static bool leavesKingInCheck(GameState* state, uint8_t fromRow, uint8_t fromCol,
                               uint8_t toRow, uint8_t toCol) {
  uint8_t movingPiece = getPiece(state, fromRow, fromCol);
  uint8_t capturedPiece = getPiece(state, toRow, toCol);
  uint8_t enPassantPiece = EMPTY_SQUARE;
  uint8_t rookPiece = EMPTY_SQUARE;
  uint8_t rookFromCol = 0;
  uint8_t rookToCol = 0;

  state->board[fromRow][fromCol] = EMPTY_SQUARE;
  state->board[toRow][toCol] = movingPiece;

  if ((movingPiece & 0x07) <= 2 && capturedPiece == EMPTY_SQUARE &&
      state->enPassantTarget == squareIndex(toRow, toCol)) {
    enPassantPiece = state->board[fromRow][toCol];
    state->board[fromRow][toCol] = EMPTY_SQUARE;
  }

  if ((movingPiece & 0x07) == 4 &&
      (fromCol > toCol ? fromCol - toCol : toCol - fromCol) == 2) {
    rookFromCol = toCol > fromCol ? 7 : 0;
    rookToCol = toCol > fromCol ? 5 : 3;
    rookPiece = state->board[fromRow][rookFromCol];
    state->board[fromRow][rookFromCol] = EMPTY_SQUARE;
    state->board[fromRow][rookToCol] = rookPiece;
  }

  uint8_t kingRow = 8;
  uint8_t kingCol = 8;
  for (uint8_t row = 0; row < 8 && kingRow == 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t piece = getPiece(state, row, col);
      if (isOwnedByPlayer(piece, state->currentPlayer) && (piece & 0x07) == 4) {
        kingRow = row;
        kingCol = col;
        break;
      }
    }
  }

  bool inCheck = kingRow == 8 || isSquareAttacked(state, kingRow, kingCol,
                                                   state->currentPlayer ^ 1);

  if (rookPiece != EMPTY_SQUARE) {
    state->board[fromRow][rookFromCol] = rookPiece;
    state->board[fromRow][rookToCol] = EMPTY_SQUARE;
  }
  if (enPassantPiece != EMPTY_SQUARE) {
    state->board[fromRow][toCol] = enPassantPiece;
  }
  state->board[fromRow][fromCol] = movingPiece;
  state->board[toRow][toCol] = capturedPiece;

  return inCheck;
}

static bool canCastle(GameState* state, uint8_t row, bool kingside) {
  uint8_t player = state->currentPlayer;
  uint8_t homeRow = player == 0 ? 7 : 0;
  uint8_t right = player == 0
                      ? (kingside ? CASTLING_WHITE_KINGSIDE : CASTLING_WHITE_QUEENSIDE)
                      : (kingside ? CASTLING_BLACK_KINGSIDE : CASTLING_BLACK_QUEENSIDE);
  uint8_t rookCol = kingside ? 7 : 0;
  uint8_t throughCol = kingside ? 5 : 3;
  uint8_t destinationCol = kingside ? 6 : 2;

  if (row != homeRow || (state->castlingRights & right) == 0 ||
      !isOwnedByPlayer(getPiece(state, row, rookCol), player) ||
      (getPiece(state, row, rookCol) & 0x07) != 6 ||
      getPiece(state, row, throughCol) != EMPTY_SQUARE ||
      getPiece(state, row, destinationCol) != EMPTY_SQUARE ||
      (!kingside && getPiece(state, row, 1) != EMPTY_SQUARE) ||
      isSquareAttacked(state, row, 4, player ^ 1)) {
    return false;
  }

  return !leavesKingInCheck(state, row, 4, row, throughCol);
}

static void addCandidateMove(GameState* state, uint8_t fromRow, uint8_t fromCol,
                             int8_t row, int8_t col) {
  if (!isValidSquare(row, col) || state->legalMoveCount >= MAX_LEGAL_MOVES ||
      leavesKingInCheck(state, fromRow, fromCol, row, col)) {
    return;
  }

  state->legalMoves[state->legalMoveCount][0] = row;
  state->legalMoves[state->legalMoveCount][1] = col;
  state->legalMoveCount++;
}

static void addRayCandidateMoves(GameState* state, uint8_t row, uint8_t col,
                                 int8_t rowStep, int8_t colStep) {
  int8_t targetRow = row + rowStep;
  int8_t targetCol = col + colStep;

  while (isValidSquare(targetRow, targetCol)) {
    uint8_t targetPiece = getPiece(state, targetRow, targetCol);
    if (isOwnedByPlayer(targetPiece, state->currentPlayer)) {
      return;
    }

    addCandidateMove(state, row, col, targetRow, targetCol);
    if (targetPiece != EMPTY_SQUARE) {
      return;
    }

    targetRow += rowStep;
    targetCol += colStep;
  }
}

static void addCandidateMovesForPiece(GameState* state, uint8_t row, uint8_t col,
                                      uint8_t pieceType) {
  switch (pieceType) {
    case 1:
    case 2: {
      int8_t direction = state->currentPlayer == 0 ? -1 : 1;
      int8_t startRow = state->currentPlayer == 0 ? 6 : 1;
      int8_t oneStepRow = row + direction;

      if (isValidSquare(oneStepRow, col) &&
          getPiece(state, oneStepRow, col) == EMPTY_SQUARE) {
        addCandidateMove(state, row, col, oneStepRow, col);

        int8_t twoStepRow = row + 2 * direction;
        if (row == startRow && isValidSquare(twoStepRow, col) &&
            getPiece(state, twoStepRow, col) == EMPTY_SQUARE) {
          addCandidateMove(state, row, col, twoStepRow, col);
        }
      }

      for (int8_t colStep = -1; colStep <= 1; colStep += 2) {
        int8_t targetCol = col + colStep;
        if (isValidSquare(oneStepRow, targetCol)) {
          uint8_t targetPiece = getPiece(state, oneStepRow, targetCol);
          if ((!isOwnedByPlayer(targetPiece, state->currentPlayer) &&
               targetPiece != EMPTY_SQUARE) ||
              state->enPassantTarget == squareIndex(oneStepRow, targetCol)) {
            addCandidateMove(state, row, col, oneStepRow, targetCol);
          }
        }
      }
      break;
    }

    case 3:
      for (int8_t rowStep = -2; rowStep <= 2; rowStep++) {
        for (int8_t colStep = -2; colStep <= 2; colStep++) {
          if ((rowStep * rowStep) + (colStep * colStep) == 5) {
            int8_t targetRow = row + rowStep;
            int8_t targetCol = col + colStep;
            if (isValidSquare(targetRow, targetCol) &&
                !isOwnedByPlayer(getPiece(state, targetRow, targetCol), state->currentPlayer)) {
              addCandidateMove(state, row, col, targetRow, targetCol);
            }
          }
        }
      }
      break;

    case 4:
      for (int8_t rowStep = -1; rowStep <= 1; rowStep++) {
        for (int8_t colStep = -1; colStep <= 1; colStep++) {
          if (rowStep != 0 || colStep != 0) {
            int8_t targetRow = row + rowStep;
            int8_t targetCol = col + colStep;
            if (isValidSquare(targetRow, targetCol) &&
                !isOwnedByPlayer(getPiece(state, targetRow, targetCol), state->currentPlayer)) {
              addCandidateMove(state, row, col, targetRow, targetCol);
            }
          }
        }
      }
      if (col == 4 && canCastle(state, row, true)) {
        addCandidateMove(state, row, col, row, 6);
      }
      if (col == 4 && canCastle(state, row, false)) {
        addCandidateMove(state, row, col, row, 2);
      }
      break;

    case 5:
      addRayCandidateMoves(state, row, col, -1, -1);
      addRayCandidateMoves(state, row, col, -1, 1);
      addRayCandidateMoves(state, row, col, 1, -1);
      addRayCandidateMoves(state, row, col, 1, 1);
      break;

    case 6:
      addRayCandidateMoves(state, row, col, -1, 0);
      addRayCandidateMoves(state, row, col, 0, -1);
      addRayCandidateMoves(state, row, col, 0, 1);
      addRayCandidateMoves(state, row, col, 1, 0);
      break;

    case 7:
      addRayCandidateMoves(state, row, col, -1, -1);
      addRayCandidateMoves(state, row, col, -1, 1);
      addRayCandidateMoves(state, row, col, 1, -1);
      addRayCandidateMoves(state, row, col, 1, 1);
      addRayCandidateMoves(state, row, col, -1, 0);
      addRayCandidateMoves(state, row, col, 0, -1);
      addRayCandidateMoves(state, row, col, 0, 1);
      addRayCandidateMoves(state, row, col, 1, 0);
      break;
  }
}

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

static uint8_t clearRookCastlingRight(uint8_t castlingRights, uint8_t row, uint8_t col) {
  if (row == 7 && col == 7) {
    castlingRights &= ~CASTLING_WHITE_KINGSIDE;
  } else if (row == 7 && col == 0) {
    castlingRights &= ~CASTLING_WHITE_QUEENSIDE;
  } else if (row == 0 && col == 7) {
    castlingRights &= ~CASTLING_BLACK_KINGSIDE;
  } else if (row == 0 && col == 0) {
    castlingRights &= ~CASTLING_BLACK_QUEENSIDE;
  }
  return castlingRights;
}

static uint8_t updatedCastlingRights(const GameState* state, uint8_t movingPiece,
                                     uint8_t capturedPiece, uint8_t fromRow,
                                     uint8_t fromCol, uint8_t toRow, uint8_t toCol) {
  uint8_t castlingRights = state->castlingRights;
  uint8_t pieceType = movingPiece & 0x07;

  if (pieceType == 4) {
    if (isOwnedByPlayer(movingPiece, 0)) {
      castlingRights &= ~(CASTLING_WHITE_KINGSIDE | CASTLING_WHITE_QUEENSIDE);
    } else {
      castlingRights &= ~(CASTLING_BLACK_KINGSIDE | CASTLING_BLACK_QUEENSIDE);
    }
  } else if (pieceType == 6) {
    castlingRights = clearRookCastlingRight(castlingRights, fromRow, fromCol);
  }

  if ((capturedPiece & 0x07) == 6) {
    castlingRights = clearRookCastlingRight(castlingRights, toRow, toCol);
  }

  return castlingRights;
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
  state->castlingRights = INITIAL_CASTLING_RIGHTS;
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
  
  if (!isOwnedByPlayer(piece, state->currentPlayer)) {
    return;  // Wrong player's piece
  }

  state->legalMoveCount = 0;
  addCandidateMovesForPiece(state, row, col, piece & 0x07);
  
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

  uint8_t movingPiece = getPiece(state, fromRow, fromCol);
  uint8_t capturedPiece = getPiece(state, toRow, toCol);
  uint8_t enPassantTarget = NO_EN_PASSANT;
  if ((movingPiece & 0x07) <= 2 &&
      ((fromRow > toRow ? fromRow - toRow : toRow - fromRow) == 2)) {
    enPassantTarget = squareIndex((fromRow + toRow) / 2, fromCol);
  }
  
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
    state->enPassantTarget = enPassantTarget;
    state->castlingRights = updatedCastlingRights(state, movingPiece, capturedPiece,
                                                  fromRow, fromCol, toRow, toCol);
    
    // Clear selection
    clearSelection(state);
    
    return true;
  }
  
  return false;
}
