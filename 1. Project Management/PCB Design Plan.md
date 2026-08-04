# PCB Design Plan

*Created: 2026-08-04*
*Status: In Progress — v1 schematic capture (Stage 1) done and ERC-clean; layout (Stage 3) next*

-----

## What This Is

A custom PCB to replace the current point-to-point/breadboard wiring for the wireless ornament variant. Tool is KiCad. Two approaches were researched:

- **Carrier/breakout board (v1, this is what we're building first)** — the custom PCB only handles interconnect, the MCP1700-3302 regulator, and MOSFET gating. Pro Mini, MFRC522, NeoPixel rings, and the RF TX module stay as pluggable off-the-shelf modules. Chosen because a mistake here costs a cheap module respin, not a full board respin — the right risk profile for a first PCB.
- **Fully-custom (v2, future learning exercise)** — replace the Pro Mini specifically with a bare ATmega328P circuit. MFRC522 and the RF TX module stay as modules in *either* approach (see below) — bare-IC integration for those is real RF/antenna engineering, not a reasonable first-project target.

Claude is driving KiCad directly (writing/validating the actual project files via `kicad-cli` DRC/ERC checks and rendered exports) rather than walking through GUI steps — user's explicit preference.

-----

## V1: Carrier/Breakout Board

### Stage 0 — KiCad Setup & Learning
- KiCad 8+ (this session installed 10.0.5 via the official AppImage, extracted for `kicad-cli` access)
- Reference tutorials if the user wants to follow along: [Zbotic "Your First PCB"](https://zbotic.in/your-first-pcb-kicad-tutorial-from-scratch-for-beginners/) for general fluency, [Zbotic "Custom Arduino PCB"](https://zbotic.in/custom-arduino-pcb-design-your-own-minimal-board/) for the exact carrier-board pattern used here

### Stage 1 — Schematic Capture — **Done, 2026-08-04**
Built in `PCB/MagicBand_Carrier/MagicBand_Carrier.kicad_sch`, ERC-clean (0 errors). See Session Log 2026-08-04 for the three coordinate-system bugs hit and fixed while building it (embedded-symbol naming, even-vs-odd connector pin spacing, library Y-up vs schematic Y-down). NeoPixel rings and the 5V/GND board-power-input net (J8) are represented; 5V/GND get `PWR_FLAG` markers since their real source (Lipo Rider Plus) is off-board.

**Independent review, 2026-08-04**: a separate pass (not the builder) checked the schematic against real datasheets rather than just internal consistency. MOSFET topology, MFRC522 pin wiring, and power/ground net separation all confirmed correct against Adafruit's PID 5648 docs, a real RC522 breakout pinout, and the exported netlist respectively. One real problem found and fixed: the MCP1700-3302's decoupling caps (C1 input, C2 output) were both 100nF, but Microchip's datasheet (DS20001826) specifies 1.0µF as the standard/minimum **output** cap for stability -- undersized output capacitance risks oscillation, and the MFRC522's active-scan current step (13-26mA) is exactly the kind of load transient that stresses that margin. Fixed: C2 -> 1.0µF (datasheet-specified), C1 -> 10µF (typical reference-design input sizing for this part, not itself a stability requirement but good headroom against transient sag). Re-ran ERC after the fix -- still 0 errors, same 120 cosmetic warnings.

### Stage 2 — Footprints

| Part | Footprint status | Confidence |
|---|---|---|
| Arduino Pro Mini | Ready-made — [SnapEDA](https://www.snapeda.com/parts/Arduino%20Pro%20Mini/Arduino/view-part/), also [etimou/arduino-pro-mini-kicad](https://github.com/etimou/arduino-pro-mini-kicad) | High |
| MFRC522 breakout | [SnapEDA "RFID-RC522 by Handson Technology"](https://www.snapeda.com/parts/RFID-RC522/Handson%20Technology/view-part/) | Medium-high |
| D-FLIFE 433MHz TX module | **No footprint exists anywhere** — no datasheet, unbranded generic board. Hand-draw a 5-pad footprint from caliper measurements once the module's in hand (~30-60 min). Normal for cheap generic parts, not a blocker. | Low (expected) |
| MCP1700-3302 | Standard SOT-23-5 — [SnapEDA MCP1700T-3302E/TT](https://www.snapeda.com/parts/MCP1700T-3302E/TT/Microchip/view-part/), or KiCad's stock `Regulator_Linear` symbol + `Package_TO_SOT_SMD:SOT-23-5` footprint | Medium-high |
| Adafruit MOSFET driver (PID 5648) | Full breakout, JST-PH 2mm input connector — just needs the JST-PH footprint below | High |
| JST-PH 2-pin (battery + MOSFET driver) | Ships in stock KiCad — [KiCad/Connectors_JST.pretty](https://github.com/KiCad/Connectors_JST.pretty) | High |
| Lipo Rider Plus | **Recommended: leave off v1 board entirely** — see Open Decisions below | N/A if excluded |

### Stage 3 — Layout & RF Considerations
- Antenna keepout: 3-5mm copper/ground-plane clearance immediately around the TX module and antenna; ~10mm general component clearance beyond that
- Full λ/4 ground plane (~17cm) isn't realistic at this board size — accept the tradeoff; bench testing already showed working range from a breadboard setup, so a compact board with a modest keepout shouldn't perform meaningfully worse
- Place the TX module at a board edge, short direct traces, keep everything else on the opposite side from the antenna
- Sources: [PCBSync RF/Antenna Layout Guide](https://pcbsync.com/rf-pcb-antenna-design/), [NextPCB Antenna Keepout Best Practices](https://www.nextpcb.com/blog/pcb-antenna-layout-and-keep-out-design)

### Stage 4 — DRC & Fab Prep
Standard KiCad DRC pass, export Gerbers/drill files (JLCPCB has a KiCad-specific export guide).

### Stage 5 — Ordering / Assembly
- **JLCPCB**: ~$2 for 5 bare 2-layer boards + shipping, ~24hr fab (real end-to-end time with shipping likely 1-2 weeks — not independently verified). Free stencil over 5 units.
- **Assembly service**: may place the MCP1700 SOT-23 automatically if it's in JLCPCB's SMT parts catalog (check before ordering) — otherwise hand-soldering one SOT-23-5 part is very achievable. Minimum 5pcs for an assembly-service prototype order, +2-3 days turnaround.

### Board Outline
Don't block Stages 1-2 (schematic/footprints are dimension-independent) on shell dimensions. Do wait on those (WP2.2, still open) before finalizing the outline shape. Rough placeholder: likely comparable to or modestly larger than the Nano's existing 18x45mm footprint alone, once NeoPixel rings stay off-board — low confidence, KiCad will show the real minimum once footprints are placed.

### Open Decisions (flagged by research, not yet locked in)
- **Lipo Rider Plus / battery connector — recommended to exclude from v1**, keep wired externally. It needs its own USB-C panel cutout, and its footprint competes for space that can't be sized correctly until shell dimensions (WP2.2) are known. Fold into a v2 board later. Proceeding on this recommendation unless overridden.
- Final board outline — blocked on WP2.2 shell dimensions.

-----

## V2: Fully-Custom (Bare ATmega328P) — Future Learning Exercise

**Sequencing: attempt this after a working v1 carrier board**, not as the first attempt — isolates debugging so a second-revision board only has to prove out the new MCU circuit, not the whole system at once.

### What "fully custom" actually means here
MFRC522 and the RF TX module stay as modules in this version too — see feasibility notes below. "Fully custom" concretely means replacing just the Pro Mini with a bare-chip circuit. That's a more modest win than it sounds: a DIP-28 + crystal + caps + reset circuit isn't dramatically smaller than the Pro Mini module it replaces (rough estimate, not measured) — RFID/RF/MOSFET/connector footprints remain the dominant board-size drivers either way. Cost savings are also small at single-unit/hobby volume (~$2-3 bare chip + ~$1 support parts + a one-time $3-5 programmer, vs. ~$5-6.50 for a Pro Mini clone). This is a learning-driven decision, not a cost or size one — consistent with how it was originally framed.

### Bare ATmega328P circuit (confirmed standard, ~7-8 parts beyond the chip)
- 16MHz crystal across XTAL1/XTAL2, two 22pF load caps to ground, short traces to those pins
- 100nF decoupling cap on VCC/GND and on AVCC/GND, right at the pins; optional bulk 1uF cap
- 10kΩ pull-up, RESET → VCC
- 6-pin ISP header (MOSI, MISO, SCK, RESET, VCC, GND), 2×3, 0.1" pitch

**Package: DIP (ATmega328P-PU), not TQFP** — both have ready KiCad symbols/footprints, but DIP is hand-solderable with no reflow needed, and can be socketed (a bad joint or fried chip means pulling it out, not desoldering a 0.8mm-pitch TQFP by hand). Matches the same risk-minimization logic behind the v1 carrier-board choice.

**Programming: ISP-only for the first board, bootloader optional later.** Burn a bootloader once via a spare Nano-as-ISP (already have spare Nanos) or a $3-5 USBasp, then upload via an external FTDI adapter through the same `arduino-cli` workflow already set up this session — no new tooling to learn. Pure ISP-every-time also works and is simpler, but loses the familiar `arduino-cli upload` path.

### MFRC522 and RF TX at the bare-IC level — recommend against both, high confidence
- **MFRC522**: NXP's antenna design guide (AN1445) specs a full 13.56MHz resonant matching network — 8 capacitors, 2 inductors, 2 resistors, plus a symmetrical antenna coil that needs tuning (and would likely need re-tuning once mounted in the actual shell, since nearby materials detune it). This is antenna engineering, not "wire up a chip." Bare ICs are sourceable in small quantities, so it's not an availability problem — it's a tuning-skill problem.
- **RF TX**: internals are simple (SAW resonator + switching transistor + passives) so conceptually approachable, but replicating it means sourcing a 433.92MHz-tuned SAW resonator and doing discrete oscillator/matching design, with real risk of spurious emissions a pre-tested module doesn't have. At ~$1-2/module, there's no cost case for this, only a marginal size case.

### Learning references (on-point, not generic tutorials)
- [SiliconWit: "ATmega328P Breakout Board: Through-Hole PCB from Scratch"](https://github.com/SiliconWit/pcb-design-kicad/blob/main/atmega328p-breakout-board-through-hole.mdx) — KiCad 9, DIP/through-hole, matches the recommended approach above. Best starting reference.
- [trentfowler/atmega328p](https://github.com/trentfowler/atmega328p) — bare KiCad starter project files, good comparison reference
- [wickerbox/wickerlib](https://github.com/wickerbox/wickerlib/tree/master/templates/atmega328) — KiCad template with conservative 2-layer DFM rules
- [Chromico/stormduino](https://github.com/Chromico/stormduino) — full open-source Uno clone (DIP + CH340); useful as a "real complete board" reference, but skip studying its onboard USB-serial section since the ISP+external-FTDI approach above doesn't need it

-----

## Toolchain (this WSL environment)

- KiCad 10.0.5, installed via the official AppImage (apt is blocked in this sandbox — no root available), extracted to `~/.local/opt/squashfs-root`
- `kicad-cli` symlinked to `~/.local/bin/kicad-cli` — used for DRC/ERC validation and rendering exports so work can be checked without a GUI session
