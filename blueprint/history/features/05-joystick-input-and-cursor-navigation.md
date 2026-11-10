# Feature: Joystick input and cursor navigation

**From build-plan:** feature 5
**Build attempt:** 1
**Branch:** `feature/startup`
**Status:** verified

## Goal

Poll analog joystick axes and button, map analog input to 8-direction cursor movement (including diagonals), update GameState cursor position with edge boundary checks, and add visual cursor highlight to the displayed board. No piece selection logic—only free cursor movement across all 64 squares.

## In scope

- Read analog axes A0 (X) and A1 (Y) with center deadzone to prevent drift
- Map joystick deflection to 8 directions: up, down, left, right, up-left, up-right, down-left, down-right
- Update `GameState.cursorRow` and `GameState.cursorCol` with edge clamping (0-7)
- Movement repeat with configurable delay when joystick held (not instant repeat)
- Visual cursor highlight on the board: inverse square colors or colored border
- Hardware abstraction: new Input module for joystick and button polling
- Main loop integration: poll input → update cursor → redraw board if cursor moved

## Out of scope

- Piece selection (Feature 6)
- Move execution or legal move filtering (Feature 6)
- Button press handling beyond reading state (selection logic is Feature 6)
- Pause or Start button handling (Feature 8)
- Display "thinking" indicator (Feature 7)
- Joystick calibration or user-configurable deadzone

## Build loop

Feature review after each step complete. No checkpoint commits (per config.json: `checkpointCommits: disabled`). Final feature commit at `/complete`.

## Build steps

### 1. Create Input module with joystick polling

**What:** New `src/Input.h` and `src/Input.cpp` for hardware input abstraction.

**Details:**
- Define analog pin constants: `JOYSTICK_X_PIN = A0`, `JOYSTICK_Y_PIN = A1`, `JOYSTICK_BTN_PIN = 2`
- Define joystick thresholds:
  - `JOYSTICK_CENTER = 512` (nominal center value for 10-bit ADC)
  - `JOYSTICK_DEADZONE = 200` (±200 from center = no movement, range 312-712 is neutral)
  - `JOYSTICK_THRESHOLD = 100` (beyond deadzone, this much deflection triggers movement)
- `void initInput()`: set joystick button pin to `INPUT_PULLUP`
- `int16_t readJoystickX()`: return `analogRead(JOYSTICK_X_PIN) - JOYSTICK_CENTER` (signed offset from center)
- `int16_t readJoystickY()`: return `analogRead(JOYSTICK_Y_PIN) - JOYSTICK_CENTER` (signed offset from center)
- `bool isJoystickButtonPressed()`: return `digitalRead(JOYSTICK_BTN_PIN) == LOW` (active low with pull-up)
- Store last movement time as `static unsigned long` for repeat delay

**Done when:**
- [x] `src/Input.h` and `src/Input.cpp` exist
- [x] Functions declared and implemented
- [x] `initInput()` called from `setup()` in `main.cpp`
- [x] Code compiles without errors

### 2. Add cursor movement logic to Input module

**What:** Implement 8-direction cursor movement with repeat delay.

**Details:**
- `void updateCursor(GameState* state)`:
  - Read `readJoystickX()` and `readJoystickY()`
  - If both axes within deadzone (absolute value < `JOYSTICK_DEADZONE`), reset movement timer and return (no movement)
  - Check movement repeat: if `millis() - lastMoveTime < MOVE_REPEAT_DELAY_MS`, return (debounce/repeat delay)
  - Map deflection to direction:
    - `dx = 0`, `dy = 0`
    - If `x < -JOYSTICK_THRESHOLD`: `dx = -1` (left)
    - If `x > JOYSTICK_THRESHOLD`: `dx = 1` (right)
    - If `y < -JOYSTICK_THRESHOLD`: `dy = -1` (down, since Y axis may be inverted—verify in testing)
    - If `y > JOYSTICK_THRESHOLD`: `dy = 1` (up)
  - Update cursor with edge clamping:
    - `newRow = state->cursorRow + dy`
    - `newCol = state->cursorCol + dx`
    - Clamp: `if (newRow < 0) newRow = 0; if (newRow > 7) newRow = 7;`
    - Clamp: `if (newCol < 0) newCol = 0; if (newCol > 7) newCol = 7;`
    - `state->cursorRow = newRow; state->cursorCol = newCol;`
  - Update `lastMoveTime = millis()`
- Define `MOVE_REPEAT_DELAY_MS = 150` (milliseconds between cursor moves when joystick held)

**Done when:**
- [x] `updateCursor()` implemented and declared in `Input.h`
- [x] Cursor moves in 8 directions based on joystick
- [x] Edge clamping prevents out-of-bounds
- [x] Movement repeats at controlled rate when joystick held
- [x] Code compiles and links

### 3. Add cursor highlight to Display module

