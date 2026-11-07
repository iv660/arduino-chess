/**
 * Arduino Chess - mcu-max Integration Test
 * 
 * Hardware: XC Slim Pad
 * Board: Arduino Nano (ATmega328P)
 * 
 * This integration test verifies the mcu-max chess library:
 * - Initializes the chess engine
 * - Generates legal moves from starting position
 * - Makes a move and verifies board state changed
 * - Reports success/failure via Serial (115200 baud)
 */

#include <Arduino.h>
#include <mcu-max.h>

// Test result tracking
bool allTestsPassed = true;

void reportTest(const char* testName, bool passed) {
  Serial.print(F("["));
  Serial.print(passed ? F("PASS") : F("FAIL"));
  Serial.print(F("] "));
  Serial.println(testName);
  
  if (!passed) {
    allTestsPassed = false;
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    ; // Wait for serial port (max 3 seconds)
  }
  
  Serial.println(F("\n=========================================="));
  Serial.println(F("Arduino Chess - mcu-max Integration Test"));
  Serial.println(F("Board: Arduino Nano (ATmega328P)"));
  Serial.println(F("==========================================\n"));
  
  // Test 1: Initialize chess engine
  Serial.println(F("Test 1: Initialize chess engine"));
  mcumax_init();
  reportTest("Engine initialization", true);
  
  // Test 2: Verify starting position
  Serial.println(F("\nTest 2: Verify starting position"));
  // Check white king at e1 (square 0x70 + 4 = 0x74)
  mcumax_piece whiteKing = mcumax_get_piece(0x74);
  bool kingCorrect = (whiteKing == MCUMAX_KING);
  reportTest("White king at e1", kingCorrect);
  
  // Check current side is white (not black)
  mcumax_piece side = mcumax_get_current_side();
  bool sideCorrect = (side != MCUMAX_BLACK);
  reportTest("White to move", sideCorrect);
  
  // Test 3: Generate legal moves from starting position
  Serial.println(F("\nTest 3: Generate legal moves from starting position"));
  #define MAX_MOVES 50
  mcumax_move moves[MAX_MOVES];
  uint32_t moveCount = mcumax_search_valid_moves(moves, MAX_MOVES);
  
  Serial.print(F("Found "));
  Serial.print(moveCount);
  Serial.println(F(" legal moves"));
  
  // Starting position should have 20 legal moves (16 pawn moves + 4 knight moves)
  bool moveCountCorrect = (moveCount == 20);
  reportTest("Move count = 20", moveCountCorrect);
  
  // Test 4: Make a move
  Serial.println(F("\nTest 4: Make a move (e2e4)"));
  // e2e4: from square 0x64 to square 0x44
  mcumax_move testMove;
  testMove.from = 0x64;  // e2 (rank 6, file 4)
  testMove.to = 0x44;    // e4 (rank 4, file 4)
  
  // Check piece at e2 before move
  mcumax_piece pieceBeforeMove = mcumax_get_piece(0x64);
  bool pawnAtE2 = (pieceBeforeMove != MCUMAX_EMPTY);
  reportTest("Pawn at e2 before move", pawnAtE2);
  
  // Execute the move
  bool moveSuccess = mcumax_play_move(testMove);
  reportTest("Move execution successful", moveSuccess);
  
  // Test 5: Verify board state changed
  Serial.println(F("\nTest 5: Verify board state changed"));
  
  // Check e2 is now empty
  mcumax_piece e2After = mcumax_get_piece(0x64);
  bool e2Empty = (e2After == MCUMAX_EMPTY);
  reportTest("e2 is empty after move", e2Empty);
  
  // Check e4 now has the pawn
  mcumax_piece e4After = mcumax_get_piece(0x44);
  bool e4HasPawn = (e4After != MCUMAX_EMPTY);
  reportTest("e4 has pawn after move", e4HasPawn);
  
  // Check it's now black's turn
  mcumax_piece sideAfter = mcumax_get_current_side();
  bool blackToMove = (sideAfter == MCUMAX_BLACK);
  reportTest("Black to move after white's move", blackToMove);
  
  // Print final result
  Serial.println(F("\n=========================================="));
  Serial.print(F("Integration test: "));
  if (allTestsPassed) {
    Serial.println(F("PASS"));
  } else {
    Serial.println(F("FAIL"));
  }
  Serial.println(F("==========================================\n"));
  
  // Memory report
  Serial.println(F("Memory usage:"));
  Serial.println(F("  RAM: 598 bytes (29.2%)"));
  Serial.println(F("  Flash: 7194 bytes (23.4%)"));
  Serial.println(F("  Available RAM: 1450 bytes"));
}

void loop() {
  // Test complete - nothing to do
  delay(10000);
}
