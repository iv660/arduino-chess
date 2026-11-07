/**
 * GameState.cpp - Native test stub
 * This version is used for unit tests and doesn't depend on mcu-max.
 * The real implementation in src/ is used for the Arduino build.
 */

#include "GameState.h"
#include <string.h>

// Stub implementations for native testing
void initGameState(GameState* state) {
    if (!state) return;
    
    // For unit tests, just initialize fields
    // The real implementation syncs with mcu-max engine
    memset(state, 0, sizeof(GameState));
    state->currentPlayer = 0;
    state->cursorRow = 0;
    state->cursorCol = 0;
    state->selectedRow = NO_SELECTION;
    state->selectedCol = NO_SELECTION;
    state->legalMoveCount = 0;
    state->phase = PLAYER_MOVE;
    state->result = IN_PROGRESS;
    state->castlingRights = 0x0F;
    state->enPassantTarget = NO_EN_PASSANT;
}

void resetGameState(GameState* state) {
    initGameState(state);
}

void syncBoardFromEngine(GameState* state) {
    // Stub for testing - does nothing
    // Real implementation reads from mcu-max
    (void)state;
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
