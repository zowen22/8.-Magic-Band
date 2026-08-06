# BOM — Wireless Ornament Variant

*Created: 2026-07-07*
*Updated: 2026-08-04*
*Status: **PAUSED 2026-08-04** — LiPo batteries carry real shipping/hazmat friction and marketplace listing restrictions (Etsy) if this becomes a sellable product. Not abandoned, may revisit if that constraint changes. Preserved as-is for reference; project has pivoted to a barrel-jack/wall-powered architecture. RFID/RF-transmitter/NeoPixel component choices below remain directly relevant regardless of power architecture — only the battery/charging section (LiPo, Lipo Rider Plus) and the battery-driven firmware complexity (sleep/wake, MOSFET gating) are specific to this paused variant.*

-----

## What This Is

The current build (documented in `4. Technical Reference.md`) already controls the tree lights wirelessly via 433MHz RF to the outlet — no mains wiring, no floor box. This variant also untethers the ornament's power: it runs on an internal rechargeable battery, charged via USB-C, instead of a barrel jack + wall adapter.

Audio (DFPlayer Mini) is **optional here, same as the wired build** — not part of the baseline BOM or power budget below, but listed separately if wanted.

-----

## Bill of Materials (baseline, no audio)

| Component | Part | Price | Consumption | Notes |
|---|---|---|---|---|
| Microcontroller | **Arduino Pro Mini, 5V/16MHz** | ~$5–6.50 (clone) / ~$10 (SparkFun official) | Active: ~15–20mA · Sleep: ~4.5µA (BOD off) | Swap from Nano — no onboard CH340 USB chip, so no 5–20mA sleep-mode floor. Same ATmega328P, same pin behavior/code. |
| RFID Reader | MFRC522 module | ~$1.25–5 (bulk) / up to $10 (kit) | Active scan: 13–26mA · Power-down: 10µA | 3.3V, SPI |
| NeoPixels | LED strip, count TBD (strip on order, replaces prior two-ring design 2026-08-06) | TBD | TBD — recalculate once strip length/pixel count known | Pin 5, NEO_GRB, 5V |
| RF Transmitter | D-FLIFE 433MHz ASK TX module ([Amazon B0BZRRBBNK](https://www.amazon.com/dp/B0BZRRBBNK), 5-pack) | ~$1–2/unit | Transmit: 20–30mA · Standby: ~0mA | Data → D8. **Power in is 3.3V, not 5V** — shares the added 3.3V regulator's output with the MFRC522 (see next row), not the raw 5V bus. See [RF Module Pinout - D-FLIFE 433MHz Kit.md](RF%20Module%20Pinout%20-%20D-FLIFE%20433MHz%20Kit.md) for full pinout. |
| **3.3V Regulator** | MCP1700-3302 LDO (e.g. [Amazon](https://www.amazon.com/MCP1700-3302E-MCP1700-Microchip-Voltage-Regulator/dp/B084LBDPC7)) | ~$0.50–1/unit (sold in 10–20 packs) | Quiescent: **1.6µA** · well under its 250mA rating for MFRC522+TX's combined peak (~56mA) | **Added 2026-08-04, corrected same day.** Pro Mini has only one onboard regulator (5V) — unlike the Nano, which has a separate 3.3V pin sourced from its USB-serial chip's own regulator. Steps the Pro Mini's 5V VCC down to 3.3V to feed the MFRC522 and RF TX module. **Not AMS1117** — that part's ~3-10mA quiescent current would run continuously off the Pro Mini's always-on 5V rail and dominate the entire sleep budget (target is ~1.3-1.5mA *total*). MCP1700's 1.6µA is genuinely negligible against that budget. |
| RF Outlet | Etekcity 433MHz outlet kit | ~$25–30 | N/A — mains powered | Pre-paired before shipping; not part of ornament battery budget |
| **Battery** | Adafruit 2000mAh 3.7V protected LiPo, JST-PH (product #2011) | $12.50 | — (source) | Onboard protection circuit (over-charge/discharge/short) confirmed |
| **Charge/Boost** | Seeed Studio Lipo Rider Plus, 5V/2.4A, USB-C | ~$5.39–6.50 | No-load quiescent current: **unverified** — flag for bench test | Replaces PowerBoost 1000C, which is out of stock. No minimum-load auto-shutoff (dedicated project-power board, not a "smart" power bank IC) — safe for always-on/duty-cycled loads |
| **Power gating** | Adafruit MOSFET Driver, PID 5648 (×1) | $3.95 | Gate draws ~0mA | Cuts NeoPixel VCC when not animating — the LEDs draw ~0.6mA/pixel even "off" if VCC stays connected (28px ≈ 16.8mA wasted if ungated) |

**Baseline total (excl. RF outlet): ~$45–53**
**All-in with outlet kit: ~$70–83**

### Optional add-on: Audio (same status as wired build)

| Component | Part | Price | Consumption |
|---|---|---|---|
| Audio player | DFPlayer Mini | $8.75–11 | Standby: ~20mA · Playing: 20–150mA (up to 200mA) |
| Speaker | 8Ω 1W mini speaker | $2.99 | Draws only while playing (included above) |
| Storage | MicroSD, 4–8GB | ~$5–8 | Negligible |

Adds ~$17–22 to cost. If added, needs its own MOSFET gate (or share the NeoPixel gate if audio and animation always fire together) and pulls the peak current during a tap event up toward the Lipo Rider Plus's 2.4A ceiling — worth re-checking headroom if this gets added later.

### Removed vs. the wired build
- 5.5mm/2.1mm panel-mount barrel jack
- 5V 3A wall adapter

-----

## Power Budget & Runtime

| Component | Idle/typical | Worst case (peak) | Source |
|---|---|---|---|
| Pro Mini (ATmega328P core) | Active ~15–20mA / Sleep ~4.5µA | ~20mA | Standard datasheet figure, low confidence — not independently re-verified |
| MFRC522 | Continuous scan: 13–26mA / Power-down: 10µA | 26mA | [NXP datasheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf) |
| NeoPixels (28px total) | Off, gated: ~0mA / Off, ungated: ~16.8mA | 1.68A (all 28px full white) | [Adafruit Überguide](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels) (worst case) + [bench-measured off-state](https://refcircuit.com/articles/876-quiescent-current-of-addressable-led-ws2812-measurement.html) |
| RF transmitter | Standby ~0mA | 20–30mA during burst, on the **3.3V rail** (shared with MFRC522, not the 5V bus) | [components101.com](https://components101.com/modules/433-mhz-rf-transmitter-module) (general ASK TX figures) + [D-FLIFE listing](https://www.amazon.com/dp/B0BZRRBBNK) for the specific sourced part's pinout/voltage |
| Boost converter (Lipo Rider Plus) | No-load Iq: **unverified** | Rated 2.4A | Open gap — bench test recommended |

**Two scenarios, same hardware:**

- **Properly engineered** — Pro Mini in power-down sleep, MFRC522 woken and polled ~once/second (not continuously), NeoPixels MOSFET-gated off between animations: average draw ≈ **1.3–1.5mA** on the 5V bus. Converting through the boost stage (~1.7× for voltage step-up + ~80% efficiency) → **~2.2–2.6mA off the battery**. On the 2000mAh cell: **~2–4 weeks per charge.**
- **Unoptimized fallback** (continuous scanning, nothing power-gated) — Pro Mini ~20mA + MFRC522 ~20mA + NeoPixels ungated ~16.8mA ≈ **~57mA continuous** on the bus → battery-side ~97mA → **~18–20 hours per charge**. (Dropping audio already removed ~20mA from this fallback case vs. the earlier draft that included it.)

The gap between these two numbers is entirely the firmware sleep/wake loop and the MOSFET gating — not optional nice-to-haves if multi-week runtime matters.

-----

## Enclosure Impact (flag only — enclosure design is Phase 2 / WP2.2, out of scope here)

- The barrel jack's 11mm panel-mount hole is replaced by a USB-C cutout for charging access.
- Lipo Rider Plus board and the LiPo cell are new physical volume inside the shell, alongside the Pro Mini (smaller footprint than the Nano it replaces). The battery is likely the largest new footprint constraint — check cell dimensions against shell interior before ordering.
- MOSFET driver board adds minor additional volume — small enough it shouldn't be a fit-blocker.

-----

## Open Questions

- Boost converter no-load quiescent current — unverified, bench-test before trusting the multi-week runtime estimate.
- Pro Mini active-current figure — confirm against Microchip's ATmega328P datasheet directly rather than the general estimate used here.
- Battery capacity vs. ornament size tradeoff — 2000mAh assumed; confirm against actual shell interior volume once WP2.2 enclosure work is further along.
- Whether to add audio later — kept optional/out of baseline per this update; re-check peak-current headroom on the Lipo Rider Plus if it's added.