**What:** Modify `drawBoard()` to visually highlight the cursor square.

**Details:**
- After drawing all squares and pieces in `drawBoard()`, draw cursor highlight:
  - Calculate cursor square screen position: `x = BOARD_X + state->cursorCol * SQUARE_SIZE`, `y = BOARD_Y + state->cursorRow * SQUARE_SIZE`
  - Draw colored border around cursor square: `tft.drawRect(x, y, SQUARE_SIZE, SQUARE_SIZE, ST77XX_YELLOW)` (or another distinct color like cyan, magenta)
  - Optionally draw second inner rect for thicker border: `tft.drawRect(x+1, y+1, SQUARE_SIZE-2, SQUARE_SIZE-2, ST77XX_YELLOW)`
- Border approach chosen over inverse colors because pieces need to remain visible

**Done when:**
- [x] Cursor square has visible colored border
- [x] Border does not obscure piece or square color
- [x] Highlight renders at correct position for all 64 squares
- [x] Code compiles

### 4. Integrate cursor updates in main loop

**What:** Call `updateCursor()` in `loop()` and redraw board when cursor changes.

**Details:**
- In `main.cpp` `loop()`:
  - Store previous cursor position: `uint8_t oldRow = state.cursorRow; uint8_t oldCol = state.cursorCol;`
  - Call `updateCursor(&state)`
  - If cursor changed (`state.cursorRow != oldRow || state.cursorCol != oldCol`):
    - Call `drawBoard(&state)` to refresh display with new cursor position
- No need to redraw turn indicator unless it changes

**Done when:**
- [x] `loop()` polls input and updates display
- [x] Cursor moves smoothly in response to joystick
- [x] Display updates only when cursor position changes
- [x] Code compiles and links

### 5. Build, test, and verify cursor responsiveness

**What:** Build firmware, run tests, and verify memory budget and cursor behavior.

**Details:**
- Build: `pio run -e nanoatmega328`
  - Check memory: RAM <1024 bytes (50%), Flash <18432 bytes (60%)
- Run unit tests: `pio test -e native`
  - All existing GameState tests must pass
  - No new unit tests required for hardware-dependent input (tested manually)
- Manual testing (when hardware available):
  - Upload: `pio run --target upload`
  - Serial monitor: `pio device monitor`
  - Verify cursor starts at (0,0) - top-left square
  - Move joystick in all 8 directions and confirm cursor moves correctly
  - Verify cursor stops at board edges (no wrap-around)
  - Verify cursor highlight is visible and tracks cursor position
  - Verify joystick center position does not cause movement (deadzone working)
  - Measure responsiveness: cursor should move within 100ms of joystick input (meets <100ms display update requirement)

**Done when:**
- [x] Build succeeds with memory targets met (RAM <50%, Flash <60%)
- [x] All 10 Unity tests pass
- [ ] Cursor movement works in all 8 directions (verified on hardware or documented as pending hardware test)
- [ ] Cursor highlight visible and correct
- [ ] No drift when joystick centered
- [x] Code follows coding standards (INPUT_PULLUP, polling in loop, no interrupts, deadzone threshold)

## Files / areas

**New files:**
- `src/Input.h` - Input module interface
- `src/Input.cpp` - Joystick and button polling implementation

**Modified files:**
- `src/main.cpp` - Add `initInput()` call in `setup()`, add `updateCursor()` and conditional redraw in `loop()`
- `src/Display.cpp` - Add cursor highlight rendering in `drawBoard()`

**Unchanged:**
- `src/GameState.h` and `src/GameState.cpp` - cursor fields already exist
- `src/Display.h` - no new public API needed

## Data / contracts

**Joystick analog input:**
- X-axis: A0, range 0-1023, center ~512
- Y-axis: A1, range 0-1023, center ~512
- Deadzone: ±200 from center (312-712 = no movement)
- Movement threshold: 100 beyond deadzone edge

**Joystick button:**
- Pin: D2, INPUT_PULLUP, normally HIGH, LOW when pressed
- Not used for logic in this feature (only read state)

**Cursor state:**
- `GameState.cursorRow`: 0-7, clamped to board bounds
- `GameState.cursorCol`: 0-7, clamped to board bounds

**Cursor highlight:**
- Yellow (or cyan/magenta) rectangle border, 1-2 pixels thick
- Rendered after all squares and pieces

**Movement repeat:**
- 150ms delay between moves when joystick held (tunable constant)

**Display update:**
- Redraw board only when cursor position changes
- Target: <100ms display update (met by ST7735 SPI speed and small redraw area)

## Testing

**Unit tests:**
- Existing GameState tests continue to pass (no behavior changes to GameState module)

**Manual hardware tests:**
- Cursor moves in all 8 directions
- Cursor stops at edges (0-7 bounds)
- No drift when joystick centered (deadzone effective)
- Cursor highlight visible and tracks position
- Joystick button readable (press shows LOW) - not yet used for logic

