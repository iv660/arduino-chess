# Feature: Core game state and board representation

**From build-plan:** feature 2  
**Build attempt:** 1  
**Branch:** `feature/startup`  
**Status:** Verified ✓

## Goal

Define the runtime GameState structure that holds the chess board, player turn, cursor position, selected piece, legal moves, game phase, and special move tracking (castling rights, en passant). Integrate it with the mcu-max chess engine to maintain synchronized state between our representation and the library's internal board.

## In scope

- `GameState` struct with all fields from project-overview
- Board representation as 8×8 array with piece encoding compatible with mcu-max
- Current player tracking (White=0, Black=1)
- Cursor position (row, col) for UI navigation
- Selected piece tracking (row, col, or 255 when nothing selected)
- Legal moves buffer for selected piece
- Game phase enum (PLAYER_MOVE, COMPUTER_THINKING, GAME_OVER)
- Game result enum (IN_PROGRESS, WHITE_WINS, BLACK_WINS, DRAW)
- Special move state (castling rights bitfield, en passant target)
- Initialization function that sets up starting chess position
- Synchronization function that copies mcu-max board state into GameState.board
- Helper functions: piece type queries, board access, state reset

## Out of scope

- Display rendering (Feature 3)
- Input handling (Feature 5)
- Move execution logic (Feature 6)
- Computer AI integration (Feature 7)
- Piece bitmaps (Feature 4)
- Game flow control (Feature 8)

## Build loop

Per `stepReview: feature` - present all steps together for review before implementation.  
Per `checkpointCommits: disabled` - no per-step commits, single commit at `/complete`.

## Build steps

### Step 1: Define enums and constants
- Create `src/GameState.h`
- Define `GamePhase` enum (PLAYER_MOVE, COMPUTER_THINKING, GAME_OVER)
- Define `GameResult` enum (IN_PROGRESS, WHITE_WINS, BLACK_WINS, DRAW)
- Define piece encoding constants compatible with mcu-max piece types
- Define special values (EMPTY_SQUARE=0, NO_SELECTION=255, NO_EN_PASSANT=255)
- Define MAX_LEGAL_MOVES constant (conservatively 64)
- **Done when:** Header compiles, constants documented with comments

### Step 2: Define GameState structure
- Add `GameState` struct to `src/GameState.h` with all fields from overview:
  - `uint8_t board[8][8]` for piece positions
  - `uint8_t currentPlayer` (0=White, 1=Black)
  - `uint8_t cursorRow, cursorCol` for UI cursor
  - `uint8_t selectedRow, selectedCol` (255 when nothing selected)
  - `uint8_t legalMoves[MAX_LEGAL_MOVES][2]` for destination squares
  - `uint8_t legalMoveCount`
  - `GamePhase phase`
  - `GameResult result`
  - `uint8_t castlingRights` (bitfield: bit 0=White K-side, 1=White Q-side, 2=Black K-side, 3=Black Q-side)
  - `uint8_t enPassantTarget` (square index or 255)
- Document memory layout: 64 (board) + 10 (tracking) + 128 (legal moves) + 3 (flags) = ~205 bytes
- **Done when:** Struct compiles, size estimate confirmed with sizeof() in test code

### Step 3: Implement initialization and reset
- Create `src/GameState.cpp`
- Implement `void initGameState(GameState* state)`:
  - Initialize mcu-max engine with `mcumax_init()`
  - Call synchronization helper (next step) to copy starting position
  - Set currentPlayer = 0 (White)
  - Set cursor to (0, 0)
  - Set selectedRow/Col to NO_SELECTION
  - Set legalMoveCount = 0
  - Set phase = PLAYER_MOVE
  - Set result = IN_PROGRESS
  - Set castlingRights = 0x0F (all castling available)
  - Set enPassantTarget = NO_EN_PASSANT
- Implement `void resetGameState(GameState* state)` that calls initGameState
- **Done when:** Function compiles, initializes all fields

