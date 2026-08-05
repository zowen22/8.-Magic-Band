# PCB Design Plan

*Created: 2026-08-04 · Last reorganized: 2026-08-04*

Claude drives KiCad directly (writing/validating actual project files via `kicad-cli` DRC/ERC checks and rendered exports) rather than walking through GUI steps — user's explicit preference. This doc is organized by topic, not chronologically — see `5. Session Log.md` for the chronological blow-by-blow.

-----

## 1. Active Board: Barrel-Jack (Wall-Powered)

**Status: Schematic complete, ERC-clean, survived two independent adversarial design reviews. Layout (placement + routing + DRC) is next.**

Location: `PCB/MagicBand_BarrelJack/MagicBand_BarrelJack.kicad_sch` + `MagicBand_BarrelJack/MagicBand_BarrelJack.ino`.

### 1.1 What it is
Wall-powered via a barrel jack (not battery — see §3 for why the battery variant was abandoned). Derived from the paused fully-custom battery board, carrying forward its proven sub-circuits and dropping battery-only complexity.

### 1.2 Target board outline: **40mm × 40mm — aggressive, not yet confirmed achievable**
User's target, set 2026-08-04. Real risk worth flagging now rather than discovering at layout: the MFRC522 module stacks directly on top of this board (see §1.4) as a physical shield, and common MFRC522 breakouts run roughly 40x60mm or larger -- if the purchased module is bigger than 40x40mm, it will overhang the custom board's edges. That's not necessarily a functional problem (no electrical issue), but it does mean the *effective* footprint inside the ornament may exceed 40x40mm regardless of how tightly the custom board itself is laid out. Layout stage needs to report the real achievable minimum and flag clearly if 40x40mm isn't met, rather than silently missing the target or silently shrinking scope to hit it.

Also still blocked on real ornament shell interior dimensions (WP2.2, open) for final confirmation -- 40x40mm is the design target in the meantime, not a confirmed-fits number.

### 1.3 Component list & footprint status

| Component | Role | Footprint status |
|---|---|---|
| ATmega328P-P (DIP-28) | MCU | Real KiCad symbol/footprint, high confidence |
| MFRC522 module | RFID reader | Stacking female socket (`PinSocket_1x08_P2.54mm_Vertical`) -- see 1.4 |
| D-FLIFE 433MHz TX module | RF transmitter | **No real footprint exists anywhere** -- no datasheet, unbranded board. Placeholder now; hand-draw from calipers once physically in hand (~30-60 min, not a blocker) |
| MCP1700-3302 (SOT-23) | 3.3V regulator for MFRC522+RF TX | Real footprint, kept deliberately (see 1.6) |
| Q1 -- AO3401A (SOT-23) + R2 (10k) | Reverse-polarity/overvoltage protection | Real part, pinout independently verified against the actual datasheet image (see 3 methodology) |
| Barrel jack (5.5mm/2.1mm, center-positive) | Power input | **Bare flying leads, hand-soldered to plain THT pads (J8)** -- not board-mounted, not a purchased connector. See 1.5 for why. |
| JST-PH 3-pin x2 (J6/J7) | NeoPixel ring connectors | Real footprint (`JST_PH_B3B-PH-K_1x03_P2.00mm_Vertical`), user's choice for easy connect/disconnect |
| 6-pin AVR ISP header (J9) | Programming | Standard 2x3, shares SPI bus with MFRC522 -- see gotcha below |
| Crystal (16MHz) + 2x22pF load caps | MCU clock | Real footprint; **cap value is generic/placeholder** until a specific crystal part (with known rated load capacitance) is chosen |
| Decoupling caps, reset pull-up (R1) | Standard support circuitry | Real, standard values, datasheet-verified |

### 1.4 Physical layout constraints -- do not violate at layout stage

