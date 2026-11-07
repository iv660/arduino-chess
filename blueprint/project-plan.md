# Project Plan

This document defines the high-level vision, scope, and constraints for the
project. It is the source of truth for the system you are building.

## 1. Problem - What problem are we solving?

Creating a fully functional chess game on minimal embedded hardware (Arduino Nano with 2KB RAM and 30KB Flash) that provides a complete chess experience with AI opponent on a portable handheld device. The challenge is implementing full chess rules, move generation, AI engine, display rendering, and input handling within severe memory and processing constraints while maintaining strict time and budget limits.

## 2. Users - Who is this for?

- DIY electronics enthusiasts building retro gaming projects
- Chess players interested in minimalist, portable chess computers
- Arduino hobbyists exploring game development on constrained hardware
- Makers creating compact handheld gaming devices

## 3. Features - What does the MVP need?

**Core gameplay:**
- Full chess rules (legal moves, check, checkmate, stalemate, castling, en passant, promotion)
- Human vs computer play only
- Computer opponent at 900 Elo difficulty (single fixed level for MVP)
- Random color assignment at game start (no UI selection)

**Display:**
- Full 8×8 chessboard on 128×160 ST7735 TFT (~16×16 pixels per square)
- Bitmap piece icons (fallback to geometric shapes if memory critical)
- Board orientation matches player color (white on bottom when player is white)
- Text indicator showing current turn ("White" / "Black")
- Cursor highlighting for square selection

**Input:**
- Analog joystick navigation with diagonal support (fallback to 4-way if needed)
- Cursor stops at board edges (no wrap-around)
- When piece selected: cursor restricted to legal destination squares only
- Joystick button confirms piece/move selection
- Start button initiates new game after game over

**Explicit exclusions:**
- No time control or clock
- No move takeback
- No threefold repetition or fifty-move rule
- No game save/load
- No move history display or analysis visualization
- No multiple difficulty levels in MVP

## 4. Data - What are we storing?

**Runtime state only (no persistence):**
- `GameState` structure containing:
  - 8×8 board representation with piece positions
  - Current player turn
  - Selected piece and cursor position
  - List of legal moves for selected piece
  - Game phase (menu, player_move, computer_thinking, game_over)
  - Win/draw/loss result
  - Castling rights per player
  - En passant target square

**No persistent storage:** All state resets on power cycle. No saved games, no move history beyond current legal move generation.

## 5. Tech - What stack are we using?

**Hardware:**
- Arduino Nano (ATmega328P, 16 MHz, 2KB SRAM, 30KB Flash)
- 1.8" ST7735 TFT display (128×160 pixels, SPI)
- Analog joystick with button (INPUT_PULLUP, no external resistors)
- Pause and Start buttons (INPUT_PULLUP)

**Build system:**
- PlatformIO (atmelavr platform, nanoatmega328 board)
- Arduino framework
- Serial 115200 baud for debug output

**Libraries:**
- Adafruit GFX + Adafruit ST7735/ST7789 for display
- Existing Arduino chess engine library (to be evaluated and selected):
  - MicroChess (full engine <2KB RAM)
  - mcu-max (compact engine)
  - htcw_chess (rules only, no AI – fallback)
  - Arduino Chess with OLED (complete reference implementation)
- Unity test framework for native PlatformIO target (chess logic only)

**Testing:**
- Unity tests on native PlatformIO target for own chess logic only
- Third-party library logic not tested
- Display and input tested manually on hardware

## 6. Monetize - How will this make money?

Not applicable. Open-source hobby project.

## 7. UI/UX - How should this look and feel?

**Visual style:**
- Minimalist, functional retro aesthetic
- High contrast alternating light/dark squares
- Clear piece differentiation through bitmap icons (8×8 or 12×12 PROGMEM bitmaps preferred)
- Clean cursor highlighting (inverse colors or colored border)

**Interaction flow:**
1. Power on → random color assignment → board displayed
2. Player move: joystick moves cursor → select piece → cursor jumps only to legal squares → select destination → move executes
3. Computer thinking: brief visual indicator (text or animation)
4. Turn indicator updates after each move
5. Game over: display result message ("White wins", "Black wins", "Draw") → wait for Start button → new game

**Responsiveness:**
- Immediate cursor response to joystick (polling in main loop)
- Computer moves complete within reasonable time (target <3 seconds at 900 Elo)
- Smooth display updates without flicker

## 8. Deployment - Where and how will this ship?

**Upload:** Direct PlatformIO upload via USB to Arduino Nano (`pio run --target upload`)

**Distribution:** Source code repository for users to build and flash themselves. No pre-compiled binaries required for MVP.

**Hardware assembly:** User assembles XC Slim Pad hardware per docs/HARDWARE_SETUP.md wiring diagram.

## 9. Usage model and constraints (optional)

**Memory constraints (critical):**
- 2KB SRAM total (stack + heap + globals)
- ~300-500 bytes reserved for stack and Arduino framework
- Chess engine library must fit within remaining ~1500 bytes
- Display buffer strategies: direct draw (no full framebuffer) or minimal tile buffer
- Early memory profiling required at each major integration step

**Performance constraints:**
- 16 MHz AVR processor (relatively slow for chess calculation)
- Move generation and AI search must complete in reasonable time
- Display refresh should be fast enough to feel responsive (<100ms for cursor move)

**Development constraints:**
- Strict time budget: minimize custom chess logic, prefer proven library
- Minimal cost: reuse existing dependencies, no additional hardware
- Early library evaluation is critical path item

**Trust boundaries:**
- Single-user device, no network, no untrusted input
- No authentication, persistence, or data validation beyond chess rule enforcement

**Security:** Not applicable for offline single-player embedded device.

**Scale:** Single device, no multiplayer, no cloud connectivity.
