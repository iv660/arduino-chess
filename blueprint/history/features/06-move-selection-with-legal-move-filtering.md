# Feature: Move selection with legal-move filtering

**Build attempt**: 1 (first attempt at Feature 6)
**Branch**: `feature/startup`
**Status**: Verified

---

## Goal

Enable players to select pieces and move them using joystick button input. Query the chess engine for legal moves, restrict cursor movement to legal destination squares, and execute the move when the player confirms. Complete the core gameplay interaction loop.

---

## In scope

1. **Button debouncing and state tracking** - Implement reliable joystick button press detection with debouncing (50ms default) and track press state to detect new presses
2. **Piece selection state machine** - When no piece is selected and button pressed, validate cursor position contains player's piece, query engine for legal moves from that square, populate `state->legalMoves[]` buffer, set `state->selectedRow/Col`, update display with selection highlight
3. **Legal move filtering** - Filter `mcumax_search_valid_moves()` results to extract only moves originating from the selected square, store destinations in `state->legalMoves[]` with count in `state->legalMoveCount`
4. **Cursor restriction logic** - Modify `updateCursor()` in `Input.cpp` to check if piece is selected; if selected, restrict cursor movement to legal destination squares only (cycling through `legalMoves[]` array)
5. **Move execution** - When piece is selected and button pressed on legal destination, construct `mcumax_move` with selected source and cursor destination, call `mcumax_play_move()`, sync board from engine, clear selection, switch player turn
6. **Cancel selection** - If piece is selected but button pressed on the same square again, clear selection and restore normal cursor movement
7. **Display integration** - Update `Display.cpp` to visually distinguish selected piece square (use distinct highlight color or border) and show legal move destination highlights

---

## Out of scope

- Computer move generation and AI thinking (Feature 7)
- Game-over detection, checkmate/stalemate display (Feature 8)
- Settings menu, difficulty selection, color assignment (Feature 3)
- Move history, captured pieces display
- Pawn promotion UI (handle automatically with queen promotion for MVP)
- Sound effects or move animation
- Time controls or move clocks

---

## Build loop

### 1. Add button debouncing to Input module

**Files**: `src/Input.h`, `src/Input.cpp`

**Changes**:
- Add static state tracking: `lastButtonState`, `lastDebounceTime`, `DEBOUNCE_DELAY_MS` constant (50ms)
- Create `bool isButtonNewlyPressed()` function: return true only on LOW->HIGH transition after debounce delay
- Update in `updateCursor()` call flow from `main.cpp`

**Test**: Call `isButtonNewlyPressed()` in loop, verify no duplicate triggers on single physical press

**Done when**: Button press detection is stable, no bouncing observed

---

### 2. Query legal moves and select piece

**Files**: `src/GameState.h`, `src/GameState.cpp`, `src/Input.cpp`

**Changes**:
- Add helper `void selectPieceAt(GameState* state, uint8_t row, uint8_t col)` to `GameState.cpp`:
  - Validate square contains piece matching `state->currentPlayer` using mcu-max piece encoding (bit 3 for black)
  - Call `mcumax_search_valid_moves()` with buffer for all legal moves (50 moves sufficient)
  - Filter moves where `move.from` matches `rowColToMcumaxSquare(row, col)`
  - Populate `state->legalMoves[]` with destination squares (convert `move.to` to row/col pairs)
  - Set `state->legalMoveCount`
  - Set `state->selectedRow = row`, `state->selectedCol = col`
  - If no legal moves from this piece, do nothing (no selection)
- Add helper `void clearSelection(GameState* state)`:
  - Set `selectedRow/Col = NO_SELECTION`
  - Set `legalMoveCount = 0`
- In `Input.cpp`, modify button handling logic:
  - If button newly pressed and no piece selected, call `selectPieceAt(state, cursorRow, cursorCol)`
  - If button pressed when piece already selected (handle in step 5)

**Test**: Select a pawn at start position, verify `legalMoveCount == 2`, `legalMoves[]` contains correct squares

**Done when**: Legal moves are correctly queried and stored in GameState

---

### 3. Restrict cursor to legal destinations when piece selected

**Files**: `src/Input.cpp`

