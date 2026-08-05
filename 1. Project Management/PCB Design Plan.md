# PCB Design Plan

*Created: 2026-08-04*
*Status: **PAUSED 2026-08-04** — project pivoted back to a barrel-jack/wall-powered architecture (LiPo shipping/hazmat + Etsy listing concerns, see `2. Project Overview.md` Decisions Log). Both schematics below (v1 carrier board, v2 fully-custom) are preserved as reference, not deleted — the RFID/RF-TX wiring, connector-fit findings (JST-PH pitch mismatch catch), and MOSFET integration all carry forward to whatever board follows. Battery-specific content (regulator quiescent-current optimization, MOSFET gating for battery life, the Pro Mini sleep/wake architecture) is not needed on wall power and can likely be dropped/simplified in a follow-on design. PCB layout was not completed for either board before this pause.*

-----

## Reverse-Polarity Protection: Source/Drain Pin Swap Fixed (2026-08-04)

A second-round adversarial design review caught a real bug in the just-added reverse-polarity protection circuit (Q1, AO3401A ideal-diode): its Source and Drain were wired backwards relative to the real part's pinout. The schematic's Value label had assumed `G-D-S` pin order (pin2=Drain, pin3=Source); the actual AOS AO3401A datasheet is `G-S-D` (pin1=Gate, pin2=Source, pin3=Drain).

This was independently confirmed by the parent session directly from the primary source -- rendered the real AOS datasheet PDF to an image locally (`pymupdf`, no root/poppler needed) and read the SOT-23 "Top View" pin diagram, rather than trusting either the circuit's original assumption or the review's claim on faith. Both agreed once verified against the actual datasheet image.

As originally wired, the circuit likely would not have functioned -- possibly blocking power entirely even under correct polarity, the exact "worse than no protection" failure mode this circuit exists to avoid.

**Fix**: swapped which net lands on Q1 pins 2 and 3 (pin 2 -> raw incoming lead from J8, pin 3 -> protected +5V rail; pin 1/Gate unchanged), corrected the Value label to `G-S-D`. Verified via netlist export both before and after the fix, not just ERC (which cannot catch this class of bug -- a self-consistent but backwards net assignment still passes ERC clean). ERC re-run clean afterward: 0 errors, 209 warnings, no regression.

**Process note**: getting the coordinate math right took real care -- this file has a documented `kicad-cli` Y-axis quirk (see the Stage-1 entry above) that has bitten two prior rounds. This fix cross-verified the target pin's actual coordinates two independent ways (via the R2 resistor's known-good geometry, and via J6's already-netlist-confirmed Conn_01x03 pin layout) before touching anything, rather than repeating the same class of mistake a third time.

## LED Strip Connector: JST-PH (2026-08-04)

