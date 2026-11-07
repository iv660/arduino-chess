# Coding Standards

> Your conventions. This file reflects the actual stack and constraints of the
> Arduino Chess embedded project.

## Platform

**Arduino Nano (ATmega328P)**
- 32 KB Flash (2 KB bootloader)
- 2 KB SRAM
- 1 KB EEPROM
- 16 MHz clock
- Tight memory and performance constraints

## Language

**C++11** (Arduino framework subset)
- Avoid dynamic allocation where possible (limited heap)
- Use stack allocation and static buffers
- Prefer compile-time constants (`const`, `constexpr`, `#define`)
- Use `F()` macro for string literals to keep them in flash memory
- Be mindful of SRAM usage: every global costs memory

## Build System

**PlatformIO**
- Framework: Arduino
- Board: `nanoatmega328`
- Platform: `atmelavr`
- Build: `pio run`
- Upload: `pio run --target upload`
- Monitor: `pio device monitor`

## File Organization

- Main sketch: `src/main.cpp`
- Headers: `src/*.h` or `include/*.h`
- Implementation: `src/*.cpp`
- Hardware abstraction modules: organized by function (display, input, chess logic)

## Naming

- Files: lowercase with underscores (`chess_engine.cpp`)
- Class files: PascalCase matching class name (`ChessBoard.cpp`, `ChessBoard.h`)
- Functions: camelCase (`makeMove()`)
- Classes: PascalCase (`ChessBoard`)
- Constants: SCREAMING_SNAKE_CASE or `k` prefix (`MAX_DEPTH`, `kBoardSize`)
- Pin definitions: ALL_CAPS with descriptive names (`TFT_CS`, `JOY_X`)
- Local variables: camelCase
- Member variables: camelCase or `m_` prefix when clarity helps

## Hardware Abstraction

- Use `#define` for all pin assignments at the top of files
- Group related pins (TFT, joystick, buttons) with comments
- Wrap hardware access in clean functions or classes when complexity warrants it
- Initialize all hardware in `setup()`, never assume pin states

## Memory Management

- **No `new`/`delete` in the main loop** - dynamic allocation fragments limited SRAM
- Use fixed-size arrays and preallocate buffers
- Check Flash and SRAM usage after every build (PlatformIO reports it)
- String literals go in PROGMEM via `F()` macro: `Serial.println(F("text"))`
- Keep stack usage predictable: avoid deep recursion

## Performance

- Minimize allocations and copying in hot paths
- Batch display updates to reduce SPI transactions
- Use integer math over floating point when possible (no FPU)
- Profile with `millis()` when optimizing timing-sensitive code

## Display

- Use Adafruit GFX for primitives, ST7735 library for hardware
- Buffer small graphics operations to reduce flicker
- Clear only changed regions when practical
- Test visual output on real hardware (emulation is unreliable for TFT)

## Input Handling

- Debounce buttons in software (no hardware debounce)
- Poll joystick analog axes, center value ~512
- Use `INPUT_PULLUP` for all buttons (no external pull-ups)
- Handle button state in main loop, not interrupts (simplicity over speed)

## Error Handling

- No exceptions (not supported on AVR)
- Return error codes or use bool return values
- Validate inputs and fail visibly (serial output, error screen)
- Use `Serial` for debugging; baud rate 115200

## Testing

The blueprint installs no test runner; testing is opt-in at the project level.
For embedded development, practical verification is:

- **Unit logic tests:** Use Unity test framework for native target. Test pure
  chess logic (move generation, validation, game state) that can run on the
  host without hardware. Configure native build target in PlatformIO for
  host-side test execution.
- **Hardware integration:** Verify on real Arduino Nano. Serial output,
  LED behavior, display rendering, and input response are best validated
  manually or with recorded observations.
- **Build verification:** Every change must compile and fit in Flash/SRAM
  limits. PlatformIO reports usage; watch for regressions.

**The opt-in switch is one signal: a `test` command in the Commands section of
`AGENTS.md`.** Declare one and tests become a gate for pure logic steps;
leave it out and the loop verifies with build + manual hardware evidence.

- **What to test:** pure chess logic (move generation, validation, game state
  transitions, position evaluation). These have assertable inputs/outputs and
  real edge cases (en passant, castling, checkmate detection).
- **What not to test:** display rendering, button debouncing, hardware I/O.
  Verify those with real hardware and screenshots.
- **The gate (when a runner is configured):** a build step that adds testable
  logic must ship a passing test in the same diff. Hardware-only steps are
  exempt and ride on build + manual evidence.

## Code Quality

- **Clean Code principles:** Follow Clean Code practices when they don't harm
  performance or memory usage. Prefer readable, maintainable code.
- **No magic numbers:** Replace all magic numbers with named constants
  (`#define`, `const`, or `constexpr`). Every numeric literal should have
  a descriptive name explaining its purpose.
- No commented-out code
- No unused variables or includes
- Keep functions focused: under 50 lines when possible
- Extract complex logic into named helper functions

## Comments

Write code that explains itself; comment only what the code cannot say.
Follow Clean Code: self-documenting names and clear structure beat comments.

- Comment the **why**, not the **what**. Delete any comment that restates the code.
- No banner blocks or step-by-step narration of obvious code.
- A comment earns its place only when it captures: a hardware constraint,
  a non-obvious decision, a timing/memory tradeoff, or why a specific
  implementation was chosen over alternatives.
- **No magic numbers in code.** Use named constants instead; the name documents
  the value's meaning better than any comment could.
- Prefer self-documenting names and small functions over explanatory comments.
- When in doubt, leave the comment out.

## Writing

- No em dashes (U+2014) in generated content: docs, comments, commit messages,
  READMEs, specs. They read as AI-generated.
- Use a hyphen for `term - description` separators; rephrase prose with commas,
  parentheses, or a colon. Avoid en dashes and the ellipsis character too.

## Libraries

Current dependencies (see `platformio.ini`):
- `Adafruit GFX Library` - graphics primitives
- `Adafruit ST7735 and ST7789 Library` - TFT driver

Add new libraries only when necessary. Prefer lightweight implementations
that fit in Flash/SRAM constraints. Check library memory footprint before
committing.

## Chess Implementation Constraints

From project requirements:
- Full chess rules (castling, en passant, promotion, checkmate/stalemate)
- Three difficulty levels: 700/900/1100 Elo (adjustable search depth or evaluation)
- No move takeback
- No time control
- No move history display
- No position analysis visualization
- Minimal UI: board, piece selection cursor, game state

Reference implementation available in `reference/3rd-party/bk_chess/` (PDP-11
disassembly) for compact chess algorithm ideas, not for direct porting.

## Budget Constraints

- **Time:** Minimal development time. Reuse proven algorithms and libraries.
- **Memory:** Must fit in 30 KB Flash + 2 KB SRAM. Profile every feature.
- **Simplicity:** Favor straightforward implementations over clever optimizations
  unless memory or performance requires it.