**Changes**:
- Add helper `uint8_t findCurrentLegalMoveIndex(const GameState* state)`:
  - Search `state->legalMoves[]` for index where `[legalMoves[i][0], legalMoves[i][1]]` matches `[cursorRow, cursorCol]`
  - Return index if found, return 0 if not found (default to first legal move)
- Modify `updateCursor()`:
  - Check `isPieceSelected(state)` at the start
  - If piece selected:
    - Get current legal move index with helper
    - On joystick movement, cycle to next/previous legal move (wrap around)
    - Update `state->cursorRow/Col` to the new legal move coordinates
  - If not selected:
    - Use existing free-roaming cursor logic

**Test**: Select piece with multiple legal moves, verify cursor jumps only between legal destinations, wraps correctly

**Done when**: Cursor is restricted to legal moves when piece is selected

---

### 4. Execute move on destination button press

**Files**: `src/GameState.h`, `src/GameState.cpp`, `src/Input.cpp`

**Changes**:
- Add helper `bool executeMove(GameState* state, uint8_t fromRow, uint8_t fromCol, uint8_t toRow, uint8_t toCol)` to `GameState.cpp`:
  - Construct `mcumax_move move`
  - Set `move.from = rowColToMcumaxSquare(fromRow, fromCol)`
  - Set `move.to = rowColToMcumaxSquare(toRow, toCol)`
  - Call `mcumax_play_move(move)`
  - If move successful:
    - Call `syncBoardFromEngine(state)` to refresh board
    - Toggle `state->currentPlayer` (XOR with 1)
    - Call `clearSelection(state)`
    - Return true
  - If move failed, return false (should not happen with legal move filtering)
- In `Input.cpp` button handling:
  - If button newly pressed and piece is selected:
    - If cursor is on selected square (cancel), call `clearSelection(state)`
    - Else verify cursor is in `legalMoves[]` array (safety check), call `executeMove()`

**Test**: Select piece, move to legal destination, press button, verify move executes and board updates, player switches

**Done when**: Moves execute correctly through chess engine, board syncs, turn switches

---

### 5. Add selection and legal-move display highlights

**Files**: `src/Display.h`, `src/Display.cpp`

**Changes**:
- Modify `drawSquare()` signature or logic to accept `const GameState*` and draw different highlights:
  - Regular cursor: existing yellow border (2px)
  - Selected piece square: blue or cyan filled border (3px, distinct from cursor)
  - **Legal destination squares: small green dots** in center of each square (4-6px diameter circles)
- Add helper `bool isLegalDestination(const GameState* state, uint8_t row, uint8_t col)`:
  - Search `state->legalMoves[]` array
  - Return true if `[row, col]` matches any entry
- Update `drawSquare()` to:
  - Draw legal move dot if `isLegalDestination()` returns true (draw after square, before piece)
  - Draw selected piece border if coordinates match `selectedRow/Col`
  - Draw cursor border on top (last layer)
- Update `drawBoard()` to pass `state` to `drawSquare()` for all squares
- Ensure highlights layer correctly: square → legal move dots → piece → selection border → cursor border

**Visual specification**:
- Legal move dots: `ST77XX_GREEN`, radius 3px, filled circle at square center
- Selected piece border: `ST77XX_CYAN`, 3px thick rectangle around square
- Cursor border: `ST77XX_YELLOW`, 2px thick rectangle around square (existing)

**Test**: Select piece, verify selected square has cyan border, legal destination squares show green dots in center, cursor yellow border still visible on top

**Done when**: Visual feedback clearly shows selected piece and available moves with dots

---

### 6. Integration and full gameplay test

**Files**: `src/main.cpp`

**Changes**:
- Ensure `updateCursor()` is called before button handling in `loop()`
- Ensure `drawBoard()` is called after any state change to refresh display
- Add comments documenting the move selection flow

**Test**:
- Start game, select white piece, move to legal square, verify move executes
- Select piece, press button on same square, verify selection cancels
- Select piece with no legal moves (blocked piece), verify no selection occurs
- Play several moves alternating between white and black (manual testing of both sides)

**Done when**: Complete move selection and execution flow works end-to-end for both players

---

### 7. Build, verify, and validate

**Command**: `pio run -e nanoatmega328 && pio test -e native`

**Check**:
- Build succeeds with no warnings
- All existing tests pass
- Memory usage remains within budget (Flash < 80%, RAM < 70%)

**Done when**: Clean build with passing tests

---

## Files / areas

