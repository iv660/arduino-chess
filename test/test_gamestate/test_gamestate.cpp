/**
 * Unit tests for GameState
 * Run with: pio test -e native
 * 
 * Note: Tests that require mcu-max engine (syncBoardFromEngine, initGameState)
 * are hardware integration tests and should be verified on actual Arduino.
 * These unit tests cover the pure logic that doesn't depend on the engine.
 */

#include <unity.h>
#include "GameState.h"

GameState testState;

void setUp(void) {
    // Reset state before each test
    testState = {};
}

void tearDown(void) {
    // Cleanup after each test
}

// Test 1: GameState structure size within budget
void test_gamestate_size(void) {
    size_t actualSize = sizeof(GameState);
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(210, actualSize, 
        "GameState exceeds 210 byte budget");
}

// Test 2: Helper - isValidSquare
void test_is_valid_square(void) {
    TEST_ASSERT_TRUE(isValidSquare(0, 0));
    TEST_ASSERT_TRUE(isValidSquare(7, 7));
    TEST_ASSERT_TRUE(isValidSquare(3, 4));
    TEST_ASSERT_TRUE(isValidSquare(0, 7));
    TEST_ASSERT_TRUE(isValidSquare(7, 0));
    
    TEST_ASSERT_FALSE(isValidSquare(8, 0));
    TEST_ASSERT_FALSE(isValidSquare(0, 8));
    TEST_ASSERT_FALSE(isValidSquare(8, 8));
    TEST_ASSERT_FALSE(isValidSquare(255, 0));
    TEST_ASSERT_FALSE(isValidSquare(0, 255));
    TEST_ASSERT_FALSE(isValidSquare(255, 255));
}

// Test 3: Helper - getPiece bounds checking
void test_get_piece_bounds(void) {
    // Set up a known piece
    testState.board[3][4] = 0x05; // Some piece value
    
    // Valid access
    TEST_ASSERT_EQUAL_UINT8(0x05, getPiece(&testState, 3, 4));
    
    // Out of bounds should return EMPTY_SQUARE
    TEST_ASSERT_EQUAL_UINT8(EMPTY_SQUARE, getPiece(&testState, 8, 0));
    TEST_ASSERT_EQUAL_UINT8(EMPTY_SQUARE, getPiece(&testState, 0, 8));
    TEST_ASSERT_EQUAL_UINT8(EMPTY_SQUARE, getPiece(&testState, 255, 255));
    
    // Null pointer safety
    TEST_ASSERT_EQUAL_UINT8(EMPTY_SQUARE, getPiece(nullptr, 0, 0));
}

// Test 4: Helper - isPieceSelected
void test_is_piece_selected(void) {
    // Initially nothing selected
    testState.selectedRow = NO_SELECTION;
    testState.selectedCol = NO_SELECTION;
    TEST_ASSERT_FALSE(isPieceSelected(&testState));
    
    // Select a piece
    testState.selectedRow = 6;
    testState.selectedCol = 4;
    TEST_ASSERT_TRUE(isPieceSelected(&testState));
    
    // Different selection
    testState.selectedRow = 0;
    testState.selectedCol = 0;
    TEST_ASSERT_TRUE(isPieceSelected(&testState));
    
    // Deselect
    testState.selectedRow = NO_SELECTION;
    testState.selectedCol = NO_SELECTION;
    TEST_ASSERT_FALSE(isPieceSelected(&testState));
    
    // Null safety
    TEST_ASSERT_FALSE(isPieceSelected(nullptr));
}

// Test 5: Helper - phase strings
void test_phase_strings(void) {
    const char* str = getPhaseString(PLAYER_MOVE);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("Player", str);
    
    str = getPhaseString(COMPUTER_THINKING);
    TEST_ASSERT_EQUAL_STRING("Computer", str);
    
    str = getPhaseString(GAME_OVER);
    TEST_ASSERT_EQUAL_STRING("Game Over", str);
}

// Test 6: Helper - result strings
void test_result_strings(void) {
    const char* str = getResultString(IN_PROGRESS);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("In Progress", str);
    
    str = getResultString(WHITE_WINS);
    TEST_ASSERT_EQUAL_STRING("White Wins", str);
    
    str = getResultString(BLACK_WINS);
    TEST_ASSERT_EQUAL_STRING("Black Wins", str);
    
    str = getResultString(DRAW);
    TEST_ASSERT_EQUAL_STRING("Draw", str);
}

