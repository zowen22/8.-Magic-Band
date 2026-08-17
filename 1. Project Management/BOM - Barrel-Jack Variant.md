# BOM — Barrel-Jack / Wall-Powered Variant

*Created: 2026-08-17*
*Compiled from: `PCB/MagicBand_BarrelJack/MagicBand_BarrelJack.kicad_sch`, `4. Technical Reference.md`, `PCB Design Plan.md`*
*Status: Draft — pricing not independently re-sourced this pass, carried forward from the wireless-variant BOM and PCB Design Plan notes where components overlap*

-----

## What This Is

This is the **active build** (Phase 4 / WP4.1): wall-powered via a 5.5mm/2.1mm barrel jack, RF-controlled outlet (no mains wiring, no floor box). Two implementations exist side by side:

1. **Nano-based (current production sketch)** — `Rev4.ino`, documented in `4. Technical Reference.md`. Off-the-shelf Arduino Nano + module stack, wired by hand. This is what's actually been built and shipped so far.
2. **Custom PCB (in progress)** — `PCB/MagicBand_BarrelJack/`, a bare-ATmega328P design derived from the paused battery variant with battery-specific complexity (MOSFET NeoPixel gating, sleep/wake) removed. **Design-complete**: schematic and layout have been through 5 rounds of adversarial PDR review, latest (2026-08-13) gave a clean bill of health with 0 blocking/should-fix items. **Not yet fabricated** — gated on hand-measuring the RF TX module's real footprint (currently a placeholder, no vendor footprint exists) and confirming the crystal's load-cap value once a specific part is chosen.

The BOM below covers the **custom PCB variant**, since that's the one with an actual component-level schematic to compile from. Audio (DFPlayer Mini) is **not on this PCB** — it stays optional and wired off-board via pins 2/3, same as the Nano build.

-----

## Bill of Materials — Custom PCB (baseline, no audio)

| Component | Part | Price | Notes |
|---|---|---|---|
| Microcontroller | **ATmega328P-PU**, DIP-28, bare chip | ~$2.50–4 | Replaces Pro Mini/Nano from earlier variants — no onboard USB/regulator, hand-solderable/socketable DIP package chosen deliberately (no reflow needed) |
| Crystal | 16MHz, HC-49 or similar | ~$0.50–1 | **Y1.** Load-cap value (C1/C2, currently 22pF placeholder) needs confirming against the actual part's datasheet once sourced |
| Load caps | 22pF ×2 (C1, C2) | ~$0.05–0.10 ea (bulk) | Crystal load caps — see above, value is generic pending real crystal spec |
| Decoupling caps | 100nF ×3 (C3, C4, C_AREF), 1µF (C5), 10µF (C6), 1.0µF (C7) | ~$0.05–0.15 ea (bulk) | Standard MCU/regulator decoupling. C5 was flagged and fixed to meet MCP1700's 1.0µF datasheet minimum during PDR review |
| Reverse-polarity protection | Q1 — AO3401A P-MOSFET (SOT-23) + R1, R2 (10k) | ~$0.10–0.30 (Q1) + negligible (resistors) | Ideal-diode protection ahead of the regulator/MCU — barrel jack is bare flying leads with no mechanical keying, so reversed/wrong-voltage adapters are a real risk. Chosen over a series diode to avoid eating into the 16MHz ATmega's voltage headroom |
| 3.3V Regulator | MCP1700-3302 LDO (SOT-23) | ~$0.50–1/unit (10–20 packs) | **U2.** Steps 5V down to 3.3V for MFRC522 + RF TX module. Same part/rationale as the wireless-variant BOM |
| RFID Reader | MFRC522 module | ~$1.25–5 (bulk) / up to $10 (kit) | **J2**, stacking female socket (`PinSocket_1x08`) — module plugs in shield-style, then hand-soldered for a permanent joint (board is sealed in the ornament) |
| RF Transmitter | D-FLIFE 433MHz ASK TX module | ~$1–2/unit | **J3.** Footprint is still a placeholder — no vendor footprint exists anywhere; needs hand-measuring with calipers once physically in hand, before this board can be sent to fab |
| NeoPixels | LED strip, count TBD (strip on order) | TBD | **J6.** Single outer strip only — inner ring dropped from the design 2026-08-06 |
| ISP Header | 6-pin AVR ISP, 2×3 | ~$0.50–1 | **J9.** Shares SPI bus with J2 (MFRC522) — unplug the MFRC522 module before programming via ISP |
| Power input | 5.5mm/2.1mm barrel jack, bare flying leads | ~$1–2 | **J8.** Hand-soldered directly to THT pads, not a board-mounted connector (a real KiCad `Barrel_Jack` symbol was tried and rejected — its pin geometry produced a genuine +5V/GND short) |
| PCB fab | 40mm × 40mm, 2-layer | **Not yet priced** | Board outline hit its 40×40mm target with zero courtyard overlaps. Get a real JLCPCB quote once Gerbers are finalized (blocked on the RF TX footprint above) |
| RF Outlet | Etekcity or similar 433MHz outlet kit | ~$25–30 | Pre-paired before shipping; not part of the board itself |
| Power supply | 5V 3A wall adapter, barrel jack output | ~$8–12 | Buyer-supplied or included; standard commodity, well above this board's actual draw |
| Programmer (one-time tooling) | USBasp or spare Nano-as-ISP | ~$3–5 | Not per-unit — bench tooling only, used to flash via the ISP header |

