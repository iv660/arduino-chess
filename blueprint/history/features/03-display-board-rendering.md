# Feature: Display board rendering

**From build-plan:** feature 3  
**Build attempt:** 1  
**Branch:** feature/startup  
**Status:** verified

## Goal

Draw the 8×8 chess board with alternating light/dark square colors on the ST7735 TFT display, render square coordinate labels (files a–h, ranks 1–8), and display the current turn indicator text ("White" or "Black"). This establishes the core visual foundation for the game before piece icons and input handling are added in later features.

## In scope

- Initialize ST7735 display (128×160 pixels, SPI, pins D10/CS, D8/DC, D11/MOSI, D13/SCK)
- Draw 8×8 board grid with alternating light/dark squares (~16×16 pixels per square on 128×160 display)
- Render file labels (a–h) along bottom edge
- Render rank labels (1–8) along side edge
- Display turn indicator text at screen top: "White" or "Black" based on `GameState.currentPlayer`
- Create `Display.h` and `Display.cpp` module to encapsulate display initialization and rendering logic
- Update `main.cpp` to initialize display and render board in `setup()`
- Verify display output manually on XC Slim Pad hardware (screenshot or photo evidence)

## Out of scope

- Piece bitmaps or geometric shapes (Feature 4)
- Cursor highlighting or user input (Feature 5)
- Move selection or legal-move indication (Feature 6)
- Animated thinking indicators or move animations (Feature 7)
- Game-over result messages (Feature 8)
- Performance optimization or flicker reduction beyond basic buffering (Feature 9)

## Build loop

Feature-level review before final commit per `workflow.stepReview: "feature"`. Checkpoint commits disabled per `workflow.checkpointCommits: "disabled"`.

## Build steps

- [x] **Step 1: Create Display module skeleton**
  - Create `src/Display.h` with function declarations: `initDisplay()`, `drawBoard(const GameState*)`, `drawTurnIndicator(const GameState*)`
  - Create `src/Display.cpp` with placeholder implementations (empty body, serial debug output)
  - Include Adafruit GFX and ST7735 headers, declare global `Adafruit_ST7735` TFT object with pin configuration (CS=10, DC=8, RST=-1)
  - **Done when:** Module compiles successfully, no undefined references

- [x] **Step 2: Implement ST7735 initialization**
  - In `Display.cpp`, implement `initDisplay()`: initialize TFT object with `INITR_BLACKTAB` or `INITR_GREENTAB` (verify with hardware docs), set rotation for portrait/landscape per XC Slim Pad orientation, fill screen black, print "Display OK" to serial
  - Call `initDisplay()` from `main.cpp setup()` after serial init but before GameState init
  - **Done when:** `pio run -e nanoatmega328` builds without errors, serial output shows "Display OK" on hardware test

- [x] **Step 3: Draw 8×8 board grid with alternating colors**
  - In `Display.cpp`, implement `drawBoard(const GameState*)`: calculate square size (width = 128/8 = 16px, height adjustable to fit 8 rows in ~128px vertical space), draw 64 filled rectangles with alternating light (e.g., ST77XX_WHITE or RGB565 light gray) and dark (e.g., ST77XX_BLACK or RGB565 dark gray) colors
  - Use standard chess board coloring: bottom-left square (a1) is dark
  - Ignore `GameState` board array contents for now (Feature 4 will add pieces)
  - **Done when:** Hardware test shows 8×8 checkerboard pattern with clear alternating colors, no gaps or overlaps

- [x] **Step 4: Render coordinate labels**
  - Add file labels (a–h) along bottom edge or below board using `tft.setCursor()` and `tft.print()`
  - Add rank labels (1–8) along left or right edge
  - Use small font size (default GFX font or `setTextSize(1)`), light color (e.g., white) on dark background or vice versa for visibility
  - **Done when:** Hardware test shows all 8 file and 8 rank labels, legible and correctly positioned