- **RFID (MFRC522, stacking socket) + MCU stack must sit at the FRONT of the ornament** -- the interactive/tap end.
- **Barrel jack must be at the BACK.** Bare leads run from a rear-panel-mounted jack forward to J8's solder pads -- board orientation inside the shell is dictated by this split, not arbitrary.
- **MFRC522 stacks directly onto the board** (shield-style, plugs into J2's female socket) -- same electrical wiring as a wired connection, just mechanical: solves "two loose boards to mount" without touching the RFID antenna (chip-level MFRC522 integration was evaluated and rejected -- real 13.56MHz antenna-matching engineering, not worth the risk -- see 3.4). **Assembly note**: solder the plugged-in pins after seating, don't rely on socket friction alone, for a permanent joint (user's explicit preference, since the board is sealed inside the ornament forever).
- **RF antenna keepout**: 3-5mm copper/ground-plane clearance immediately around the TX module and its antenna; ~10mm general component clearance beyond that. Place the TX module at a board edge with short, direct traces; keep other components on the opposite side of the board.
- **MFRC522 module footprint clearance**: treat the area under/around where it sits like the antenna keepout -- no other tall components underneath, so the two boards can actually stack flush.
- **MFRC522/ISP shared-SPI-bus gotcha**: MOSI/MISO/SCK are shared between J2 (MFRC522) and J9 (ISP header). **Unplug the MFRC522 before programming via ISP** -- its presence on the bus can contend with the programmer on MISO while the chip is held in reset. Worth a physical reminder label on the assembled board, not just this doc.

### 1.5 Power input: bare-lead barrel jack, not a connector
Represented in the schematic as plain 2-pin THT pads (`J8`, Value `PWR_IN_BarrelJack_BareLeads`), not a purchased connector or a board-mounted jack part. The physical barrel jack mounts through the ornament's rear panel with its own flying leads, hand-soldered to these pads. A real KiCad `Barrel_Jack` library symbol was tried and rejected -- its pin geometry didn't match the intended wiring and produced a genuine short (both pins landing on +5V, GND left disconnected) that ERC didn't catch (see 3.2). The generic-pads approach avoids that risk entirely and matches how the jack is actually going to be assembled.

### 1.6 Regulator: MCP1700-3302, a deliberate keep
Originally chosen for its 1.6uA quiescent current (battery-life optimization). That advantage is moot now that the board is wall-powered, and it's a somewhat pricier/more specialty SOT-23 LDO than a generic 3.3V regulator. Kept anyway: it's correctly wired, correctly decoupled (10uF in / 1.0uF out, matching Microchip's datasheet minimum for output-cap stability -- this specific value was itself a bug caught and fixed early on, see 3.1), and there's no functional benefit to churning the design just to swap in a marginally cheaper part. Recorded here so this reads as a conscious tradeoff, not unexamined inertia.

-----

## 2. Firmware: `MagicBand_BarrelJack.ino`

Split off from the older `MagicBand_Wireless.ino` (battery-variant sketch) rather than continuing to edit a misleadingly-named sketch for a now wall-powered board. Removed: the ~1Hz sleep/wake watchdog loop and the MOSFET NeoPixel-gating calls -- both existed purely to conserve battery current, which a continuously wall-powered board doesn't need to manage. Their removal also fixed a real UX regression: the old 1Hz polling meant up to a ~1 second delay between tapping a card and the ornament responding, with zero remaining upside once battery life stopped being a constraint. RFID-read, RF-fire, and NeoPixel animation logic is unchanged in behavior. A small idle delay (50ms) was added between poll cycles so the RFID reader isn't hammered with literally zero gap.

-----

## 3. Design Review Methodology

**Established practice for this project, not a one-time event -- apply this again before ordering the actual board, and to any future schematic/layout work.**

The core discipline: **ERC/DRC passing is necessary but not sufficient.** Both check internal consistency (every pin connected to something, no rule violations) -- neither checks whether a net actually connects the *right* signals, or whether a part's real-world pinout matches what the schematic assumes. A self-consistent but backwards-wired schematic passes ERC clean every time. Every real bug found on this project was in that gap:

1. **MCP1700 decoupling cap undersized** (100nF vs. the datasheet's 1.0uF minimum) -- found by an independent review checking against Microchip's actual datasheet, not just the schematic's internal logic.
2. **MOSFET driver connector wrong pitch** (2.54mm drawn, real part is 2mm JST-PH) -- found by checking the connector against the real vendor's product docs, not just "a 3-pin connector exists."
3. **Barrel jack short** (both pins on +5V, GND disconnected) -- ERC reported 0 errors the entire time. Only caught by manually reading net *membership* in an exported netlist.
4. **Reverse-polarity MOSFET wired backwards** (Source/Drain transposed vs. the real AO3401A) -- the fix for one bug introduced this one. Caught by a second independent review round, then personally verified by rendering the actual manufacturer datasheet PDF to an image locally (`pymupdf`, no root/poppler-utils needed) and reading the physical pin diagram directly -- not trusting web search summaries, which gave contradictory answers for this exact part.

**Practical rules that follow from this**:
- After any schematic change, export and manually inspect net membership for anything safety- or correctness-critical (power rails, protection circuits) -- don't stop at "ERC clean."
- Before trusting a part's pinout, check the primary datasheet source directly if the part is unusual or the stakes are high (reverse-polarity, power). Web search summaries for pinouts have proven unreliable -- cross-check or render the real datasheet page yourself.
- Run an independent review pass (a different agent/session, not the one that built the change) before layout/fab commitments -- every real bug on this project was caught this way, none were caught by the builder re-checking their own work.
- After closing review findings, run the review again. The MOSFET pin-swap bug above is a direct product of this: round 1 found a real gap (no protection circuit) and the fix for it introduced a new bug, which round 2 caught. One review pass is not enough for anything going to fab.

### 3.1 Why the battery-powered variant was abandoned
LiPo batteries carry real shipping/hazmat friction (UN38.3 testing, carrier dangerous-goods rules) and marketplace listing restrictions (Etsy) if this becomes a sellable product. Paused, not deleted -- see 4. Pivoted to barrel-jack/wall power instead, which also let the design shed the sleep/wake and MOSFET-gating complexity that battery life had required (see 2).

### 3.2 KiCad tooling gotchas found this project (useful for any future KiCad work here)
- `kicad-cli`'s headless netlist/ERC engine resolves a symbol's pin Y-position as `placement_y - local_y` (not `+`) relative to the library's local pin definition. Confirmed empirically across every symbol type used. Getting this backwards doesn't produce an ERC error -- it produces a wrong but self-consistent netlist.
- `kicad-cli sch export netlist` prints "schematic has annotation errors" on every board built this project, including ones that are genuinely ERC-clean with all-unique reference designators (checked). Most likely a side effect of the still-missing sym-lib-table/fp-lib-table setup (open task), not a real defect -- worth a GUI re-annotate check once that's set up, to close the loop.
- KiCad's `(extends "...")` symbol inheritance (used by variant parts like `ATmega328P-P extends ATmega48PV-10P`) doesn't resolve pins in this headless, no-library-table context -- metadata resolves fine, pins silently don't. Fix: flatten the variant into a standalone symbol rather than relying on inheritance.
- The official `kicad-symbols` repo lives on GitLab now (`gitlab.com/kicad/libraries/kicad-symbols`), split into per-part `.kicad_symdir` directories -- fetch individual files via the GitLab file API, not a guessed flat raw URL.
- This WSL environment's KiCad install is the AppImage "lite" variant (no bundled `kicad-footprints`) -- verify footprint names against the official `kicad-footprints` GitHub repo's API rather than assuming a local library file exists.

### 3.3 Toolchain (this WSL environment)
- KiCad 10.0.5, installed via the official AppImage (apt is blocked in this sandbox -- no root available), extracted to `~/.local/opt/squashfs-root`, `kicad-cli` symlinked to `~/.local/bin/kicad-cli`.
- `pymupdf` (Python, already available, no install needed) -- used to render datasheet PDF pages to images locally when a primary source needs direct visual verification and `poppler-utils`/`pdftoppm` isn't available (root-blocked).

### 3.4 Why MFRC522 and the RF TX module stay off-chip (both boards, permanently)
Evaluated and rejected for chip-level integration, high confidence either way:
- **MFRC522**: NXP's antenna design guide (AN1445) specs a full 13.56MHz resonant matching network -- 8 capacitors, 2 inductors, 2 resistors, plus a tuned antenna coil that would likely need re-tuning once mounted in the actual shell (nearby materials detune it). Real antenna engineering, not "wire up a chip." Bare ICs are available, so it's not a sourcing problem -- it's a tuning-skill/risk problem.
- **RF TX**: conceptually simple internals (SAW resonator + switching transistor + passives), but replicating it means sourcing a 433.92MHz-tuned SAW resonator and discrete oscillator/matching design, with real risk of spurious emissions a pre-tested module doesn't have. At ~$1-2/module, no cost case for doing this yourself.

-----

## 4. Archived: Earlier Board Variants (reference only, not active)

Two earlier boards were built and are preserved for reference -- their proven sub-circuits (RFID wiring, RF TX wiring, connector-fit findings) carried forward into the active barrel-jack board; battery-specific content did not.

### 4.1 V1: Carrier/Breakout Board (`PCB/MagicBand_Carrier/`)
First attempt -- kept the Pro Mini, MFRC522, RF TX module, and NeoPixel rings as pluggable off-the-shelf modules rather than integrating anything at chip level, specifically to minimize first-PCB risk (a mistake costs a cheap module respin, not a full board respin). Schematic complete, ERC-clean (0 errors, 120 warnings), independently reviewed (caught the MCP1700 decoupling bug, 3). PCB layout was never completed for this board -- superseded before reaching that stage.

Useful reference: the Stage 2 footprint-sourcing table researched for this board (Pro Mini, MFRC522, D-FLIFE TX module, MCP1700, JST-PH -- sourcing/confidence notes) and the Stage 5 fab/ordering research (JLCPCB pricing, assembly-service scope) both still apply generally.

### 4.2 V2: Fully-Custom, Battery-Powered (`PCB/MagicBand_FullyCustom/`)
Replaced the Pro Mini with a bare ATmega328P-P (DIP-28) circuit -- crystal, load caps, decoupling, reset pull-up, 6-pin ISP header -- while keeping MFRC522/RF TX as modules (see 3.4 for why). Added a MOSFET NeoPixel-gating circuit (Q1/R2/R3, low-side switching via a `GND_SW` net) for battery-life current conservation. Schematic complete, ERC-clean (0 errors, 206->208 warnings across two revisions). This is the direct ancestor of the active barrel-jack board -- everything except the MOSFET gating and the battery-specific regulator rationale carried forward (see 1).

Paused along with the rest of the battery variant (3.1) before PCB layout was attempted -- a partial layout WIP existed briefly but was discarded as stale once the schematic changed (MOSFET integration, then the barrel-jack pivot).

**"Fully custom" scope note** (still relevant if this ever gets revisited): replacing just the Pro Mini with a bare-chip circuit is a more modest win than "fully custom" sounds -- RFID/RF/connector footprints remain the dominant board-size drivers either way, and cost savings are small at hobby volume (~$2-3 bare chip + ~$1 support parts + a one-time $3-5 ISP programmer, vs. ~$5-6.50 for a Pro Mini clone). This was a learning-driven decision, not a cost or size one.

Package choice: **DIP (ATmega328P-PU), not TQFP** -- hand-solderable with no reflow needed, and can be socketed (a bad joint means pulling the chip, not desoldering a fine-pitch part by hand). Programming: **ISP-only for the first board**, bootloader-once optional later via a spare Nano-as-ISP or a cheap USBasp, then the familiar `arduino-cli upload` path via an external FTDI adapter.

Learning references used, still on-point if revisited: [SiliconWit ATmega328P Breakout PCB from Scratch](https://github.com/SiliconWit/pcb-design-kicad/blob/main/atmega328p-breakout-board-through-hole.mdx) (best starting reference), [trentfowler/atmega328p](https://github.com/trentfowler/atmega328p), [wickerbox/wickerlib](https://github.com/wickerbox/wickerlib/tree/master/templates/atmega328), [Chromico/stormduino](https://github.com/Chromico/stormduino) (skip its USB-serial section).