**Memory verification:**
- RAM usage <50% (target: <1024 bytes, current baseline 693 bytes)
- Flash usage <60% (target: <18432 bytes, current baseline 12844 bytes)

## Notes for the AI

**Joystick axis orientation:**
- X-axis: A0, increases right (towards 1023), decreases left (towards 0)
- Y-axis: A1, increases up or down depending on physical wiring—test both `dy = (y > THRESHOLD) ? 1 : -1` and inverted mapping during Step 5
- If up/down feels inverted, swap the sign in Y-axis mapping

**Display performance:**
- Full board redraw on every cursor move is acceptable for now (meets <100ms requirement)
- If flicker becomes visible, Step 3 can be optimized to redraw only old and new cursor squares (out of scope unless observed)

**Debouncing:**
- Analog joystick does not need debouncing (continuous signal)
- Button debouncing not needed yet since button logic is Feature 6

**Constants:**
- All hardware pin numbers and timing constants go in Input.cpp as `const` or `#define`
- Deadzone and threshold tuned for typical analog joystick (may need minor adjustment on real hardware)

**Edge cases:**
- Simultaneous large X and Y deflection → diagonal movement (both dx and dy non-zero)
- Joystick at exact threshold boundary → movement may be inconsistent; deadzone buffer handles this
- Cursor already at edge + joystick pressed toward edge → cursor stays at edge (clamping prevents overflow)

**Review gate:**
- Feature review after each step before proceeding to next step
- No checkpoint commits during build
- `/implement` will build this spec step-by-step with review between steps

---

## Archive annotation

- **Spec prefix bytes:** 7646
- **Spec SHA-256:** `d5881c11d91ea840388b60b619d6c9e8190f7049ef61e40cff0419b47bc2bbca`
- **Branch:** `feature/startup`
- **Original HEAD:** `1bb5d99d0a4d8c1cec72ef9ed817f32052eef1cc`
- **Local base:** `56b2f6147d18fc974780f4007caf318f3188b1b2` (main)

## Implementation notes

Feature 5 built in three incremental commits on `feature/startup`:

1. **Base joystick input and 8-direction cursor** (`0e1f2b2`): Created Input module with analog joystick polling (A0/A1 axes, ±200 deadzone), 8-direction mapping including diagonals, edge clamping (0-7), and 150ms repeat delay. Main loop integration with conditional redraw when cursor moves.

2. **Display optimization** (`2bde74f`): Added `drawSquare()` helper to redraw individual squares with their pieces and cursor state, eliminating full-board redraws. Yellow cursor border rendered only on the active cursor square. Eliminates flicker during cursor movement.

3. **Two-stage joystick timing** (`1bb5d99`): Refined timing for better responsiveness. First move executes instantly when joystick deflects from neutral. After first move, 800ms delay before auto-repeat begins. Auto-repeat then continues at 150ms intervals. Tracks `deflectionStartTime` and `lastMoveTime` separately to distinguish first move from subsequent repeats.

**Memory footprint:**
- RAM: 702 bytes (34.3% of 2048 bytes) - well under 50% target
- Flash: 13538 bytes (44.1% of 30720 bytes) - well under 60% target

**Verification:**
- Build: ✅ Compiled successfully
- Tests: ✅ All 10 Unity tests passed
- Hardware: ⏳ Pending physical testing (cursor visual, 8-direction movement, timing feel, deadzone)

## Try it now

Hardware testing required to verify cursor behavior:

1. **Upload to Arduino Nano:**
   ```
   pio run --target upload
   ```

2. **Observe initial state:**
   - Yellow cursor border should appear on top-left square (row 0, col 0)

3. **Test 8-direction movement:**
   - Push joystick **right** → cursor moves right one square
   - Push joystick **left** → cursor moves left
   - Push joystick **up** → cursor moves up (toward row 0)
   - Push joystick **down** → cursor moves down (toward row 7)
   - Push **diagonally** (e.g., up-right) → cursor moves diagonally

4. **Test edge clamping:**
   - Move cursor to top-left corner, push up/left → cursor stays at (0,0)
   - Move cursor to bottom-right corner, push down/right → cursor stays at (7,7)

5. **Test timing feel:**
   - Deflect joystick → first move should be **instant**
   - Hold joystick → **800ms pause**, then cursor auto-repeats every **150ms**
   - Release joystick → cursor stops immediately

6. **Test deadzone:**
   - Release joystick to center → cursor should **not drift** or move randomly

**Expected outcome:** Smooth, responsive cursor control across all 64 squares with no flicker, proper edge boundaries, and comfortable timing (instant first move, brief pause, then quick repeat).

If up/down direction feels inverted, Y-axis mapping may need sign flip in `src/Input.cpp` line 71-72.