- [x] **Step 5: Display turn indicator**
  - In `Display.cpp`, implement `drawTurnIndicator(const GameState*)`: read `state->currentPlayer` (0=White, 1=Black), draw "White" or "Black" text at screen top (y=0 or small offset) using `tft.setCursor()`, `tft.setTextColor()`, `tft.setTextSize(1 or 2)`
  - Clear previous text or use a fixed bounding box to avoid overwrite artifacts
  - **Done when:** Hardware test shows correct turn indicator text updating when `GameState.currentPlayer` is changed manually in code

- [x] **Step 6: Integrate display rendering into main.cpp**
  - In `main.cpp setup()`, call `initDisplay()`, then `drawBoard(&gameState)`, then `drawTurnIndicator(&gameState)`
  - Add serial output confirming each stage ("Board drawn", "Turn indicator shown")
  - Keep `loop()` empty for now (no refresh needed until cursor/input in Feature 5)
  - **Done when:** Full build completes, hardware shows complete board + labels + turn indicator on startup

- [x] **Step 7: Verify and document**
  - Run `pio run -e nanoatmega328 && pio test -e native` (tests pass, build succeeds)
  - Check memory usage: RAM should remain well under 50% (display state is minimal, Adafruit libs use some global buffers)
  - Upload to XC Slim Pad hardware via `pio run --target upload`, photograph or screenshot display output showing board, labels, and turn indicator
  - Update README or HARDWARE_SETUP.md if display pin configuration differs from documented values
  - **Done when:** Verify command passes, memory usage acceptable, hardware evidence captured

## Files / areas

- `src/Display.h` (new): Display module public API
- `src/Display.cpp` (new): Display initialization and rendering functions
- `src/main.cpp` (edit): Add display init and render calls in `setup()`
- `docs/HARDWARE_SETUP.md` (reference): Display pin configuration (already documented, verify against implementation)

## Data / contracts

**Display configuration:**
- ST7735 controller, 128×160 pixels
- SPI pins: CS=D10, DC=D8, MOSI=D11, SCK=D13, RST=Arduino RST or unused
- Rotation: TBD based on XC Slim Pad physical orientation (portrait or landscape)
- Initialization mode: `INITR_BLACKTAB` or `INITR_GREENTAB` (verify with hardware)

**Board rendering:**
- 8×8 grid, square size ~16×16 pixels (exact size adjusted to fit display)
- Light square color: white or light gray (RGB565)
- Dark square color: black or dark gray (RGB565)
- Bottom-left square (a1) is dark per standard chess board
- Coordinate labels: files a–h (bottom), ranks 1–8 (left or right), small font, high contrast

**Turn indicator:**
- Text: "White" when `GameState.currentPlayer == 0`, "Black" when `== 1`
- Position: top of screen, centered or left-aligned
- Font size: 1 or 2, color: white or high-contrast
- No animation or blinking

## Testing

**Manual hardware verification:**
- Visual inspection on XC Slim Pad: board grid, labels, turn indicator all present and legible
- Confirm board coloring matches standard chess (a1 dark)
- Verify turn indicator updates when `currentPlayer` is toggled in test code

**Automated tests:**
- No new Unity tests (display rendering is not unit-testable on native target)
- Existing GameState tests continue to pass (`pio test -e native`)
- Build verification: `pio run -e nanoatmega328 && pio test -e native` completes without error

## Notes for the AI

- Use Adafruit GFX library for primitives (`fillRect`, `setCursor`, `print`), ST7735 library for hardware control
- Minimize flicker by drawing the full board once in `setup()`; no refresh in `loop()` until later features add dynamic elements
- Do not hardcode cursor position or piece rendering—those are separate features
- If display init fails on hardware, add diagnostic serial output to confirm SPI pin wiring
- Memory budget is tight (2KB RAM): Adafruit libs use some global buffers, monitor RAM usage after each step
- Coordinate label placement is flexible: choose layout that fits display dimensions and remains legible
- If text overlaps board, adjust board vertical offset or label placement
- Use `F()` macro for string literals to keep them in Flash instead of RAM

## Open questions

None. Display hardware is documented, color scheme and layout are defined in project-overview.md, and rendering is purely visual with no persisted data or external API dependencies.
