# Chess Library Evaluation

## Evaluation Date
2026-11-08

## Target Platform
- **Board**: Arduino Nano (ATmega328P)
- **SRAM**: 2KB (2048 bytes total)
- **Flash**: 32KB
- **Memory Budget**: <1000 bytes SRAM for chess library, <1500 bytes total system SRAM

## Baseline Memory Usage
- Current baseline (LED blink test): **188 bytes RAM**, **2186 bytes Flash**
- Available for chess engine: ~1312 bytes RAM (after reserving 1000 bytes for game state)

## Candidate Libraries

### 1. MicroChess
- **Repository**: https://github.com/ripred/MicroChess
- **Last Update**: 2024 (Active, Release 1.0.0)
- **Stars**: 31 | **Forks**: 4
- **License**: MIT

**Features:**
- Full chess rules: en passant, castling, pawn promotion, check, checkmate, stalemate
- Quiescent search for better position evaluation
- 6-ply look-ahead support
- Stack depth checking for adaptive search
- Designed for <2K RAM and 32K Flash

**API Surface:**
- Board representation with piece tracking
- Move generation and validation
- Position evaluation with minimax/alpha-beta
- Game state management

**Claimed Memory**: <2KB RAM total (includes display and game logic)

**Arduino Nano Compatibility**: ✅ Explicitly designed for Arduino Uno/Nano

**Notes**: 
- Full-featured engine with complete game implementation
- May include UI code that increases footprint
- Need to extract core engine only for our use case

---

### 2. mcu-max
- **Repository**: https://github.com/Gissio/mcu-max
- **Last Update**: 2024 (43 commits, actively maintained)
- **Stars**: 22 | **Forks**: 7
- **License**: MIT

**Features:**
- Based on micro-Max (minimal chess engine)
- MCU-optimized C implementation
- Configurable max depth and node limit
- Valid move listing
- Best-move search termination
- UCI chess interface support
- Arduino serial port example included

**API Surface:**
- FEN position support
- Configurable hashing
- Move generation
- Position evaluation

**Claimed Memory**: Low-resource design for MCUs (specific numbers not stated)

**Arduino Nano Compatibility**: ✅ Arduino Library available, PlatformIO compatible

**PlatformIO**: Listed in Arduino Libraries registry as "mcu-max"

**Notes**:
- Clean, minimal design based on proven micro-Max algorithm
- Available as Arduino library (easy integration)
- Explicitly designed for memory-constrained MCUs

---

### 3. htcw_chess
- **Repository**: https://github.com/codewitch-honey-crisis/htcw_chess
- **Last Update**: 2024 (19 commits)
- **Stars**: 2 | **Forks**: 1
- **License**: MIT

**Features:**
- Implements chess rules only (no AI/evaluation)
- Move validation and generation
- Castling and en passant support
- Check, checkmate, stalemate detection
- Move preview functionality
- Score tracking

**API Surface:**
- `chess_init()` for initialization
- Move generation and validation
- Board representation in C structs
- No automated play (rules engine only)

**Claimed Memory**: "Compact" implementation (no specific numbers)

**Arduino Nano Compatibility**: ✅ Available as Arduino library

**PlatformIO**: Listed in Arduino Libraries registry as "htcw_chess"

**Notes**:
- Rules-only library (no position evaluation or AI)
- Would need separate minimax implementation for computer opponent
- Potentially smallest footprint (no evaluation logic)
- Not suitable for our requirements (needs evaluation for difficulty levels)

---

### 4. Arduino Chess (viktorstrate)
- **Repository**: https://github.com/viktorstrate/arduino-chess
- **Last Update**: 2017 (Last commit ~7 years ago)
- **Stars**: Unknown from search
- **License**: Not specified in search results

**Features:**
- Mini-max algorithm implementation
- Designed for Arduino

**Claimed Memory**: Not specified

**Arduino Nano Compatibility**: Designed for Arduino but unclear

**Notes**:
- Project appears unmaintained (2017)
- Limited information available
- Not in Arduino library registry
- **Eliminated from evaluation** due to age and lack of maintenance

---

## Initial Assessment (Pre-Testing)

### Top Candidates for Memory Testing:

1. **mcu-max** - Purpose-built for MCUs, clean API, maintained, easy integration
2. **MicroChess** - Proven <2KB design, full-featured, but may need core extraction
3. **htcw_chess** - Minimal rules-only (eliminated: lacks evaluation)

### Eliminated:
- **Arduino Chess (viktorstrate)**: Unmaintained, unclear documentation
- **htcw_chess**: No position evaluation (required for difficulty levels)

---

## Memory Measurements

### Test Configuration
- **Profiling harness**: Custom test in `src/main.cpp` with memory reporting
- **Initial position**: Starting chess position (32 pieces)
- **Mid-game position**: 10 pieces (4k3/8/8/3pp3/3PP3/8/8/4K3 w - - 0 1)
- **Search depth**: 1 ply (depth=1, nodes=100) for quick profiling

### Results

| Library | SRAM (bytes) | Flash (bytes) | Initial Position | Mid-Game Position | Notes |
|---------|--------------|---------------|------------------|-------------------|-------|
| **Baseline** | 250 | 2772 | N/A | N/A | Profiling harness only |
| **mcu-max** | **598** | **7194** | ✅ Tested | ✅ Tested | Within budget! |
| **MicroChess** | Not tested | Not tested | - | - | Not in PlatformIO registry |

