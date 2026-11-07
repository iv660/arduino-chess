# Build Plan

List the features that make up your project, high level and in rough build order.
Keep each item to one line; the details come later in `/feature`.

Plain bullets are fine. When both planning docs are ready, run `/overview`.
It adds tracking numbers and checkboxes to your feature list before generating
the project overview.

Run `/feature` to spec the next unchecked item, or `/feature 2` to pick one.
Keep completed items checked and append new features as the project grows.
Do not renumber completed features; their archived specs refer to those IDs.

Scaffolding the app and prototyping its look are pre-build steps, not features.
Start with your first real slice of functionality.

## Your features

- [x] 1. **Chess library evaluation and integration** - Research and integrate MicroChess, mcu-max, htcw_chess, or Arduino Chess OLED; measure memory footprint; verify move generation and AI at 900 Elo
- [ ] 2. **Core game state and board representation** - Define GameState structure, 8×8 board array, piece encoding, turn tracking, game phase enum
- [ ] 3. **Display board rendering** - Draw 8×8 grid with alternating colors on ST7735, render square coordinates, display turn indicator text
- [ ] 4. **Piece bitmap icons** - Create or source 8×8/12×12 monochrome bitmaps for all 12 piece types, store in PROGMEM, render at correct board positions
- [ ] 5. **Joystick input and cursor navigation** - Poll analog joystick, map to board coordinates, move cursor with diagonal support, highlight selected square, handle edge boundaries
- [ ] 6. **Move selection with legal-move filtering** - Select piece with joystick button, query chess engine for legal moves, restrict cursor to legal destination squares, execute move on second button press
- [ ] 7. **Computer move integration** - Trigger AI move calculation after player move, display thinking indicator, execute computer move, update board and display
- [ ] 8. **Game flow and lifecycle** - Random color assignment at startup, detect checkmate/stalemate/draw, display game result, handle Start button for new game
- [ ] 9. **Memory optimization pass** - Profile RAM usage at runtime, optimize GameState size, reduce display buffer if needed, apply geometric piece fallback if bitmap memory critical
- [ ] 10. **Hardware testing and polish** - End-to-end testing on XC Slim Pad hardware, verify joystick responsiveness, tune AI thinking time, fix display flicker if present