**Per-unit baseline total (excl. RF outlet, wall adapter, PCB fab, one-time tooling): ~$8–15**
**Board-level components (chip + passives + regulator + MOSFET + ISP header): ~$5–8 of that**

### Optional add-on: Audio (not on this PCB, wired off-board — same as Nano build)

| Component | Part | Price |
|---|---|---|
| Audio player | DFPlayer Mini | $8.75–11 |
| Speaker | 8Ω 1W mini speaker | $2.99 |
| Storage | MicroSD, 4–8GB | ~$5–8 |

Adds ~$17–22. Wired to pins 2/3 same as the Nano build — no PCB footprint exists for it, so this is a hand-wired bolt-on if wanted.

-----

## Power Budget

Wall-powered, so no runtime/battery math needed — the 5V/3A adapter has wide headroom over any realistic draw here (MCU ~15–20mA active, MFRC522 ~13–26mA scanning, RF TX ~20–30mA burst, NeoPixels the only component that can meaningfully swing — full-white worst case scales with strip length once count is known). The one design decision this drove: PDR #4 flagged the +5V trace as uniformly 0.2mm with no widened trunk for LED current — the J8→Q1 input leg was widened to 0.5mm, but the rest of the net was kept at 0.2mm after review concluded the LED load's duty cycle doesn't need more (0.2mm at 1oz copper has headroom for the expected ~0.7–1A continuous case).

-----

## Enclosure Impact (flag only — enclosure design is Phase 2 / WP2.2, out of scope here)

- Barrel jack: 5.5mm/2.1mm panel-mount, requires an 11mm hole in the shell; mount on the bottom so the cable exits downward
- Minimum 15mm clearance behind the jack inside the shell for solder tabs + wiring
- Board target is 40mm × 40mm, but the MFRC522 module stacks on top as a shield — common MFRC522 breakouts run ~40×60mm or larger, so the *effective* footprint inside the ornament may exceed the bare board's outline. Not confirmed until the actual module is in hand
- Shell interior dimensions still unconfirmed (open in WP2.2/WP4.1) — blocks setting a final board outline target with certainty

-----

## Open Questions

- RF TX module (J3) footprint — hand-measure against the real part before ordering fab; currently a placeholder with no vendor data
- Crystal (Y1) load-cap value — generic 22pF placeholder until a specific crystal part is chosen
- NeoPixel strip pixel count — TBD pending the strip order; recalculate power/pricing once known
- PCB fab cost — no real JLCPCB quote pulled yet for this specific board
- Ornament shell interior dimensions — needed to finalize board outline and confirm the MFRC522 module's stacked footprint actually fits