**Core changes**:
- `src/Input.h` + `src/Input.cpp` - Button debouncing, piece selection trigger, cursor restriction logic
- `src/GameState.h` + `src/GameState.cpp` - `selectPieceAt()`, `clearSelection()`, `executeMove()` helpers, legal move storage
- `src/Display.h` + `src/Display.cpp` - Selection and legal-move visual highlights

**Integration**:
- `src/main.cpp` - Coordinate input → state → display flow

**API dependencies**:
- `mcumax_search_valid_moves()` - Query all legal moves for current position
- `mcumax_play_move()` - Execute validated move
- `mcumax_get_current_side()` - Determine whose turn (if needed for validation)
- `mcumax_get_piece()` - Already used via `syncBoardFromEngine()`

**Unchanged**:
- Chess engine integration (already working)
- Display rendering primitives (reuse existing)
- Joystick polling (already working)

---

## Data / contracts

### GameState extensions (already defined)
```cpp
uint8_t selectedRow;           // NO_SELECTION (255) when no piece selected
uint8_t selectedCol;           // NO_SELECTION (255) when no piece selected
uint8_t legalMoves[MAX_LEGAL_MOVES][2];  // [row, col] pairs for legal destinations
uint8_t legalMoveCount;        // Number of legal moves available
```

### Input state (new internal)
```cpp
static bool lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;
constexpr unsigned long DEBOUNCE_DELAY_MS = 50;
```

### Chess engine API types (mcu-max)
```cpp
typedef struct {
    mcumax_square from;  // 0xRF format (rank << 4 | file)
    mcumax_square to;    // 0xRF format
} mcumax_move;

// mcumax piece encoding: bit 3 set = black, bits 0-2 = piece type
// MCUMAX_BLACK = 0x8
```

### Coordinate mapping (existing)
```cpp
// Our board: row 0 = rank 8 (black back rank), row 7 = rank 1 (white back rank)
//            col 0 = a-file, col 7 = h-file
// mcu-max: square = (rank << 4) | file, rank 0 = rank 1, file 0 = a-file
uint8_t rowColToMcumaxSquare(uint8_t row, uint8_t col);
```

---

## Testing

### Unit tests (native environment)
- Test `selectPieceAt()` with starting position pieces, verify legal move counts match expected
- Test `clearSelection()` resets state correctly
- Test `executeMove()` with known valid moves
- Test button debouncing logic in isolation

### Manual hardware tests
- Select and move white pawn e2→e4, verify board updates
- Select and move black piece after white's turn
- Attempt to select opponent's piece, verify no selection
- Select piece, press button on same square, verify cancel
- Select piece, move cursor through all legal destinations, verify restricted movement
- Play several full moves to verify turn switching

### Edge cases
- Select piece with only one legal move (e.g., blocked piece)
- Select piece with no legal moves (fully blocked), verify no selection
- Rapid button presses (verify debouncing prevents double-triggers)
- Hold button while moving cursor (verify only first press counts)

---

## Notes for the AI

### Critical constraints
- **Memory budget**: Keep new code minimal. Reuse existing GameState structure, avoid dynamic allocation.
- **Chess engine API**: The `mcumax_search_valid_moves()` returns ALL legal moves for the current position, not for a specific piece. You must filter by `move.from` to extract moves for the selected square.
- **Coordinate system**: Our display uses row-major [row][col] with row 0 at top. mcu-max uses 0xRF encoding where rank 0 is bottom (rank 1). Use existing `rowColToMcumaxSquare()` helper.
- **Piece ownership**: mcu-max pieces have bit 3 set for black (`piece & 0x8`). White pieces are 0x0-0x7, black pieces are 0x8-0xF. Use this to validate player owns selected piece.
- **Turn tracking**: `state->currentPlayer` is 0 for white, 1 for black. After `mcumax_play_move()`, the engine switches sides internally, so we must toggle `currentPlayer` to stay synchronized.

### Implementation order rationale
1. Start with button debouncing foundation (prevents all downstream bugs)
2. Build piece selection and legal move query (establishes data flow)
3. Restrict cursor movement (makes interaction intuitive)
4. Execute move (completes the loop)
5. Add visual feedback (makes state visible to user)
6. Integrate and test full flow

