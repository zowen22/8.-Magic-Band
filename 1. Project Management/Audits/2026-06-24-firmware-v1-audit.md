# Firmware v1 Code Audit

**Type:** Code Quality / Bug Review  
**Status:** Resolved  
**Priority:** High — 3 critical bugs would prevent standalone operation  
**Linked WP:** WP1 (initial firmware)  
**Date:** 2026-06-24

---

## Critical Bugs (Fixed)

### 1. `while (!Serial)` hangs standalone device
`setup()` blocked indefinitely waiting for a USB serial host. Device never starts on battery or barrel jack.  
**Fix:** Replaced with a 2-second timeout; device continues without serial if no host connects.

### 2. DFPlayer pins conflict with MFRC522 SPI bus
`PIN_MP3_TX = 11` (SPI MOSI) and `PIN_MP3_RX = 10` (SPI SS) shared pins with the RFID reader.  
Both peripherals driving the same lines causes bus collisions; audio and RFID behavior would be unpredictable.  
**Fix:** Moved DFPlayer SoftwareSerial to D2 (TX) / D3 (RX). **Update physical wiring to match.**

### 3. `player.play()` called without checking init status
`player.play(1)` ran unconditionally even if `player.begin()` returned false.  
**Fix:** Added `playerReady` bool; audio only fires when DFPlayer initialized successfully.

---

## Code Quality (Fixed)

| # | Issue | Fix |
|---|-------|-----|
| 4 | `String content` built and uppercased, never used; `String` class fragments heap on Nano over many scans | Removed |
| 5 | `void(*resetFunc)(void) = 0` — defined, never called | Removed |
| 6 | Early `return` in name-reading skipped `PICC_HaltA()`/`PCD_StopCrypto1()` — left RFID state dirty | Extracted `readGuestName()` with inline error handling; halt always called by loop |
| 7 | `buffer1[18]` and `buffer2[18]` — two arrays for the same sequential read | Consolidated to one `buffer[18]` |
| 8 | `PICC_DumpDetailsToSerial()` in main flow — verbose internal card dump not useful at runtime | Removed |
| 9 | Key/status/len declared at top of `loop()`, used only in name-reading tail | Moved to `readGuestName()` |
| 10 | RAM string literals (`"Speaker on"`, etc.) on a 2 KB Nano | Wrapped in `F()` macro throughout |
| 11 | `"SoftwareSerial.h"` / `"DFRobotDFPlayerMini.h"` — quotes for installed libraries | Changed to `<>` |
| 12 | `static const uint8_t` for pin defs at file scope — `static` meaningless here; inconsistent with other pins | Replaced with `const uint8_t` across all pins |
| 13 | `scanCount = 2` — same even/odd behavior from 0, clearer intent | Changed to 0 |
| 14 | `5 * 65536` in `rainbow2` — implicit long promotion works but intent unclear | Made explicit `5 * 65536L` |
| 15 | Dead card-detect check inside `rainbow()` — card detected but not acted on, just delays | Removed |
| 16 | Excessive `delay()` sprinkled through `setup()` without rationale | Consolidated |

---

## Open Items / Watchlist

- **UID filtering not implemented** — all cards currently authorized. Add a UID allowlist when the guest list is known.
- `ledsAccess()` and `rainbow()` are implemented but not wired to any trigger — available for future use.
- `SPIN_SPEED` and color constants are easily tunable at the top of the file.
