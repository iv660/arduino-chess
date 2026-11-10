# Current Feature

**Status: verified**

**Feature 4: Piece bitmap icons**

Create or source monochrome bitmaps for all 12 piece types (6 pieces × 2 colors), store in PROGMEM Flash, and render at correct board positions.

## Branch

`feature/startup` _(no new feature branch per project-specific branching rule)_

## Goals

- Define 1-bit monochrome bitmaps for King, Queen, Rook, Bishop, Knight, Pawn (white and black)
- Store bitmaps in PROGMEM to preserve RAM
- Integrate with existing `drawBoard()` to render pieces at correct positions
- Maintain memory targets: RAM <50%, Flash <60%
- Verify pieces render correctly at all 64 board positions

## Context

- `GameState.board[8][8]` stores piece encoding (MCUMAX_EMPTY=0, MCUMAX_PAWN_UPSTREAM=1, MCUMAX_KNIGHT=3, MCUMAX_BISHOP=4, MCUMAX_ROOK=5, MCUMAX_QUEEN=6, MCUMAX_KING=7, MCUMAX_BLACK=0x08 for black pieces)
- Display uses ST7735 TFT at 128×160, landscape rotation, board at (0,0)
- SQUARE_SIZE=16 pixels per square
- Adafruit GFX library provides `drawBitmap(x, y, bitmap, w, h, color)`
- Current memory: RAM 693/2048 (33.8%), Flash 12092/30720 (39.4%)
- No piece rendering exists yet; `drawBoard()` only draws squares and labels

## Non-Goals

- Piece movement animation (future)
- Anti-aliasing or grayscale (1-bit only)
- Piece selection highlight (Feature 5)
- Multiple piece styles or themes

## Steps

### 1. Choose bitmap size and create piece bitmaps (5–15 min)

- **Decision point:** 8×8 (96 bytes total) or 12×12 (216 bytes total)
  - 8×8: fits easily in squares, minimal Flash cost, may lack legibility
  - 12×12: better detail, centered in 16×16 squares with 2px margin, higher Flash cost
  - **Recommendation:** Start with 12×12; if Flash exceeds 60%, fall back to 8×8
- Define 6 piece bitmaps (white: King, Queen, Rook, Bishop, Knight, Pawn)
- Use standard chess iconography: King with cross, Queen with crown, Rook as castle, Bishop as mitre, Knight as horse profile, Pawn as simple circle/oval
- Store as `const uint8_t PROGMEM` arrays in `Display.cpp` or new `Bitmaps.h`
- Black pieces use same bitmaps with inverted color argument to `drawBitmap()`
- **Validation:** Bitmap arrays compile, sizes match 12×12 or 8×8 format

### 2. Add piece rendering to Display module (10–15 min)

- Create `void drawPiece(uint8_t piece, uint8_t row, uint8_t col)` helper in `Display.cpp`
  - Extract piece type: `uint8_t type = piece & 0x07;`
  - Extract color: `bool isBlack = piece & MCUMAX_BLACK;`
  - Calculate screen position: `x = BOARD_X + col * SQUARE_SIZE`, `y = BOARD_Y + row * SQUARE_SIZE`
  - Center bitmap in square if 12×12: add 2px offset
  - Select bitmap from lookup table/switch based on `type`
  - Call `tft.drawBitmap(x, y, bitmap, w, h, isBlack ? ST77XX_BLACK : ST77XX_WHITE)` on white square, inverted on black square for contrast
- Add piece-drawing loop to `drawBoard()` after drawing squares:
  ```cpp
  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t piece = state->board[row][col];
      if (piece != MCUMAX_EMPTY) {
        drawPiece(piece, row, col);
      }
    }
  }
  ```
- Declare `drawPiece()` in `Display.h`
- **Validation:** Code compiles without errors

### 3. Verify rendering and memory budget (5–10 min)

- Build: `pio run -e nanoatmega328`
- Check Flash and RAM usage in build output
  - **Pass:** RAM <1024 bytes (50%), Flash <18432 bytes (60%)
  - **Fail:** If Flash >60%, reduce bitmap size from 12×12 to 8×8 and rebuild
- Run tests: `pio test -e native`
  - All existing tests must pass (no behavior changes to GameState or engine)
- Upload to hardware: `pio run --target upload`
- Serial monitor: `pio device monitor` (check for startup messages, no crashes)
- Visual verification (if hardware available):
  - Pieces appear at correct positions matching initial FEN
  - White pieces distinguishable from black pieces
  - All 6 piece types recognizable
  - Pieces centered in squares
  - No rendering artifacts or overlap
- **Validation:** Memory targets met, tests pass, pieces render correctly

## Done When

- [x] 12 piece bitmaps (6 types × 2 colors as color inversion) defined in PROGMEM
- [x] `drawPiece()` helper added to Display module
- [x] `drawBoard()` renders all pieces from `GameState.board`
- [x] Build succeeds with RAM <50%, Flash <60%
- [x] All Unity tests pass
- [ ] Visual verification: pieces render at correct positions with correct icons
- [x] Code follows coding standards (PROGMEM for const data, no magic numbers)

## Risks & Mitigations

- **Risk:** 12×12 bitmaps exceed Flash budget
  - **Mitigation:** Fallback to 8×8 bitmaps (halves bitmap Flash cost)
- **Risk:** Pieces not legible at small size
  - **Mitigation:** Simplify bitmap designs, test on hardware
- **Risk:** Poor contrast on alternating square colors
  - **Mitigation:** Use inverted colors based on square background (white pieces on dark squares, black pieces on light squares, or add small border)

## Dependencies

- Feature 3 (Display board rendering) complete ✓
- `GameState.board` with piece encoding ✓
- Adafruit GFX library ✓

## Related Files

- `src/Display.cpp` (add `drawPiece()`, modify `drawBoard()`)
- `src/Display.h` (declare `drawPiece()`)
- `src/GameState.h` (piece encoding constants)
- Optional: `src/Bitmaps.h` (bitmap arrays if separated from Display.cpp)
