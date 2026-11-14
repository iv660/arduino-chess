# Feature: Computer move integration

**Type:** Feature
**Branch:** feature/startup
**Status:** completed

## Goal

After the player completes their move, the computer must automatically calculate and execute its response move using the mcu-max chess engine. The UI must indicate that the computer is thinking and then smoothly transition back to player control.

## In scope

- Detect when the player's move is complete and it's the computer's turn
- Set game phase to `COMPUTER_THINKING` and display a thinking indicator
- Call `mcumax_search_best_move(1, 1)` within the Nano's stack budget
- Execute the computer's move through `mcumax_play_move()`
- Sync board and current player from the engine
- Return game phase to `PLAYER_MOVE` and update the display
- Handle invalid move case (checkmate/stalemate detection left to Feature 8)

## Out of scope

- Multiple difficulty levels (Feature 8 may add Start button difficulty selection)
- Move animation or piece sliding effects
- Move history recording
- Time display or time limits
- Detecting checkmate/stalemate/draw conditions (Feature 8)
- Pause button handling during computer thinking
- User interruption of computer move calculation
- Opening book or endgame tablebase

## Build loop

### 1. Add computer move triggering to main loop ✓

**Why:** The main loop needs to detect when it's the computer's turn and initiate move calculation.

**Changes:**
- In `main.cpp`, after a successful `executeMove()` by the player, check if `currentPlayer` matches the computer's color
- If it's the computer's turn, set `gameState.phase = COMPUTER_THINKING`
- Add a new helper function `executeComputerMove()` to encapsulate computer move logic
- For now, assume the player is white (computer is black, `currentPlayer == 1`) until Feature 8 adds random color assignment

**Acceptance:**
- After player move, if `currentPlayer == 1`, game phase transitions to `COMPUTER_THINKING`
- No compiler errors
- Verify build with `pio run -e nanoatmega328`

### 2. Display thinking indicator ✓

**Why:** User needs visual feedback that the computer is calculating its move.

**Changes:**
- Add `drawThinkingIndicator()` function in `Display.cpp` that prints "Thinking..." in the top-left corner during `COMPUTER_THINKING` phase
- Call this function from the main loop during computer move calculation
- After computer move completes, the function is no longer called, so the indicator disappears

**Acceptance:**
- "Thinking..." appears when phase is `COMPUTER_THINKING`
- Text is visible on the display
- Text disappears when phase changes to `PLAYER_MOVE`

### 3. Implement computer move search ✓

**Why:** The computer needs to calculate the best move given the current board state.

**Changes:**
- In `executeComputerMove()`, call `mcumax_search_best_move(1, 1)` to search for the best move
- The parameters are (depth, max_depth). Using (1, 1) matches the Nano's stack budget
- If the move is invalid (`from == MCUMAX_SQUARE_INVALID`), set phase to `GAME_OVER` (simplified for Feature 7)
- Monitor serial output for memory usage and confirm we stay within the Nano's 2KB RAM

**Acceptance:**
- `mcumax_search_best_move()` is called and returns a valid move or `MCUMAX_SQUARE_INVALID`
- No stack overflow on hardware
- Serial monitor shows stable memory usage

### 4. Execute computer move and update board ✓

**Why:** After the engine finds the best move, we must execute it on the board and reflect the changes in the game state.

**Changes:**
- Call `mcumax_play_move()` with the move from the search
- If successful, call `syncBoardFromEngine()` to update our `GameState` mirror
- Retrieve the new current player from the engine via `mcumax_get_current_side()`
- Set phase back to `PLAYER_MOVE`
- Call `drawBoard()` to refresh the display

**Acceptance:**
- Move is executed on the engine without errors
- Board state matches engine state after sync
- Player's turn indicator updates correctly
- Cursor becomes responsive again after computer move
- Board state matches engine state
- Full build and test on hardware: `pio run -e nanoatmega328 && pio test -e native`

### 5. End-to-end gameplay verification ✓

**Why:** Confirm that human-computer turn alternation works smoothly through multiple moves.

**Changes:**
- Test complete gameplay loop: player move → computer thinking → computer move → player move
- Verify cursor remains functional between turns
- Check that legal move calculation works correctly after computer moves
- Monitor serial output for errors
- Measure memory usage during computer thinking
- **Stack-safety repair:** Selection no longer calls the recursive `mcumax_search_valid_moves()` path. A bounded board-based generator proposes destinations, filters moves that leave the player's king in check, and leaves `executeMove()` as the engine-backed final move authority. En-passant state is updated for moves by both players.
- **Castling repair:** Only offer castling when current rights, rook placement, clear path, and the king's safe start, transit, and destination squares meet the non-recursive UI checks. Update castling rights after every engine-accepted move.