### Detailed mcu-max Measurements

**Static Allocation (from build output):**
- SRAM: 598 bytes (29.2% of 2048 bytes)
- Flash: 7194 bytes (23.4% of 30720 bytes)
- **Remaining**: 1450 bytes RAM, 23526 bytes Flash

**Memory Breakdown:**
- Baseline overhead: 250 bytes
- mcu-max library: 348 bytes (598 - 250)
- **Available for game state**: ~1450 bytes (well above 1000 byte target)

**Test Results:**
- ✅ Engine initialization successful
- ✅ Valid move generation works (20 moves from initial position)
- ✅ Best move search functional
- ✅ FEN position loading works
- ✅ Mid-game position tested successfully

### Memory Budget Analysis

| Component | Budget | Actual | Status |
|-----------|--------|--------|--------|
| Chess library | <1000 bytes | 348 bytes | ✅ Pass (65% under budget) |
| Total system SRAM | <1500 bytes | 598 bytes | ✅ Pass (60% under budget) |
| Available for game state | ~1000 bytes | 1450 bytes | ✅ 45% margin |

---

## Selection Criteria

1. **Memory constraint**: SRAM usage <1000 bytes (hard requirement)
2. **Feature completeness**: Legal move generation + position evaluation
3. **Maintainability**: Active project, clear API, MIT license
4. **Integration ease**: Available in library registry or clean import
5. **Arduino Nano support**: Confirmed compatibility

---

## Selected Library

**mcu-max v1.0.7** ✅

### Selection Rationale

**Why mcu-max:**

1. **Memory Efficiency** ⭐
   - Only 348 bytes RAM for full chess engine
   - 65% under our 1000-byte budget
   - Leaves 1450 bytes for game state (45% safety margin)

2. **Feature Completeness** ⭐
   - Full legal move generation (`mcumax_search_valid_moves`)
   - Position evaluation with configurable depth (`mcumax_search_best_move`)
   - FEN position support for testing
   - Supports difficulty levels via node_max/depth_max parameters

3. **Integration & Maintenance** ⭐
   - Available in PlatformIO registry (`gissio/mcu-max @ ^1.0.7`)
   - Actively maintained (2024, 43 commits)
   - MIT license
   - Clean C API with Arduino example

4. **Arduino Nano Compatibility** ⭐
   - Explicitly designed for MCUs
   - Tested on ATmega328P (2KB RAM)
   - Includes Arduino serial port example
   - Based on proven micro-Max algorithm

**Why not MicroChess:**
- Not available in PlatformIO registry (manual installation required)
- Claims <2KB RAM but unclear how much of that is engine vs UI
- Would require extracting core from full game implementation

**Why not htcw_chess:**
- Rules-only library (no position evaluation)
- Would need separate minimax implementation
- Cannot meet requirement for difficulty levels without additional work

### Difficulty Level Implementation Plan

mcu-max supports configurable search strength via two parameters:
- `node_max`: Maximum nodes to search
- `depth_max`: Maximum search depth (ply)

**Proposed difficulty mappings:**

| Difficulty | Estimated Elo | node_max | depth_max | Est. Time |
|------------|---------------|----------|-----------|-----------|
| Easy | ~700 | 500 | 1 | <1s |
| Medium | ~900 | 1000 | 2 | 2-3s |
| Hard | ~1100 | 2000 | 3 | 5-8s |

*Note: These values will be tuned in Feature 3 based on actual gameplay testing.*

---

## Integration Notes

### PlatformIO Configuration

Added to `platformio.ini`:
```ini
lib_deps = 
    gissio/mcu-max @ ^1.0.0
```

Installed version: **mcu-max@1.0.7**

### API Summary

**Core Functions Used:**
```c
// Initialization
void mcumax_init(void);

// Position setup
void mcumax_set_fen_position(const char *fen);

// Move generation
uint32_t mcumax_search_valid_moves(mcumax_move *buffer, uint32_t buffer_size);

// AI search
mcumax_move mcumax_search_best_move(uint32_t node_max, uint32_t depth_max);

// Move execution
bool mcumax_play_move(mcumax_move move);

// Board inspection
mcumax_piece mcumax_get_piece(mcumax_square square);
mcumax_piece mcumax_get_current_side(void);
```

**Data Types:**
```c
typedef uint8_t mcumax_square;  // 0xRF format (R=rank 0-7, F=file 0-7)
typedef struct { mcumax_square from, to; } mcumax_move;
```

### Build Verification

✅ Compiles successfully with Arduino Nano (ATmega328P)  
✅ Memory usage: 598 bytes RAM (29.2%), 7194 bytes Flash (23.4%)  
✅ Move generation tested and working  
✅ FEN position loading tested and working

### Next Steps for Feature 2

The selected library provides all functionality needed for the game loop:
1. Initialize engine with `mcumax_init()`
2. Display board using `mcumax_get_piece()` for each square
3. Get valid moves for move hints/validation
4. Execute player moves with `mcumax_play_move()`
5. Generate computer moves with `mcumax_search_best_move()`
6. Check game state (checkmate/stalemate) via move count

---

**Evaluation completed**: 2026-11-08  
**Selected library**: mcu-max v1.0.7  
**Memory verified**: 348 bytes (65% under budget)  
**Status**: ✅ Ready for integration

