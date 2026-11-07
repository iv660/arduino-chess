# Arduino Chess

<!-- blueprint:source-hash 5867c86c47350330d1d0b7d610dc918e6639b146c89a7b968cf253d0f03f1b46 -->

A minimalist chess game for Arduino Nano with ST7735 TFT display. MVP chess program for human vs computer play with full chess rules, 900 Elo AI difficulty, joystick control, and random color selection. Built for XC Slim Pad hardware under strict memory (2KB RAM) and time constraints.

## Problem

Implementing a complete chess experience on minimal embedded hardware (Arduino Nano: 2KB SRAM, 30KB Flash, 16 MHz) within severe memory and processing constraints. Must deliver full chess rules, move generation, AI opponent, display rendering, and input handling while maintaining strict time and budget limits.

## Users

- DIY electronics enthusiasts building retro gaming projects
- Chess players interested in minimalist portable chess computers
- Arduino hobbyists exploring constrained game development
- Makers creating compact handheld gaming devices

## Data model

### GameState

Runtime state structure (no persistence):

```cpp
struct GameState {
  uint8_t board[8][8];        // Piece positions: empty=0, or piece type+color encoding
  uint8_t currentPlayer;      // 0=White, 1=Black
  uint8_t cursorRow;
  uint8_t cursorCol;
  uint8_t selectedRow;        // 255 when no piece selected
  uint8_t selectedCol;
  uint8_t legalMoves[MAX_MOVES][2];  // Destination squares for selected piece
  uint8_t legalMoveCount;
  GamePhase phase;            // PLAYER_MOVE | COMPUTER_THINKING | GAME_OVER
  GameResult result;          // IN_PROGRESS | WHITE_WINS | BLACK_WINS | DRAW
  uint8_t castlingRights;     // Bitfield: white K/Q, black k/q
  uint8_t enPassantTarget;    // Target square or 255 if none
};
```

All state resets on power cycle. No saved games, no move history beyond current legal move generation.

## Features

Build order:

1. **Chess library evaluation and integration** - Research and integrate MicroChess, mcu-max, htcw_chess, or Arduino Chess OLED; measure memory footprint; verify move generation and AI at 900 Elo
2. **Core game state and board representation** - Define GameState structure, 8×8 board array, piece encoding, turn tracking, game phase enum
3. **Display board rendering** - Draw 8×8 grid with alternating colors on ST7735, render square coordinates, display turn indicator text
4. **Piece bitmap icons** - Create or source 8×8/12×12 monochrome bitmaps for all 12 piece types, store in PROGMEM, render at correct board positions
5. **Joystick input and cursor navigation** - Poll analog joystick, map to board coordinates, move cursor with diagonal support, highlight selected square, handle edge boundaries
6. **Move selection with legal-move filtering** - Select piece with joystick button, query chess engine for legal moves, restrict cursor to legal destination squares, execute move on second button press
7. **Computer move integration** - Trigger AI move calculation after player move, display thinking indicator, execute computer move, update board and display
8. **Game flow and lifecycle** - Random color assignment at startup, detect checkmate/stalemate/draw, display game result, handle Start button for new game
9. **Memory optimization pass** - Profile RAM usage at runtime, optimize GameState size, reduce display buffer if needed, apply geometric piece fallback if bitmap memory critical
10. **Hardware testing and polish** - End-to-end testing on XC Slim Pad hardware, verify joystick responsiveness, tune AI thinking time, fix display flicker if present

## Stack

**Hardware:**
- Arduino Nano (ATmega328P, 16 MHz, 2KB SRAM, 30KB Flash)
- 1.8" ST7735 TFT display (128×160 pixels, SPI)
- Analog joystick with button (INPUT_PULLUP, no external resistors)
- Pause and Start buttons (INPUT_PULLUP)

**Build system:**
- PlatformIO (atmelavr platform, nanoatmega328 board)
- Arduino framework
- Serial 115200 baud for debug

**Libraries:**
- Adafruit GFX + Adafruit ST7735/ST7789 for display
- Chess engine library (to be selected from candidates: MicroChess, mcu-max, htcw_chess, Arduino Chess OLED)
- Unity test framework for native target (chess logic only)

**Testing:**
- Unity tests on native PlatformIO target for own chess logic only
- Third-party library logic not tested
- Display and input tested manually on hardware

## UI/UX

**Visual style:**
- Minimalist, functional retro aesthetic
- High contrast alternating light/dark squares on 8×8 board (~16×16 pixels per square)
- Bitmap piece icons (8×8 or 12×12 PROGMEM bitmaps preferred; fallback to geometric shapes if memory critical)
- Clean cursor highlighting (inverse colors or colored border)

**Interaction flow:**
1. Power on → random color assignment → board displayed (orientation matches player color)
2. Player move: joystick moves cursor (diagonal support, stops at edges) → button selects piece → cursor restricted to legal destination squares → button confirms move
3. Computer thinking: brief visual indicator (text or animation), target <3 seconds
4. Turn indicator updates ("White" / "Black" text at top/bottom)
5. Game over: result message ("White wins", "Black wins", "Draw") → wait for Start button → new game

**Responsiveness:**
- Immediate cursor response to joystick (main loop polling)
- Computer moves <3 seconds at 900 Elo
- Display updates <100ms for cursor movement
- No flicker

## Core gameplay

- Full chess rules: legal moves, check, checkmate, stalemate, castling, en passant, promotion to queen
- Human vs computer only
- Computer opponent at 900 Elo (single fixed level for MVP)
- Random color assignment at game start (no UI selection)

**Explicit exclusions:**
- No time control or clock
- No move takeback
- No threefold repetition or fifty-move rule (simplified draw detection)
- No game save/load
- No move history display or analysis visualization
- No multiple difficulty levels in MVP

## Deployment

**Upload:** Direct PlatformIO upload via USB (`pio run --target upload`)

**Distribution:** Source repository for users to build and flash. No pre-compiled binaries.

**Hardware assembly:** User assembles XC Slim Pad per docs/HARDWARE_SETUP.md wiring diagram.

## Usage model and constraints

**Memory constraints (critical):**
- 2KB SRAM total (stack + heap + globals)
- ~300-500 bytes reserved for stack and Arduino framework
- Chess engine library must fit within remaining ~1500 bytes
- Display: direct draw (no full framebuffer) or minimal tile buffer
- Early memory profiling required at each major integration step

**Performance constraints:**
- 16 MHz AVR processor (slow for chess calculation)
- Move generation and AI search must complete in reasonable time
- Display refresh fast enough to feel responsive (<100ms for cursor move)

**Development constraints:**
- Strict time budget: minimize custom chess logic, prefer proven library
- Minimal cost: reuse existing dependencies, no additional hardware
- Early library evaluation is critical path item

**Trust boundaries:**
- Single-user device, no network, no untrusted input
- No authentication, persistence, or data validation beyond chess rule enforcement
- Security: not applicable for offline single-player embedded device
- Scale: single device, no multiplayer, no cloud connectivity