// Test 7: Board state manipulation
void test_board_state(void) {
    // Clear board
    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t col = 0; col < 8; col++) {
            testState.board[row][col] = EMPTY_SQUARE;
        }
    }
    
    // Place some pieces
    testState.board[0][0] = 0x06; // Rook-like value
    testState.board[7][4] = 0x04; // King-like value
    testState.board[3][3] = 0x01; // Pawn-like value
    
    // Verify through getPiece
    TEST_ASSERT_EQUAL_UINT8(0x06, getPiece(&testState, 0, 0));
    TEST_ASSERT_EQUAL_UINT8(0x04, getPiece(&testState, 7, 4));
    TEST_ASSERT_EQUAL_UINT8(0x01, getPiece(&testState, 3, 3));
    TEST_ASSERT_EQUAL_UINT8(EMPTY_SQUARE, getPiece(&testState, 4, 4));
}

// Test 8: Game state fields
void test_game_state_fields(void) {
    // Set various game state values
    testState.currentPlayer = 1;
    testState.cursorRow = 5;
    testState.cursorCol = 3;
    testState.selectedRow = 6;
    testState.selectedCol = 4;
    testState.legalMoveCount = 15;
    testState.phase = COMPUTER_THINKING;
    testState.result = WHITE_WINS;
    testState.castlingRights = 0x0C;
    testState.enPassantTarget = 42;
    
    // Verify all fields stored correctly
    TEST_ASSERT_EQUAL_UINT8(1, testState.currentPlayer);
    TEST_ASSERT_EQUAL_UINT8(5, testState.cursorRow);
    TEST_ASSERT_EQUAL_UINT8(3, testState.cursorCol);
    TEST_ASSERT_EQUAL_UINT8(6, testState.selectedRow);
    TEST_ASSERT_EQUAL_UINT8(4, testState.selectedCol);
    TEST_ASSERT_EQUAL_UINT8(15, testState.legalMoveCount);
    TEST_ASSERT_EQUAL(COMPUTER_THINKING, testState.phase);
    TEST_ASSERT_EQUAL(WHITE_WINS, testState.result);
    TEST_ASSERT_EQUAL_HEX8(0x0C, testState.castlingRights);
    TEST_ASSERT_EQUAL_UINT8(42, testState.enPassantTarget);
}

// Test 9: Legal moves buffer
void test_legal_moves_buffer(void) {
    testState.legalMoveCount = 3;
    testState.legalMoves[0][0] = 6;
    testState.legalMoves[0][1] = 4;
    testState.legalMoves[1][0] = 5;
    testState.legalMoves[1][1] = 4;
    testState.legalMoves[2][0] = 4;
    testState.legalMoves[2][1] = 4;
    
    TEST_ASSERT_EQUAL_UINT8(3, testState.legalMoveCount);
    TEST_ASSERT_EQUAL_UINT8(6, testState.legalMoves[0][0]);
    TEST_ASSERT_EQUAL_UINT8(4, testState.legalMoves[0][1]);
    TEST_ASSERT_EQUAL_UINT8(5, testState.legalMoves[1][0]);
    TEST_ASSERT_EQUAL_UINT8(4, testState.legalMoves[1][1]);
    TEST_ASSERT_EQUAL_UINT8(4, testState.legalMoves[2][0]);
    TEST_ASSERT_EQUAL_UINT8(4, testState.legalMoves[2][1]);
}

// Test 10: Constants validation
void test_constants(void) {
    TEST_ASSERT_EQUAL_UINT8(0, EMPTY_SQUARE);
    TEST_ASSERT_EQUAL_UINT8(255, NO_SELECTION);
    TEST_ASSERT_EQUAL_UINT8(255, NO_EN_PASSANT);
    TEST_ASSERT_EQUAL_UINT8(64, MAX_LEGAL_MOVES);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gamestate_size);
    RUN_TEST(test_is_valid_square);
    RUN_TEST(test_get_piece_bounds);
    RUN_TEST(test_is_piece_selected);
    RUN_TEST(test_phase_strings);
    RUN_TEST(test_result_strings);
    RUN_TEST(test_board_state);
    RUN_TEST(test_game_state_fields);
    RUN_TEST(test_legal_moves_buffer);
    RUN_TEST(test_constants);
    
    return UNITY_END();
}
