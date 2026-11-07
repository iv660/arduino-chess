# Current Feature

## Feature 1: Chess library evaluation and integration

**Branch:** `feature/startup`  
**Status:** Verified  
**Build attempt:** 1

### Overview

Evaluate four candidate chess libraries for Arduino, measure their memory footprint, and integrate the best-fit library that satisfies our strict memory constraints while providing full legal move generation and basic position evaluation for three difficulty levels.

### Requirements

- Chess library must use <1000 bytes SRAM (hard constraint)
- Total system SRAM after integration must remain <1500 bytes
- Library must provide legal move generation
- Library must support position evaluation for 700/900/1100 Elo difficulty levels
- Must work with Arduino Nano (ATmega328P: 2KB SRAM, 32KB Flash)
- Integration must preserve baseline build (currently 188 bytes RAM, 2186 bytes Flash)

### Candidate Libraries

1. **MicroChess** - Minimal embedded chess engine
2. **mcu-max** - Microcontroller chess with MAX algorithm
3. **htcw_chess** - Embedded chess library
4. **Arduino Chess OLED** - Arduino-specific chess implementation

### Build Steps

#### Step 1: Research candidate libraries
- Search GitHub and Arduino library registry for each candidate
- Document repository URLs, last update dates, and stated memory requirements
- Record API surface area (move generation, evaluation, board representation)
- Check Arduino Nano compatibility claims
- **Done when:** `docs/library-evaluation.md` exists with research findings table

#### Step 2: Create memory profiling harness
- Add minimal test harness in `src/main.cpp` that initializes each library
- Include Serial output for memory reporting (using `__brkval` for heap)
- Use conditional compilation (`#define TEST_LIBRARY_X`) to test one at a time
- **Done when:** Harness compiles and runs with dummy/baseline case

#### Step 3: Measure actual memory usage
- Build and upload each candidate with profiling harness
- Record SRAM usage from build output and runtime Serial reports
- Record Flash usage from build output
- Test with initial position and mid-game position (8-10 pieces)
- **Done when:** All four candidates measured, data in `docs/library-evaluation.md`

#### Step 4: Select library and add to dependencies
- Choose library that meets <1000 byte SRAM constraint with best tradeoff of features/size
- Add to `platformio.ini` `lib_deps` if available in registry, or document manual installation
- Document selection rationale in `docs/library-evaluation.md`
- **Done when:** Selected library builds successfully, memory verified

#### Step 5: Create integration test
- Write minimal integration test in `src/main.cpp` that:
  - Initializes chess library
  - Generates legal moves from starting position
  - Makes one move
  - Verifies board state changed
  - Reports results via Serial (115200 baud)
- **Done when:** Test runs, outputs "Integration test: PASS" to Serial

#### Step 6: Document chess engine API
- Create `docs/chess-engine-api.md` with:
  - Initialization calls
  - Move generation interface
  - Position evaluation interface
  - Memory requirements (measured)
  - Example usage for our three difficulty levels
- **Done when:** API doc complete and matches selected library

### Files Changed

**New:**
- `docs/library-evaluation.md` - Research findings and memory measurements
- `docs/chess-engine-api.md` - Chess library API documentation

**Modified:**
- `platformio.ini` - Add chess library dependency
- `src/main.cpp` - Integration test code (temporary, will be replaced in Feature 2)

### Testing

- Build verification: `pio run` succeeds
- Memory check: SRAM usage ≤1500 bytes total in build output
- Integration test: `pio run --target upload && pio device monitor` shows "Integration test: PASS"

### Success Criteria

- [x] All four candidates researched and documented
- [x] Memory measurements complete for all candidates
- [x] One library selected that meets <1000 byte SRAM constraint
- [x] Library integrated and building successfully
- [x] Integration test passes and outputs to Serial
- [x] API documentation complete
- [x] Build passes with memory budget satisfied

### Notes

- This is a research and integration spike - we expect to discard some exploration code
- If no library meets the <1000 byte constraint, we'll need to implement a minimal custom engine (out of scope for this feature)
- Integration test is temporary scaffolding - real game loop comes in Feature 2
- Serial output at 115200 baud matches existing baseline test

---

## Archive metadata

**Completed:** 2026-11-08  
**Spec SHA-256:** `bf1b09ecad600184fb74add77d29c0d874462aba65d17e7cf44d32c20f4b6e60`  
**Branch:** `feature/startup`  
**Merge base:** `56b2f6147d18fc974780f4007caf318f3188b1b2` (main)  
**Work HEAD:** `963a968bbc0f85289cc128a9533fb303ecdf9c13`

## Selection summary

**Selected:** mcu-max v1.0.7  
**Memory:** 348 bytes RAM (65% under <1000 byte budget), 6642 bytes Flash total  
**Available remaining:** 1454 bytes RAM for game state/UI

**Why mcu-max:**
- Lowest memory footprint of all testable candidates (348 bytes)
- Full legal move generation and minimax evaluation
- Available in PlatformIO registry (`gissio/mcu-max @ ^1.0.0`)
- MIT license, actively maintained (2024)
- Clean C API with Arduino examples
- Proven micro-Max algorithm

**Why not others:**
- MicroChess: Not in PlatformIO registry, requires manual extraction
- htcw_chess: Rules-only, no position evaluation
- Arduino Chess: Unmaintained since 2017

## Difficulty mappings

| Difficulty | Estimated Elo | node_max | depth_max | Est. Time |
|------------|---------------|----------|-----------|-----------|
| Easy       | ~700          | 500      | 1         | <1s       |
| Medium     | ~900          | 1000     | 2         | 2-3s      |
| Hard       | ~1100         | 2000     | 3         | 5-8s      |

*Values will be tuned in Feature 3 based on gameplay testing.*
