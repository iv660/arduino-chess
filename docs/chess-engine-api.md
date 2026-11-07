# Chess Engine API Documentation

## Library: mcu-max v1.0.7

### Overview

mcu-max is an MCU-optimized chess game engine based on micro-Max. It provides complete chess rules including move generation, position evaluation, and game state management for embedded systems.

**Repository**: https://github.com/Gissio/mcu-max  
**License**: MIT  
**Memory**: 348 bytes SRAM, ~4.5KB Flash  
**Author**: Gissio

---

## Initialization

### `mcumax_init()`

```c
void mcumax_init(void);
```

**Description**: Resets the engine to the initial chess position and clears all state.

**Usage**:
```c
mcumax_init();  // Sets up starting position: white to move
```

**Memory**: Allocates internal board state (~348 bytes static)

**Call once**: At program startup or when starting a new game

---

## Position Management

### `mcumax_set_fen_position()`

```c
void mcumax_set_fen_position(const char *fen);
```

**Description**: Sets the board position from a FEN (Forsyth-Edwards Notation) string.

**Parameters**:
- `fen`: Null-terminated FEN string (e.g., "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

**Usage**:
```c
// Set starting position explicitly
mcumax_set_fen_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

// Set mid-game position for testing
mcumax_set_fen_position("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1");
```

**Note**: Useful for testing, loading saved games, or implementing puzzles

---

## Board Inspection

### `mcumax_get_piece()`

```c
mcumax_piece mcumax_get_piece(mcumax_square square);
```

**Description**: Returns the piece at the specified square.

**Parameters**:
- `square`: Board square in 0xRF format (R=rank 0-7, F=file 0-7)
  - Rank 0 = row 1 (white's back rank)
  - Rank 7 = row 8 (black's back rank)
  - File 0 = column a, File 7 = column h

**Returns**: `mcumax_piece` (uint8_t) with piece type and color:
- `MCUMAX_EMPTY` (0): No piece
- `MCUMAX_PAWN_UPSTREAM` (1): Pawn moving up the board
- `MCUMAX_PAWN_DOWNSTREAM` (2): Pawn moving down the board
- `MCUMAX_KNIGHT` (3): Knight
- `MCUMAX_KING` (4): King
- `MCUMAX_BISHOP` (5): Bishop
- `MCUMAX_ROOK` (6): Rook
- `MCUMAX_QUEEN` (7): Queen
- `MCUMAX_BLACK` (0x08): OR'd with piece type for black pieces

**Usage**:
```c
// Get piece at e2 (rank 6, file 4)
mcumax_piece piece = mcumax_get_piece(0x64);

// Check if it's a white pawn
if (piece == MCUMAX_PAWN_UPSTREAM) {
    // It's a white pawn
}

// Check if it's a black piece
if (piece & MCUMAX_BLACK) {
    // It's black's piece
}

// Get piece type (without color)
mcumax_piece type = piece & 0x07;
```

**Square calculation**:
```c
// Convert algebraic notation to square
// e.g., "e4" -> rank=4 (from '8'-'4'), file=4 (from 'e'-'a')
mcumax_square square = (('8' - '4') << 4) | ('e' - 'a');  // 0x44

// Convert coordinates to square
mcumax_square square = (rank << 4) | file;  // rank=0-7, file=0-7
```

### `mcumax_get_current_side()`

```c
mcumax_piece mcumax_get_current_side(void);
```

**Description**: Returns the side to move.

**Returns**:
- `0` (not MCUMAX_BLACK): White to move
- `MCUMAX_BLACK` (0x08): Black to move

**Usage**:
```c
if (mcumax_get_current_side() == MCUMAX_BLACK) {
    // It's black's turn
} else {
    // It's white's turn
}
```

---

## Move Generation

### `mcumax_search_valid_moves()`

```c
uint32_t mcumax_search_valid_moves(mcumax_move *buffer, uint32_t buffer_size);
```

**Description**: Generates all legal moves for the current position.

**Parameters**:
- `buffer`: Array to store valid moves
- `buffer_size`: Maximum number of moves to store

**Returns**: Number of valid moves found

**Move Structure**:
```c
typedef struct {
    mcumax_square from;  // Source square (0xRF format)
    mcumax_square to;    // Destination square (0xRF format)
} mcumax_move;
```

**Usage**:
```c
#define MAX_MOVES 50
mcumax_move moves[MAX_MOVES];
uint32_t count = mcumax_search_valid_moves(moves, MAX_MOVES);

// Iterate through all valid moves
for (uint32_t i = 0; i < count; i++) {
    mcumax_move move = moves[i];
    // Display or process move.from -> move.to
}

// Check for checkmate/stalemate (no legal moves)
if (count == 0) {
    // Game over
}
```

**Typical move counts**:
- Starting position: 20 moves
- Mid-game: 30-40 moves
- Endgame: 10-30 moves
- Maximum: ~218 moves (rare positions)

**Buffer size recommendation**: 50-80 moves (covers 99% of positions)

---

## Computer Move (AI)

### `mcumax_search_best_move()`

```c
mcumax_move mcumax_search_best_move(uint32_t node_max, uint32_t depth_max);
```

**Description**: Searches for the best move using minimax with alpha-beta pruning.

**Parameters**:
- `node_max`: Maximum number of nodes to search (controls thinking time)
- `depth_max`: Maximum search depth in ply (half-moves)

**Returns**: `mcumax_move` with best move found, or `MCUMAX_MOVE_INVALID` if no move exists

**Invalid move check**:
```c
mcumax_move move = mcumax_search_best_move(1000, 2);
if (move.from == MCUMAX_SQUARE_INVALID) {
    // No legal move (checkmate or stalemate)
}
```

**Usage**:
```c
// Easy difficulty: Quick, weak play (~700 Elo)
mcumax_move move = mcumax_search_best_move(500, 1);

// Medium difficulty: Moderate strength (~900 Elo)
mcumax_move move = mcumax_search_best_move(1000, 2);

// Hard difficulty: Strong play (~1100 Elo)
mcumax_move move = mcumax_search_best_move(2000, 3);
```

**Performance**:
| node_max | depth_max | Time (approx) | Strength |
|----------|-----------|---------------|----------|
| 500 | 1 | <1 second | Beginner |
| 1000 | 2 | 2-3 seconds | Casual |
| 2000 | 3 | 5-8 seconds | Intermediate |

**Note**: Times vary by position complexity. Opening positions are faster than tactical middlegame positions.

---

## Move Execution

### `mcumax_play_move()`

```c
bool mcumax_play_move(mcumax_move move);
```

**Description**: Executes a move and updates the board state.

**Parameters**:
- `move`: Move to play (from `mcumax_search_valid_moves()` or `mcumax_search_best_move()`)

**Returns**:
- `true`: Move was legal and executed
- `false`: Move was illegal (board state unchanged)

**Usage**:
```c
mcumax_move playerMove;
playerMove.from = 0x64;  // e2
playerMove.to = 0x44;    // e4

if (mcumax_play_move(playerMove)) {
    // Move successful, switch to opponent
} else {
    // Illegal move, ask for another move
}
```

**Side effects**:
- Updates internal board state
- Switches current side
- Updates castling rights, en passant, etc.

**Move validation**: Always returns false for illegal moves (no need to validate separately)

---

## Complete Game Loop Example

```c
#include <mcu-max.h>

void setup() {
    Serial.begin(115200);
    mcumax_init();  // Start new game
}

void loop() {
    // Display board
    displayBoard();
    
    // Check for game over
    mcumax_move validMoves[50];
    uint32_t moveCount = mcumax_search_valid_moves(validMoves, 50);
    if (moveCount == 0) {
        Serial.println("Game over!");
        return;
    }
    
    // Get player move (from joystick/buttons)
    mcumax_move playerMove = getPlayerMove();
    
    // Validate and execute player move
    if (!mcumax_play_move(playerMove)) {
        Serial.println("Illegal move!");
        return;
    }
    
    // Computer thinks
    Serial.println("Computer thinking...");
    mcumax_move computerMove = mcumax_search_best_move(1000, 2);
    
    if (computerMove.from == MCUMAX_SQUARE_INVALID) {
        Serial.println("Checkmate! You win!");
        return;
    }
    
    // Execute computer move
    mcumax_play_move(computerMove);
}

void displayBoard() {
    for (uint8_t rank = 0; rank < 8; rank++) {
        for (uint8_t file = 0; file < 8; file++) {
            mcumax_square square = (rank << 4) | file;
            mcumax_piece piece = mcumax_get_piece(square);
            
            // Draw piece on TFT display
            drawPiece(piece, file, 7 - rank);
        }
    }
}
```

---

## Memory Requirements

**Static allocation** (measured):
- **SRAM**: 348 bytes (engine state)
- **Flash**: ~4.5KB (code)

**Stack usage during search**:
- Depth 1: ~50 bytes
- Depth 2: ~100 bytes
- Depth 3: ~150 bytes

**Total system with integration test**: 594 bytes RAM, 6642 bytes Flash

**Remaining for Arduino Nano**:
- RAM: 1454 bytes available for game state and UI
- Flash: 24078 bytes available for display and control code

---

## Difficulty Level Recommendations

For our three difficulty levels (700/900/1100 Elo):

### Easy (700 Elo)
```c
#define EASY_NODES 500
#define EASY_DEPTH 1
mcumax_move move = mcumax_search_best_move(EASY_NODES, EASY_DEPTH);
```
- Think time: <1 second
- Makes obvious mistakes
- Good for beginners

### Medium (900 Elo)
```c
#define MEDIUM_NODES 1000
#define MEDIUM_DEPTH 2
mcumax_move move = mcumax_search_best_move(MEDIUM_NODES, MEDIUM_DEPTH);
```
- Think time: 2-3 seconds
- Sees basic tactics
- Good for casual players

### Hard (1100 Elo)
```c
#define HARD_NODES 2000
#define HARD_DEPTH 3
mcumax_move move = mcumax_search_best_move(HARD_NODES, HARD_DEPTH);
```
- Think time: 5-8 seconds
- Strong tactical play
- Challenging for most players

**Note**: These values will be tuned during gameplay testing in Feature 3.

---

## Limitations

- **No underpromotion**: Pawns always promote to Queen
- **No draw detection**: 50-move rule and repetition not tracked (acceptable for our MVP)
- **No opening book**: Plays from scratch (acceptable, adds variety)
- **No time controls**: Moves are calculated synchronously (acceptable for our use case)

All limitations are within MVP scope and acceptable for the XC Slim Pad hardware constraints.

---

## References

- GitHub: https://github.com/Gissio/mcu-max
- Based on: micro-Max 4.8 by H.G. Muller
- License: MIT
- Arduino example: mcu-max-serial.ino (in library examples)
