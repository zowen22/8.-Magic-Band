# BOM — Wireless Ornament Variant

*Created: 2026-07-07*
*Status: Draft — assumptions below need confirmation*

-----

## What This Is

The current build (documented in `4. Technical Reference.md`) already controls the tree lights wirelessly via 433MHz RF to the outlet — no mains wiring, no floor box. But the ornament itself is still tethered: it draws power through a 5.5mm/2.1mm barrel jack wired to a wall adapter.

This variant removes that cord. The ornament runs on an internal rechargeable battery, charged via USB-C when needed. Everything else (RFID, NeoPixels, RF transmitter, audio) is unchanged from the current architecture.

**Assumption flag:** if "wireless ornament" meant something other than untethering the power (e.g. a wireless data link, wireless charging pad, etc.), let me know and I'll rework this.

-----

## Bill of Materials

| Component | Part | Notes |
|---|---|---|
| Microcontroller | Arduino Nano (ATmega328P) | Unchanged |
| RFID Reader | MFRC522 | Unchanged, 3.3V, SPI |
| NeoPixels (outer) | Ring, 16px | Unchanged, Pin 5, NEO_GRB, 5V |
| NeoPixels (inner) | Ring, 11px | Unchanged, Pin 6, NEO_GRB, 5V |
| RF Transmitter | 433MHz transmitter module | Unchanged, Pin 8 |
| RF Outlet | Etekcity or similar 433MHz outlet | Unchanged, pre-paired before shipping |
| Audio | DFPlayer Mini + 8Ω 1W speaker | Unchanged, Pins 2/3 |
| MicroSD | Any small card | Unchanged, `001.mp3` |
| **Battery** | Single-cell LiPo, 3.7V nominal, JST-PH connector, **2000mAh** | Sized for multi-day use on tap-triggered duty cycle — see Power Budget below. Swap for a smaller cell (500–1000mAh) if shell space is tight. |
| **Charge/Boost** | Adafruit PowerBoost 1000C | LiPo charger + 5V boost (up to 1A continuous) in one board, USB-C input. Replaces barrel jack, wall adapter, *and* a separate charge/boost circuit. Has onboard battery-protection (no separate protection IC needed). |
| **Power switch** | SPDT slide switch on PowerBoost `EN` pin | Physical on/off without disconnecting the battery |

### Removed vs. the wired build
- 5.5mm/2.1mm panel-mount barrel jack
- 5V 3A wall adapter

-----

## Power Budget

| Component | Idle/typical | Worst case (peak) |
|---|---|---|
| Arduino Nano | ~20mA | ~50mA |
| MFRC522 (scanning) | ~13–26mA | ~26mA |
| NeoPixels (27px total, animation) | ~50–150mA (partial brightness/color) | ~1.6A (all 27px full white) |
| DFPlayer Mini + speaker | ~20mA idle / ~100–200mA playing | ~200mA |
| RF transmitter (burst, sending 5×) | negligible (short duty cycle) | ~30mA during burst |

**Two numbers matter here:**
- **Boost board headroom:** PowerBoost 1000C supports up to 1A continuous / short peaks higher. Full-white 27-pixel NeoPixel bursts alone can approach or exceed that — keep animation brightness/color moderate (e.g. cap `setBrightness()` well below 255) rather than sizing for worst-case draw. This is a firmware-side constraint, not just hardware.
- **Battery life:** the ornament is idle almost all the time between taps (RFID scanning loop only, LEDs off, no audio). Average draw is dominated by MFRC522 idle current, not animation peaks. A 2000mAh cell should comfortably run for many days of realistic seasonal use between charges.

-----

## Enclosure Impact (flag only — enclosure design is Phase 2 / WP2.2, out of scope here)

- The barrel jack's 11mm panel-mount hole is replaced by a USB-C cutout for charging access.
- PowerBoost 1000C board (~1.2" × 0.9") and the LiPo cell are new physical volume inside the shell, in addition to the Nano (18mm × 45mm). The battery is likely the largest new footprint constraint — worth checking cell dimensions against shell interior before ordering.
- Recommend exposing the USB-C port at the same shell location/orientation as the old barrel jack (bottom, cable/access facing down) to keep the enclosure design mostly reusable.

-----

## Open Questions

- Confirm the "wireless" scope assumption above (power untethering vs. something else).
- Confirm battery chemistry preference: rechargeable LiPo (this doc) vs. disposable AA/AAA (simpler, no charge circuit, but periodic swaps and needs its own regulator to hit 5V cleanly).
- Battery capacity vs. ornament size tradeoff — 2000mAh assumed here; confirm against actual shell interior volume once enclosure work (WP2.2) is further along.