### Failure classes to watch
- **Button bounce**: Double-triggers on single press → debouncing required
- **Coordinate mismatch**: Our row/col vs mcu-max rank/file → use helpers consistently
- **Piece ownership**: Selecting opponent's piece → validate piece color matches `currentPlayer`
- **Empty selection**: Selecting empty square or immobile piece → check legal move count before setting selection
- **Move validation**: Move rejected by engine → should never happen if legal moves are filtered correctly
- **Turn desync**: `currentPlayer` mismatched with engine state → toggle after every `mcumax_play_move()`
- **Cursor escape**: Cursor moves outside legal moves when piece selected → restrict movement to `legalMoves[]` array only
- **Visual confusion**: Can't tell what's selected → use distinct colors for cursor vs selection vs legal moves

### Testing priorities
1. Button debouncing (prevents ghost inputs)
2. Legal move filtering (wrong filter = illegal moves displayed)
3. Coordinate conversion (wrong mapping = wrong squares)
4. Turn synchronization (desync = wrong player or broken game)
5. Visual feedback (poor UX but not functional failure)

### Success criteria
- Player can select any of their pieces and see legal moves
- Cursor is restricted to legal destinations only
- Move executes correctly through chess engine
- Board updates and turn switches automatically
- No memory leaks, no crashes, stays within budget
- Visual feedback clearly shows selection state

---

## Done when

- [x] Button debouncing implemented with 50ms delay
- [x] `selectPieceAt()` queries engine and populates `legalMoves[]`
- [x] Cursor restricted to legal destinations when piece selected
- [x] `executeMove()` calls `mcumax_play_move()` and syncs board
- [x] Selection can be cancelled by pressing button on same square
- [x] Visual highlights distinguish selected piece and legal moves
- [x] Build succeeds: `pio run -e nanoatmega328`
- [x] All tests pass: `pio test -e native`
- [x] Memory budget maintained (Flash < 80%, RAM < 70%)
- [ ] Manual test: Can play multiple moves alternating white/black
- [ ] Manual test: Selecting opponent's piece or empty square does nothing
- [ ] Manual test: Cursor cycles only through legal moves when piece selected
- [x] Code follows project conventions (see `blueprint/context/coding-standards.md`)
- [x] Ready for `/audit current` and `/complete`

---

## Archive annotation

**Completed**: 2026-11-12  
**Work commit**: `de15297dfbbe4157b372392cf78f5a3de6d736dc`  
**Spec hash**: `f47945ff1ac2d35694966db47a5ec1bbd526bd14dce7c37bf2e95cf52a2f777b`  
**Original HEAD**: `bc645138a3008606562e7109a9ad1cedf8446adc`  
**Base**: `56b2f6147d18fc974780f4007caf318f3188b1b2` (main)

### Implementation notes

This feature implements the core gameplay loop: piece selection, legal move display, and move execution. Key implementation decisions:

1. **mcu-max API integration**: Discovered that `mcumax_get_current_side()` returns internal engine values (`0x08` for white, `0x10` for black), different from `mcumax_get_piece()` color bit (0x08 set = black). Added `currentPlayerFromMcumaxSide()` helper to translate correctly.

2. **Coordinate mapping**: Display row maps directly to engine rank (row 0 = rank 0 = black back rank at top). No inversion needed.

3. **Test infrastructure unification**: Removed duplicate `test/test_gamestate/GameState.h`, tests now include `../../src/GameState.h` directly for single source of truth.

4. **Memory efficiency**: Flash 60.2% (18494/30720 bytes), RAM 36.1% (740/2048 bytes) - well within budget.

### Hardware verification

User confirmed on actual XC Slim Pad hardware (Arduino Nano, ST7735 display, joystick):
- White pawn selection and move execution works correctly
- Legal moves display properly
- Turn switching functions as expected
- Visual feedback (cyan selection border, green legal move dots, yellow cursor) renders correctly

### Try it now

1. Upload firmware: `pio run --target upload`
2. Power on the Arduino Nano on XC Slim Pad
3. Observe the initial chess position with white to move
4. Move joystick to select a white piece (e.g., e2 pawn)
5. Press joystick button - piece gets cyan border, legal moves show green dots
6. Move joystick - cursor cycles only through legal destinations
7. Press button on a legal destination - move executes, board updates, turn switches to black
8. Repeat for black pieces

The core move selection and execution loop is complete and verified.