### Step 4: Implement board synchronization
- Implement `void syncBoardFromEngine(GameState* state)`:
  - Loop through all 64 squares (8 rows × 8 cols)
  - For each square, call `mcumax_get_piece()` with mcu-max square encoding
  - Map mcu-max piece constants to our encoding (document mapping)
  - Store in `state->board[row][col]`
- Document square coordinate mapping: our [row][col] ↔ mcu-max square format
- **Done when:** Function compiles, correctly copies piece positions from engine

### Step 5: Implement helper functions
- Implement `uint8_t getPiece(const GameState* state, uint8_t row, uint8_t col)`:
  - Bounds check (return EMPTY_SQUARE if out of range)
  - Return `state->board[row][col]`
- Implement `bool isValidSquare(uint8_t row, uint8_t col)`:
  - Return true if row < 8 && col < 8
- Implement `bool isPieceSelected(const GameState* state)`:
  - Return `state->selectedRow != NO_SELECTION`
- Implement `const char* getPhaseString(GamePhase phase)`:
  - Return "Player", "Computer", "Game Over" for debugging
- Implement `const char* getResultString(GameResult result)`:
  - Return "In Progress", "White Wins", "Black Wins", "Draw"
- **Done when:** All helpers compile and return correct values

### Step 6: Add test harness in main.cpp
- Replace integration test code with GameState test
- Include `GameState.h`
- In setup():
  - Create GameState instance
  - Call initGameState()
  - Print board state via Serial to verify starting position
  - Print struct size via Serial
  - Print memory usage estimate
- Verify that board shows standard chess starting position
- **Done when:** Test outputs correct starting position, memory usage under budget

## Files / areas

**New files:**
- `src/GameState.h` - Structure definition, enums, constants, function declarations
- `src/GameState.cpp` - Implementation of initialization, sync, and helpers

**Modified:**
- `src/main.cpp` - Replace mcu-max integration test with GameState test

## Data / contracts

### Piece Encoding

Map mcu-max piece constants to our board representation:
```
EMPTY = 0
WHITE pieces = mcu-max values for PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
BLACK pieces = WHITE values | 0x08 (or match mcu-max encoding exactly)
```

Document exact mapping after inspecting mcu-max.h piece constants.

### Square Coordinates

Our board: `board[row][col]` where row 0 = rank 8 (Black's back rank), row 7 = rank 1 (White's back rank), col 0 = a-file, col 7 = h-file.

mcu-max uses square encoding from its API - document the exact conversion in syncBoardFromEngine.

### Memory Budget

- GameState struct: ~205 bytes
- Current total: 594 bytes
- After Feature 2: ~799 bytes RAM (39% of 2KB, well under 1500 byte budget)

## Testing

- Build verification: `pio run -e nanoatmega328` passes (566 bytes RAM, 2026 bytes Flash)
- Unity tests: `pio test -e native` passes all 10 tests
  - test_gamestate_size: Validates struct is 205 bytes
  - test_is_valid_square: Validates square bounds checking
  - test_get_piece_bounds: Validates piece access with bounds
  - test_is_piece_selected: Validates selection state detection
  - test_phase_strings: Validates GamePhase enum string mapping
  - test_result_strings: Validates GameResult enum string mapping
  - test_board_state: Validates board array structure
  - test_game_state_fields: Validates all field types and ranges
  - test_legal_moves_buffer: Validates legal moves buffer size
  - test_constants: Validates enum values and piece encoding
- Memory check: RAM usage at 566 bytes (27.6%), well under 1500 byte budget

## Notes for the AI

- Use mcu-max API from Feature 1 documentation (`docs/chess-engine-api.md`)
- Read `mcu-max.h` to confirm exact piece constants and square encoding
- Keep all state in GameState struct - no global variables for game state
- Cursor and selection tracking is UI-only, doesn't affect engine
- Legal moves buffer will be filled in Feature 6 when move selection is implemented
- Castling rights and en passant will be updated when moves are executed (Feature 6/7)
- Current implementation assumes mcu-max maintains its own internal board - we sync from it
- All string functions should use F() macro to keep strings in Flash
- Test code in main.cpp is temporary - will be replaced with real game loop in Features 3-8