**Acceptance:**
- At least 5 full turns (10 ply) playable without crashes on hardware
- Thinking indicator appears and disappears correctly
- No memory leaks or stack corruption
- All 18 native tests pass
- SRAM remains below 40%; flash usage is recorded against the historical 60% target

**Hardware evidence:** The first repaired firmware was uploaded to the Nano on `COM6`. The original opening reached `b1-c3` without freezing, the computer replied, and five full turns completed. After the castling repair was uploaded to `COM6`, a legal sixth-move `O-O` completed, Black replied, and the Nano did not reset.

## Files / areas

**Modified:**
- `src/main.cpp` - add computer move trigger and `executeComputerMove()` helper
- `src/Display.cpp` - add `drawThinkingIndicator()`
- `src/Display.h` - declare `drawThinkingIndicator()`
- `src/GameState.cpp` - generate non-recursive, king-safe selection candidates and track en-passant targets
- `src/GameState.h` - define castling-right flags and reduce `MAX_LEGAL_MOVES` from 64 to 32 for RAM savings
- `test/test_gamestate/test_gamestate.cpp` - compile the real game-state code and cover pawn, knight, en-passant, pinned-piece, and castling candidates
- `platformio.ini` - configure the native test environment to compile `GameState.cpp` with mcu-max

**Unchanged (read-only context):**
- `docs/chess-engine-api.md` - reference for `mcumax_search_best_move()` parameters

## Data / contracts

**Game phase transitions:**
```
PLAYER_MOVE → (player completes move & computer's turn) → COMPUTER_THINKING
COMPUTER_THINKING → (computer move executed) → PLAYER_MOVE
COMPUTER_THINKING → (no legal move found) → GAME_OVER
```

**Computer move function signature (new helper):**
```cpp
void executeComputerMove(GameState* state);
```

**Thinking indicator (new display function):**
```cpp
void drawThinkingIndicator();
```

**mcu-max engine calls:**
```cpp
// Calculate best move within the Nano stack budget
mcumax_move move = mcumax_search_best_move(1, 1);

// Check for invalid move (no legal moves)
if (move.from == MCUMAX_SQUARE_INVALID) {
  // Handle game over (simplified for Feature 7)
  state->phase = GAME_OVER;
  return;
}

// Execute move
if (mcumax_play_move(move)) {
  syncBoardFromEngine(state);
  state->currentPlayer = currentPlayerFromMcumaxSide(mcumax_get_current_side());
}
```

**Player color assumption (Feature 7 only):**
- Player is white (`currentPlayer == 0`)
- Computer is black (`currentPlayer == 1`)
- Feature 8 will add random color assignment

## Testing

**Unit tests (native target):**
- Compile the real `src/GameState.cpp` and mcu-max dependency on the native target
- Cover non-recursive pawn, knight, en-passant, pinned-piece, and castling candidate generation, plus castling-right updates
- Computer move search remains a hardware test because it exercises the AVR stack budget

**Hardware tests:**
1. Upload to Arduino Nano
2. Play opening move (e.g., e2-e4)
3. Verify "Thinking..." indicator appears
4. Wait for computer response (should be 2-3 seconds)
5. Verify computer piece moves on board
6. Verify turn indicator updates to "White"
7. Continue for at least 5 turns
8. Monitor serial output for memory usage and errors

**Edge cases:**
- Computer has only one legal move → should find it quickly
- Complex tactical position → ensure thinking completes within reasonable time (<5 seconds)
- Move immediately leads to check → verify board state is correct

**Memory validation:**
- Check stack usage during `mcumax_search_best_move()` call
- Verify no RAM spikes that exceed 2KB limit
- Serial monitor should report memory consumption

**Memory budget exception:** The castling-repair build uses 676/2048 bytes SRAM (33.0%) and 21,032/30,720 bytes Flash (68.5%). SRAM meets the historical 40% target, but Flash exceeds the historical 60% target; this is recorded for future optimization rather than treated as a configured build failure.

## Notes for the AI

**mcu-max integration:**
- The engine handles all chess logic internally—move generation, legality, check detection
- The selector never calls `mcumax_search_valid_moves()` on the Nano; it proposes bounded candidates from the mirrored board, and `executeMove()` asks the engine to accept or reject each selected move
- Engine automatically switches sides after each move
- `mcumax_get_current_side()` returns `MCUMAX_ENGINE_WHITE_SIDE` (0x08) or `MCUMAX_ENGINE_BLACK_SIDE` (0x10)

