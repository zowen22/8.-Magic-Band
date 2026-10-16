# Build Instructions — Barrel-Jack Variant

*Created: 2026-09-21. Board: `PCB/MagicBand_BarrelJack/`. See `BOM - Barrel-Jack Variant.md` for sourcing and `4. Technical Reference.md`'s "ISP Programming" section for background on the flashing gotchas referenced below.*

-----

## Before you start — parts checklist

**From JLCPCB** (assembled): the board itself, with C1-C8, Q1, R1/R2, U1 (MCU, direct chip), U2, Y1, J6, J9 already populated. J2, J3, J8 arrive as **bare, empty through-holes** — nothing pre-installed there by design.

**You still need to have in hand:**
- [ ] Spare Arduino Nano (for flashing)
- [ ] 6-pin 2×3 IDC ribbon cable (female sockets both ends) — cut one end, solder to the Nano
- [ ] A 10µF electrolytic capacitor (ArduinoISP gotcha, see Step 1)
- [ ] MFRC522 RFID module
- [ ] D-FLIFE 433MHz TX module (already owned)
- [ ] Pre-wired 5.5mm/2.1mm barrel jack, center-positive
- [ ] NeoPixel LED strip + JST-PH 3-pin pigtail (strip length/count still an open decision as of this doc — see BOM doc)
- [ ] Firmware ready to flash (current production sketch — see `4. Technical Reference.md`'s Sketches table)

-----

## Step 1 — Flash firmware via J9 (ISP), **before soldering anything to J2**

**Do this first, while J2 is still empty.** Two independent reasons:
1. **Physical clearance** — J2 and J9 are only ~7mm apart (flagged in the 2026-09-21 adversarial PDR). With J2 empty there's nothing to interfere with the ISP cable.
2. **Shared SPI bus** — J2 and J9 share MISO/MOSI/SCK. Once the MFRC522 module is soldered to J2, it can contend with the programmer on MISO during flashing. Before J2 is populated, that risk doesn't exist at all. Since J2 will be a **permanent solder joint** (not a socket), this is your one guaranteed-clean flashing window — get firmware right (or at least reflashable-adjacent) now if you can, since future reflashes will need to work around the module already being there.

**J9 pinout** (traced from the schematic, standard AVR ISP-6):

| J9 pin | Signal |
|---|---|
| 1 | MISO |
| 2 | VCC (+5V) |
| 3 | SCK |
| 4 | MOSI |
| 5 | RESET (target) |
| 6 | GND |

**Nano → J9 wiring:**

| Nano pin | J9 pin | Signal |
|---|---|---|
| 5V | 2 | VCC |
| GND | 6 | GND |
| D10 | 5 | RESET |
| D11 | 4 | MOSI |
| D12 | 1 | MISO |
| D13 | 3 | SCK |

**Procedure:**
1. Upload **File → Examples → 11.ArduinoISP → ArduinoISP** to the Nano (normal Mini-USB upload).
2. Bridge a **10µF electrolytic cap between the Nano's RESET pin and GND** (+ to RESET). Without this, avrdude's handshake resets the Nano itself instead of putting it into ISP-passthrough mode. Remove the cap before ever reprogramming the Nano again.
3. Wire the Nano to J9 per the table above (via your cut/soldered IDC cable).
4. In Arduino IDE: Board = "Arduino Pro or Pro Mini" (ATmega328P, 5V, 16MHz), Programmer = "Arduino as ISP".
5. **Sketch → Upload Using Programmer.** Confirm success in the console before moving on.

-----

## Step 2 — Solder J2 (RFID reader) directly to the board

No header or socket — the MFRC522 module's own pins go straight through J2's 8 through-holes and get soldered on the underside. This is a **permanent joint**, per your call to skip the plug-in connector for a more secure connection.

**J2 pin mapping** (traced from the schematic — matches the MFRC522 module's own standard silkscreen order exactly, so aligning the module's printed labels against these positions is correct):

| J2 pin | Signal |
|---|---|
| 1 | SDA / SS |
| 2 | SCK |
| 3 | MOSI |
| 4 | MISO |
| 5 | IRQ (intentionally not connected on our board) |
| 6 | GND |
| 7 | RST |
| 8 | 3.3V |

This is the same order printed on virtually every MFRC522 breakout (HiLetgo, SunFounder, etc.) — line up the module's own pin-1 marking with J2's pin-1 pad and the rest follows.

-----

## Step 3 — Solder J3 (RF transmitter) directly to the board

Same approach as J2 — the D-FLIFE TX module's own pins solder straight through J3's 4 through-holes, no header. A prior session (2026-08-17) specifically repositioned J3's pad layout to match the D-FLIFE module's **real physical pin order (Enable–Data–Power–GND)**, so align the module's own silkscreen directly against J3's board silkscreen rather than counting pin numbers — the physical layout was built to match.

J3 is **not** on the shared SPI bus, so there's no equivalent timing concern to Step 1 — solder it whenever convenient.

-----

## Step 4 — Solder J8 (barrel jack)

No header here either — J8 is bare THT pads by design. Solder the pre-wired jack's 2 flying leads directly.

**Verify before soldering: center pin = +5V, sleeve = GND.** Check this against whatever specific jack you bought — a prior board revision caught a real reverse-polarity risk from a mismatched symbol, so don't assume.

-----

## Step 5 — Connect J6 (NeoPixel strip, outer ring)

J6 is already fab-placed (JST B3B-PH-KL, keyed) — nothing to solder to the board here. Crimp or solder the JST-PH 3-pin pigtail onto the LED strip's own cut wire end, then plug it into J6.

**Wire color convention — this one runs backwards of normal, confirmed intentional (2026-10-13):** the sourced pigtail kit's own internal wiring is yellow=data/**black=+5V**/**red=GND** — inverted from the standard red=power/black=ground convention (flagged 2026-10-02, WP4.1). Solder the outer ring strip's wires to **match the pigtail's actual colors, not the universal convention** — i.e. the strip's black wire goes where the pigtail carries +5V, red where it carries GND. Validated 2026-10-13 via a dedicated isolation test (`Arduino/MagicBand_BarrelJack_JstIntactTest/`): a correctly-wired full JST connection (data+power+ground all through the connector, matching the pigtail's real colors) ran clean through a rainbow cycle and solid-color holds including full white, with no issues. **Do not solder this strip in standard red=+/black=GND convention** — that mismatch against the pigtail's actual wiring is exactly what caused the second smoke incident (see Work Packages WP4.3, 2026-10-13).

-----

## Step 5a — Second LED strip (inner ring, bodge wiring, not a designed connector)

*(Added 2026-10-13 — second strip is a late addition on the already-fabricated rev 1 board, no header for it exists. Rev 2 should give this its own proper connector — see Work Packages WP4.3.)* Two separate points of soldering, not one:

**Data line** — bodge a wire directly to U1's own DIP leg, not to any existing connector:
- U1 is a 28-pin DIP. With the pin-1 notch/dot oriented up, pin 1 is the top-left pin; numbering runs down the left column, then back up the right.
- Use **D2 (Arduino digital pin 2 = ATmega328P physical pin 4 = PD2)** — genuinely unused (confirmed via netlist: `unconnected-(U1-PD2-Pad4)`, no stale net name baggage, unlike D6). It's the 4th pin down the left column from the pin-1 notch, two pins past RESET.
- Solder the strip's data wire to this leg directly.

**Power + ground** — direct to the barrel jack, **standard wire-color convention** (red=+5V, black=GND — this strip is NOT going through the inverted-color pigtail, so use the normal convention here, unlike Step 5's outer ring):
- Two ways to land it, your choice: solder directly to the jack's own leads, or use the ordered **KF128-2.54-2P screw terminal block** (2.54mm pitch, 8A/130V rated, 18-26AWG — see Work Packages WP4.3 for the sourced part) landed on J8's existing footprint for a clamp-in connection instead of a solder joint.
- Either way, this bypasses the board's thin 0.2mm +5V trunk entirely (the confirmed smoke-incident bottleneck) — same reasoning as the outer ring's power/ground fix, see Work Packages WP4.3.
- **Trade-off, same as before**: this path is upstream of Q1's reverse-polarity protection, so this strip's wiring isn't covered by it the way the rest of the board is.
- **Wiring discipline, learned the hard way (second smoke incident, 2026-10-13)**: this strip uses standard colors, the outer ring's pigtail uses inverted colors (Step 5 above) — don't infer either one from the other. Multimeter-verify polarity on each strip independently before powering up.

-----

## Step 6 — Dry-fit check

Before final assembly (or at least before you're fully committed), physically test-fit the soldered MFRC522 module against J9's location — confirm the module's body doesn't block access to J9 for any future ISP cable connection. This was flagged, not verified, in the 2026-09-21 PDR (neither footprint has courtyard geometry, so DRC can't catch it).

-----

## Step 7 — Power up and test

1. Apply 5V via the barrel jack.
2. Scan an RFID card, confirm the expected LED/RF response.
3. If the 433MHz outlet isn't already paired: run `Sniffer.ino` once (receiver on pin 2, capture the remote's codes), fill in `RF_CODE_ON`/`RF_CODE_OFF`/`RF_BITS`/`RF_PROTOCOL` in the production sketch, reflash (you'll need J2 unplugged... except it's permanently soldered now — see Step 1's note about this being your main flashing window), put the outlet in learn mode, scan a card to pair. See `4. Technical Reference.md`'s "RF Workflow (per unit)" section for the full per-unit pairing flow.