J6/J7 (outer/inner ring headers) changed from generic 2.54mm pin headers to JST-PH 3-pin connectors (`JST_PH_B3B-PH-K_1x03_P2.00mm_Vertical`, same footprint family already verified for the MOSFET driver's connector). User is cutting their own WS2812B strip (confirmed 5V, 144/m, individually addressable, protocol-compatible with existing firmware) and terminating their own JST housing onto the cut ends -- board-side JST chosen for ease of connect/disconnect during assembly, per user request ("for simplicity").

**Pin order, identical on both J6 and J7 -- solder the strip's wires to match this exactly:**
| Pin | Signal |
|---|---|
| 1 | DATA (J6→D5 outer ring, J7→D6 inner ring) |
| 2 | +5V |
| 3 | GND |

No industry-standard pin order exists for JST-terminated WS2812 strips (varies by manufacturer) -- since the user is terminating both the strip end and the connector end themselves, consistency with the table above is what matters, not matching some external standard.

## Physical Layout Constraint (2026-08-04, barrel-jack board) -- do not violate at layout stage

*Referenced directly from Work Packages' "PCB layout for the barrel-jack board" task -- see `3. Work Packages.md`.*

- **RFID (MFRC522 stacking socket) + MCU (ATmega328P) stack must sit at the FRONT of the ornament** -- the interactive end, where the user taps their band.
- **Barrel jack connection must be at the BACK.** The barrel jack itself is a separate panel-mount part with bare flying leads (this is how barrel jacks are most commonly available) -- it is NOT board-mounted. It mounts through the rear of the ornament shell, and its two leads run forward to solder pads on the board (J8, relabeled `PWR_IN_BarrelJack_BareLeads` in the schematic to make this explicit -- plain THT solder pads, not a purchased connector).
- **Layout implication**: place J8 at the physical edge of the board that ends up nearest the rear of the ornament, and the MFRC522/MCU footprints at the opposite (front) edge -- board orientation inside the shell is not arbitrary, it's dictated by this front/back split.


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
| MCP1700-3302 | Standard SOT-23 (3-pin) — [SnapEDA MCP1700T-3302E/TT](https://www.snapeda.com/parts/MCP1700T-3302E/TT/Microchip/view-part/), or KiCad's stock `Regulator_Linear` symbol + `Package_TO_SOT_SMD:SOT-23` footprint | Medium-high |
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
- **Assembly service**: may place the MCP1700 SOT-23 automatically if it's in JLCPCB's SMT parts catalog (check before ordering) — otherwise hand-soldering one SOT-23 part is very achievable. Minimum 5pcs for an assembly-service prototype order, +2-3 days turnaround.

### Board Outline
Don't block Stages 1-2 (schematic/footprints are dimension-independent) on shell dimensions. Do wait on those (WP2.2, still open) before finalizing the outline shape. Rough placeholder: likely comparable to or modestly larger than the Nano's existing 18x45mm footprint alone, once NeoPixel rings stay off-board — low confidence, KiCad will show the real minimum once footprints are placed.

### Open Decisions (flagged by research, not yet locked in)
- **Lipo Rider Plus / battery connector — recommended to exclude from v1**, keep wired externally. It needs its own USB-C panel cutout, and its footprint competes for space that can't be sized correctly until shell dimensions (WP2.2) are known. Fold into a v2 board later. Proceeding on this recommendation unless overridden.
- Final board outline — blocked on WP2.2 shell dimensions.

-----

## V2: Fully-Custom (Bare ATmega328P) — Future Learning Exercise

**Sequencing: attempt this after a working v1 carrier board**, not as the first attempt — isolates debugging so a second-revision board only has to prove out the new MCU circuit, not the whole system at once.

### Stage 1 — Schematic Capture — **Done, 2026-08-04**
Built in `PCB/MagicBand_FullyCustom/MagicBand_FullyCustom.kicad_sch`, ERC-clean (0 errors, 206 cosmetic warnings — same grid-alignment/library-table categories as v1). Bare ATmega328P-P (DIP-28) replaces the Pro Mini header: 16MHz crystal + 22pF load caps (Y1/C1/C2), 100nF decoupling on VCC/GND and AVCC/GND (C3/C4), 1uF bulk cap (C5), 100nF AREF decoupling to GND (C_AREF, not driven externally), 10k RESET pull-up (R1), and a 6-pin AVR ISP header (J9, standard MISO/VCC/SCK/MOSI/RESET/GND pinout) sharing the SPI lines with MFRC522. MFRC522, RF TX module, MOSFET driver, NeoPixel headers, 5V input, and the MCP1700-3302 regulator are carried over from v1 unchanged (same net names: D4-D13, +5V, +3V3, GND, GND_SW), including v1's independent-review decoupling fix (C6=10uF in / C7=1.0uF out, not 100nF — see v1's Stage 1 entry for why). Every net independently verified against design intent via exported netlist (not just "ERC clean," which only checks connectivity, not correctness) — all 9 Arduino-pin-to-ATmega-physical-pin mappings, MCU_RESET, GND_SW, and all three power rails matched exactly.

**Coordinate-system finding (matters for all future KiCad work here, not just this file):** `kicad-cli`'s headless netlist/ERC engine resolves a symbol's pin Y-position as `placement_y − local_y` (local Y from the library's `(pin ... (at x y angle))` definition), not `placement_y + local_y`. This was confirmed empirically across every symbol type used (Device:C, Device:R, Connector_Generic:Conn_01x11, Connector_Generic:Conn_02x03_Odd_Even, Regulator_Linear:MCP1700x-300xxTT — cross-checked against v1's own committed file as ground truth — and MCU_Microchip_ATmega:ATmega328P-P) via isolated single-symbol probe tests (place a symbol, wire a distinctly-named global label to a guessed coordinate, export the netlist, read back which real pin the engine associated with that point). X is unaffected. Getting this wrong doesn't show up as an ERC error — a self-consistent but wrong-net schematic still passes ERC clean, since ERC checks connectivity, not correctness against intent, which is exactly why the netlist-based net-by-net verification step above matters as much as the ERC pass itself.

**KiCad library-fetch note:** the official `kicad-symbols` repo (needed for real parts like ATmega328P-P) migrated to GitLab (`gitlab.com/kicad/libraries/kicad-symbols`) and now splits large families into per-part `.kicad_symdir` directories rather than one flat `.kicad_sym` file per family — fetch individual part files via the GitLab file API (`/api/v4/projects/.../repository/files/<url-encoded-path>/raw?ref=master`), not a flat raw URL guess. Also: KiCad's `(extends "...")` symbol inheritance (used by variant parts like `ATmega328P-P extends ATmega48PV-10P`) does not resolve in this headless, no-library-table context — pins won't appear in ERC/netlist at all even though top-level metadata (Value, Footprint, Description) resolves fine. Fix: flatten the variant into a standalone symbol (copy the base's full graphic/pin content under the variant's name, drop the `extends` reference) rather than embedding both and relying on inheritance — same workaround the v1 fork already used for MCP1700, now confirmed necessary more generally.

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

## MOSFET Integration (2026-08-04)

Decision: the Adafruit MOSFET driver module (external, JST-PH connector) is now **replaced with an integrated MOSFET switch circuit directly on the v2 board** — Q1 (N-channel logic-level MOSFET, e.g. AO3400A, SOT-23), R2 (100R gate series resistor), R3 (10k gate pull-down, ensures the NeoPixels default OFF if D7 floats during boot/reset before firmware drives it). Same low-side-switching topology as before (V+ passes through to the rings continuously, drain switches the GND_SW return) — verified in the exported netlist that J6/J7 (NeoPixel headers) still connect to the same GND_SW net, now through Q1's drain instead of an external module's pin.

Rationale: unlike the RFID/RF modules (real antenna engineering, stay external), a MOSFET switch is basic, well-understood circuit design — integrating it removes a whole module + connector from the BOM and is low risk. Confirmed via ERC (0 errors, 208 warnings, same cosmetic categories as before, +2 from the added parts) and independent netlist inspection, not just ERC passing.

Note: Q1 is represented as a generic labeled 3-pin placeholder (SOT-23 footprint, pins named Gate/Drain/Source in the Value field) rather than a vendor-specific symbol, matching this project's established convention for parts where footprint-perfect precision is deferred to layout stage (same treatment as the Pro Mini and RF TX module). The exact SOT-23 pad-to-G/D/S mapping for the specific part chosen still needs final confirmation against its datasheet at layout time.

The PCB layout (component placement/routing) for v2 needs to be redone fresh against this updated schematic — the prior layout attempt was based on the old J4/J5 netlist and has been removed as stale.

## Barrel-Jack (Wall-Powered) Board (2026-08-04)

**Active board**, derived from the paused v2 fully-custom battery schematic. Location: `PCB/MagicBand_BarrelJack/`.

Carried forward unchanged: the ATmega328P-P (DIP-28) circuit (crystal, load caps, decoupling, reset pull-up, ISP header), MFRC522 stacking-socket connection, RF TX module wiring, the MCP1700-3302 regulator feeding the 3.3V rail for MFRC522+RF TX (its low-quiescent-current advantage is moot on wall power, but it still works fine electrically -- kept rather than force a swap for no functional reason).

Removed: the MOSFET NeoPixel-gating circuit (Q1/R2/R3, the `GND_SW` net) -- existed purely to cut NeoPixel standby current for battery life, not needed on continuous wall power. NeoPixel ring headers now wire directly to the main +5V/GND rails.

Power input: barrel jack, 5.5mm/2.1mm, center-positive, per the original wired-build spec in Technical Reference. Represented in the schematic as a generic 2-pin connector symbol (`Connector_Generic:Conn_01x02`) with Value "DC_Barrel_Jack_5.5x2.1mm" -- same deferred-footprint-precision convention already used for the Pro Mini and RF TX module placeholders, chosen deliberately after a real KiCad `Connector:Barrel_Jack` symbol swap caused a genuine short (both jack pins landed on the +5V net, GND left disconnected from the jack) due to a pin-geometry mismatch that wasn't obvious from ERC alone (0 errors even with the short present -- only caught by manually inspecting net membership in the exported netlist). Reverted rather than risk it; real barrel-jack footprint gets selected precisely at layout stage instead.

ERC-clean: 0 errors, 194 warnings (same cosmetic categories as prior boards -- grid alignment, missing library-table config).

## MCP1700 Regulator: Deliberate Keep, Not Inertia (2026-08-04, from PDR)

Independent PDR review confirmed the MCP1700-3302 is correctly wired and correctly decoupled -- but flagged that its original selling point (1.6uA quiescent current, chosen for the now-paused battery variant) is moot on wall power, and it's a somewhat pricier/more specialty SOT-23 LDO than a generic 3.3V regulator (e.g. AMS1117, which was rejected earlier this project specifically *because of* its high quiescent current -- a concern that no longer applies on wall power). Conclusion: **keeping MCP1700 is a conscious choice, not unexamined leftover inertia.** It works correctly, decoupling is right, and there's no compelling reason to churn the schematic further just to swap in a marginally cheaper part with no functional benefit. Recorded here so this reads as a decision, not an oversight.

## MFRC522 / ISP Shared-SPI-Bus Gotcha (2026-08-04, found in PDR)

MOSI/MISO/SCK are shared between the MFRC522 stacking socket (J2) and the ISP programming header (J9) -- both tap the same physical SPI bus on the ATmega328P. **Unplug the MFRC522 module before programming via ISP.** If it stays seated during a flash attempt, its presence on the bus can contend with the programmer on MISO, especially while the chip is held in reset -- a real, documented gotcha with SPI-sharing shield/stack designs, not theoretical. Worth a physical reminder label near the board, not just this doc.

## Stacking Header for MFRC522 (2026-08-04)

J2 (MFRC522 module connector on the v2 board) changed from a generic male pin header footprint to a **female socket strip** (`Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical`, verified against KiCad's official footprint repo). The MFRC522 breakout's own male header pins now plug directly into this socket, stacking the module directly onto the custom board -- like an Arduino shield -- instead of connecting via loose jumper wires. Solves the "two loose boards to mount inside the ornament" problem mechanically, without touching the RFID antenna/RF question (full chip-level MFRC522 integration was evaluated and ruled out -- see the fully-custom research section above -- real 13.56MHz antenna matching-network engineering, likely needs re-tuning once mounted, not worth the risk). Wiring/pin order unchanged, already verified correct against a real MFRC522 breakout's pinout.

Layout note for later: treat the area under/around where the MFRC522 module will sit similarly to the RF TX antenna keepout -- keep it clear of other tall components so the two boards can actually stack without interference.

## Component Selection Audit (2026-08-04)

Focused audit pass on component/connector selection for v2, to catch mismatches before layout locks in.

- **Fixed**: MOSFET driver (Adafruit PID 5648) connector was drawn as a generic 2.54mm pin header — the real board uses a 3-pin STEMMA JST-PH connector at 2mm pitch (V+/GND/In together, confirmed against Adafruit's pinout docs). Incompatible pitch, would not have mated with the real cable. Fixed to `JST_PH_B3B-PH-K_1x03_P2.00mm_Vertical`, verified against KiCad's official footprint repo.
- **Open, unverified**: Lipo Rider Plus's 5V output appears to route through its USB Type-A port rather than a simple header pin (its separate pin header explicitly outputs 3.3V, not 5V, per available docs) — J8's footprint choice on the custom board is not yet confirmed correct. Needs a direct check of Seeed's pin diagram before finalizing.
- **Confirmed adequate, no action needed**: JST-PH library footprint availability in stock KiCad (Connector_JST.pretty, official repo) — no unofficial/community libraries needed.
- **Assembly service note**: connectors (JST-PH, headers, ISP header) are through-hole and always hand-soldered regardless of JLCPCB assembly tier — the assembly-service cost question only ever applied to the one SMD MCP1700 part, which is also easy to hand-solder if skipping assembly entirely.

## Toolchain (this WSL environment)

- KiCad 10.0.5, installed via the official AppImage (apt is blocked in this sandbox — no root available), extracted to `~/.local/opt/squashfs-root`
- `kicad-cli` symlinked to `~/.local/bin/kicad-cli` — used for DRC/ERC validation and rendering exports so work can be checked without a GUI session