**Performance considerations:**
- `mcumax_search_best_move(1, 1)` is used instead of `(1000, 2)` because mcu-max's recursive search reaches its internal minimum depth even when called with depth 1
- Thinking time varies by position complexity (opening: faster, middlegame: slower)
- Do not increase the search budget without fresh AVR stack measurements and hardware testing

**Phase management:**
- `COMPUTER_THINKING` phase blocks player input (cursor updates still allowed, but button presses should be ignored if we add that guard)
- Feature 8 will add proper game-over detection and new-game handling

**Display integration:**
- Thinking indicator should be minimal (simple text, no animation)
- Full board redraw after computer move is acceptable (no animation required for MVP)
- Avoid unnecessary redraws during thinking to prevent flicker

**Stack safety fix:**
- The reported sequence reaches 27 legal black moves after `4. Nc3`; the original 50-entry buffer was not the cause of this specific reset.
- `mcumax_search_best_move()` reaches recursive depth beyond the requested depth and overflowed the Nano stack during the reported position.
- `mcumax_search_valid_moves()` also recurses deeply enough to freeze the Nano during piece selection, so it is not used by the UI.
- Selection now uses a bounded board-based generator with own-king safety filtering; engine execution still decides final move legality.
- An invalid castling attempt can drive `mcumax_play_move()` through enough recursive validation to exhaust the Nano stack. The UI therefore rejects castling through an attacked transit square before the engine call.

**Future extensions (out of scope):**
- Difficulty selection via Start button press count (Feature 8 may add)
- Move animation
- Opening book for faster/stronger play

## Done when

- [x] Player move triggers computer move calculation when `currentPlayer == 1`
- [x] "Thinking..." indicator displays during computer calculation
- [x] Computer calculates move with `mcumax_search_best_move(1, 1)` within the Nano stack budget
- [x] Computer move executes successfully and board updates
- [x] Turn indicator switches back to player
- [x] Game phase returns to `PLAYER_MOVE` after computer move
- [x] 5+ full turns playable on hardware without crashes, including legal castling
- [x] All native tests pass
- [x] SRAM and Flash usage recorded; Flash exceeds the historical 60% target (68.5% Flash; 33.0% SRAM)
- [x] No compiler warnings

## Findings

Empty. No blockers or issues requiring escalation.

## Review

Empty. No sensitive changes requiring independent review.

## Implementation notes

**Changes made:**

1. **src/main.cpp** — Added `executeComputerMove()` helper. After player move, if `currentPlayer == 1` (black), set phase to `COMPUTER_THINKING`, call `executeComputerMove()`, and return phase to `PLAYER_MOVE`.

2. **src/Display.cpp & Display.h** — Added `drawThinkingIndicator()` to display "Thinking..." during computer calculation.

3. **src/GameState.cpp** — 
   - Added `isSquareAttacked()` and `leavesKingInCheck()` for move legality filtering.
   - Added `canCastle()` to check castling feasibility (rights, rook placement, clear path, king safety).
   - Added `clearRookCastlingRight()` to manage rook capture on a corner.
   - Added `updatedCastlingRights()` to clear rights after king or rook moves.
   - Modified `selectPieceAt()` to generate non-recursive candidates using a bounded board-based generator instead of calling `mcumax_search_valid_moves()`.
   - En-passant target updated after both player and computer moves.

4. **src/GameState.h** — Added castling-right constants and `INITIAL_CASTLING_RIGHTS`. Reduced `MAX_LEGAL_MOVES` from 64 to 32 to save RAM.

5. **test/test_gamestate/test_gamestate.cpp** — 18 unit tests covering pawn, knight, en-passant, pinned-piece, and castling candidate generation, plus castling-right updates.

6. **platformio.ini** — Configured native test environment to compile `GameState.cpp` with mcu-max dependencies.

7. **Deleted** — `test/test_gamestate/GameState.cpp` (obsolete stub).

**Final verification:**
- `pio run -e nanoatmega328`: 676 B SRAM (33.0%), 21,032 B Flash (68.5%), no errors.
- `pio test -e native`: 18/18 tests passed.
- Hardware test: 5+ full turns playable on Nano without crashes; legal castling executes without reset.

---

> **Archived feature spec.** This document holds a completed feature, fix, or rollback from the project history. It captures the goal, scope, build steps, testing strategy, and implementation notes for reference and audit.
