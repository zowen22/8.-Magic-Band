# PCB Design Plan

*Created: 2026-08-04 · Last reorganized: 2026-08-04*

Claude drives KiCad directly (writing/validating actual project files via `kicad-cli` DRC/ERC checks and rendered exports) rather than walking through GUI steps — user's explicit preference. This doc is organized by topic, not chronologically — see `5. Session Log.md` for the chronological blow-by-blow.

-----

## 1. Active Board: Barrel-Jack (Wall-Powered)

**Status: Schematic complete, ERC-clean, survived two independent adversarial design reviews. Layout (placement + routing + DRC) is next.**

Location: `PCB/MagicBand_BarrelJack/MagicBand_BarrelJack.kicad_sch` + `Arduino/MagicBand_BarrelJack/MagicBand_BarrelJack.ino`.

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
| JST-PH 3-pin (J6) | NeoPixel LED strip connector | Real footprint (`JST_PH_B3B-PH-K_1x03_P2.00mm_Vertical`). Value renamed to `NeoPixel_LED_Strip` (was `NeoPixel_Outer_16px`) since count is TBD pending the ordered strip. **J7 (was second/inner-ring connector) removed 2026-08-06** -- user is sourcing a single LED strip only, no second ring. Removed from both schematic (ERC-verified 0 errors) and PCB (DRC-verified 0 unconnected pads, 0 footprint errors, also cleared 2 dangling track stubs left routed to J7's old position). Confirmed side benefit: resolved the MH3/J7 courtyard conflict (only MH3/J6 remains). |
| 6-pin AVR ISP header (J9) | Programming | Standard 2x3, shares SPI bus with MFRC522 -- see gotcha below |
| Crystal (16MHz) + 2x27pF load caps | MCU clock | **Resolved 2026-08-21.** Part chosen: Euroquartz `16.000MHz HC49-4H/30/50/-40+85/18PF/ATF` (Farnell 1640875 / Newark 40T4490), 2-pin THT, low-profile HC49-4H can (10.77x4.34x3.5mm, matches this board's `Crystal_HC49-4H_Vertical` footprint), 18pF rated load capacitance, +-30ppm tolerance. Load caps sized from that rating: `C = 2 x (CL - Cstray)`, Cstray assumed 3-5pF (standard estimate for a simple 2-layer THT crystal circuit) -> 26-30pF computed range, **27pF** (C1/C2) chosen as the nearest standard E24 value. Schematic updated, ERC unchanged (129, exact baseline -- value-only edit) |
| Decoupling caps, reset pull-up (R1) | Standard support circuitry | Real, standard values, datasheet-verified |

### 1.4 Physical layout constraints -- do not violate at layout stage

**Board orientation convention (confirmed 2026-08-12, corrects a wrong assumption in earlier reviews):** the board carries physical silkscreen labels for orientation -- **"Front" near X=2mm** (i.e. **low X = front, high X = back**) and **"Right" near Y=2mm** (i.e. **low Y = right, high Y = left**). Two separate adversarial PDRs (2026-08-04 through 2026-08-11) assumed front/back mapped to the Y-axis instead -- that's wrong; front/back is the **X**-axis. A third PDR (2026-08-12) flagged J2 (RFID socket) as being centered rather than at the front based on that wrong Y-axis assumption; re-checked against the real convention, **J2 (X≈4mm, near zero) is correctly at the front and always was** -- that finding was a false positive from the axis mixup, not a real placement problem. Use X for front/back checks, not Y, in any future review.

- **RFID (MFRC522, stacking socket) must sit at the FRONT of the ornament** -- the interactive/tap end, since that's physically where the reader needs to be for a tap gesture to work.
- **Dropped (2026-08-13): the barrel jack (J8) does not need to be at the back.** J8 is actually placed at the front (X≈9.5-12mm) -- a fourth adversarial PDR (2026-08-12) found this contradicts the constraint above, same "never re-checked after the axis-convention fix" pattern as the U1 and J2 findings before it. On review, the underlying rationale doesn't hold: the barrel jack's leads come off the board *normal* (perpendicular) to it, not in-plane, so pad position on the board doesn't determine lead routing distance across the panel the way this constraint assumed. Explicitly dropped, same treatment as the U1 decision above -- J8 stays where it is.
- **Dropped (2026-08-12): the MCU (U1) does not need to be paired with the RFID socket at the front.** The original version of this constraint bundled "RFID + MCU" together as one stack requiring front placement. A second adversarial PDR (2026-08-11) found this was never actually being respected -- U1's DIP-28 package spans nearly the full board depth and its anchor point was at the *back*, and the design plan's own self-report in 1.7 claiming otherwise was simply wrong (never genuinely verified). Rather than relayout to force compliance, the user reviewed it and found no compelling reason for the MCU-specific half of the constraint, and explicitly dropped it -- U1 is now centered for maximum edge clearance instead. The RFID-at-front requirement above is unaffected and still stands.
- **MFRC522 stacks directly onto the board** (shield-style, plugs into J2's female socket) -- same electrical wiring as a wired connection, just mechanical: solves "two loose boards to mount" without touching the RFID antenna (chip-level MFRC522 integration was evaluated and rejected -- real 13.56MHz antenna-matching engineering, not worth the risk -- see 3.4). **Assembly note**: solder the plugged-in pins after seating, don't rely on socket friction alone, for a permanent joint (user's explicit preference, since the board is sealed inside the ornament forever).
- **RF antenna keepout**: 3-5mm copper/ground-plane clearance immediately around the TX module and its antenna; ~10mm general component clearance beyond that. Place the TX module at a board edge with short, direct traces; keep other components on the opposite side of the board.
- **MFRC522 module footprint clearance**: treat the area under/around where it sits like the antenna keepout -- no other tall components underneath, so the two boards can actually stack flush.
- **MFRC522/ISP shared-SPI-bus gotcha**: MOSI/MISO/SCK are shared between J2 (MFRC522) and J9 (ISP header). **Unplug the MFRC522 before programming via ISP** -- its presence on the bus can contend with the programmer on MISO while the chip is held in reset. Worth a physical reminder label on the assembled board, not just this doc.

### 1.5 Power input: bare-lead barrel jack, not a connector
Represented in the schematic as plain 2-pin THT pads (`J8`, Value `PWR_IN_BarrelJack_BareLeads`), not a purchased connector or a board-mounted jack part. The physical barrel jack mounts through the ornament's rear panel with its own flying leads, hand-soldered to these pads. A real KiCad `Barrel_Jack` library symbol was tried and rejected -- its pin geometry didn't match the intended wiring and produced a genuine short (both pins landing on +5V, GND left disconnected) that ERC didn't catch (see 3.2). The generic-pads approach avoids that risk entirely and matches how the jack is actually going to be assembled.

### 1.6 Regulator: MCP1700-3302, a deliberate keep
Originally chosen for its 1.6uA quiescent current (battery-life optimization). That advantage is moot now that the board is wall-powered, and it's a somewhat pricier/more specialty SOT-23 LDO than a generic 3.3V regulator. Kept anyway: it's correctly wired, correctly decoupled (10uF in / 1.0uF out, matching Microchip's datasheet minimum for output-cap stability -- this specific value was itself a bug caught and fixed early on, see 3.1), and there's no functional benefit to churning the design just to swap in a marginally cheaper part. Recorded here so this reads as a conscious tradeoff, not unexamined inertia.

-----

## 1.7 PCB Layout: 40x40mm target MET on placement, DRC not yet clean

**Board outline: 40mm x 40mm -- the aggressive target was hit.** All 20 components (U1 ATmega328P-P through J9 ISP header) placed with zero courtyard overlaps, fully within the 40x40mm outline. Verified programmatically (courtyard-rectangle overlap check across every component pair), not just visually.

Two footprint swaps were needed to make 40x40mm achievable: R1 and R2 (simple resistors) and C5 (bulk cap) were originally assigned bulky THT footprints (a 10.16mm-pitch axial resistor body, a 5mm THT disc cap) inconsistent with every other passive on the board already being small SMD parts. Swapped to SMD (R_0603, C_0805) for consistency and to reclaim real board area -- same electrical function, no schematic/wiring change, ERC re-confirmed clean after each swap (still 0 errors, 209 warnings).

**Placement constraints confirmed respected** *(CORRECTED 2026-08-12 -- this was wrong)*: this originally claimed MFRC522 socket (J2) + MCU (U1) were both at the front (low-Y) end, barrel jack (J8) at the back (high-Y) end. That was never actually verified against the file and turned out to be false for U1 -- a second adversarial PDR (2026-08-11) computed U1's real courtyard and found it spanning nearly the entire board depth, anchored at the *back*, in the same zone as J8. See the 1.4 update above: the MCU-at-front half of this constraint has since been explicitly dropped by the user rather than fixed, so this is no longer a compliance gap -- just a correction to a previously false claim. J2 (RFID) at the front is accurate. **J8 (barrel jack) at the back was NOT accurate** -- it's actually at the front too, per PDR #4 (2026-08-12). See the 1.4 update above: this constraint has since been explicitly dropped as well, for the same reason the leads exit normal to the board rather than in-plane. Both J2 and U1 findings above were genuinely re-verified this time (not just asserted); this document's self-reported claims should not be trusted again without independent re-checking, given three of the last four rounds of review have caught this doc asserting something false about placement compliance.

**DRC status: this section is stale, predates the FreeRouting/manual-routing work done 2026-08-09 through 2026-08-12.** The 139-violation scripted-routing state described below was superseded long ago. Current status: board is DRC-clean except 3 pre-existing cosmetic footprint-library warnings (see `5. Session Log.md`, 2026-08-12 entry, for the current/authoritative state -- this doc is not kept live for DRC numbers). Original text preserved below for the historical record of *why* scripted point-to-point routing was abandoned in favor of FreeRouting:

139 violations remained (86 solder-mask-bridge, 29 tracks-crossing, 19 shorting-items, 5 clearance, 1 unconnected), down from an initial naive pass's 240. Root cause, diagnosed not guessed: routing was done via scripted point-to-point/nearest-neighbor tracks (`pcbnew` Python API, no netlist-autorouter available in this headless environment), which cannot reliably route *around* nearby unrelated pads -- especially THT pads (like J2's and U1's pins), which physically span both copper layers, so a trace on either layer can coincide with them. This is a genuine limitation of scripted point-to-point routing on a board this component-dense, not a placement or constraint-compliance problem. GND is routed as a copper pour (zone) rather than point-to-point, which is standard practice and worked well; +5V/+3V3/signal nets are the ones still producing violations.

**What this means practically**: the placement is sound and validated; the *routing* needs either (a) a real autorouter (e.g., export a Specctra DSN and run it through FreeRouting, then re-import), or (b) manual routing in KiCad's GUI, which wasn't available in this headless session. Do not treat the current `.kicad_pcb`'s routing as fab-ready -- it's a real, saved, DRC-checked starting point (not a placeholder), but genuinely not done.

Files: `PCB/MagicBand_BarrelJack/MagicBand_BarrelJack.kicad_pcb`, `drc_report.txt`, `board_top.svg`, `board_bottom.svg`.

## 1.8 Layout Review Fixes (2026-08-05) -- real progress, DRC still not clean

Closed findings from the adversarial placement/routing review, worked by hand-editing the `.kicad_pcb` directly (no `pcbnew` module available in this lite AppImage install -- confirmed, not assumed). Honest status per item:

- **RF antenna keepout: genuinely fixed, not just re-labeled.** The review found the old keepout didn't even touch J3's footprint and had no F.Cu presence at all. Replaced with a real keepout on BOTH layers, sized to J3's actual courtyard (34.2,1.2)-(37.8,9.85) + 4mm margin -> region (30.2,0)-(40,13.85). Rerouted every track segment found crossing through it as an unrelated corridor (+3V3's 20mm run at x=36, two D11/D12 crossings, two +5V crossings) so they now exit the zone directly rather than travel through it. Verified by geometry inspection, not just DRC (DRC doesn't understand RF requirements). Known remaining limitation: the GND copper pour's *stored fill polygon* still overlaps the enlarged keepout -- its underlying rule now correctly excludes that area, but no headless zone-refill tool exists in this environment (no `pcbnew`, no GUI) to regenerate the stored fill to match. Needs a GUI "Refill All Zones" pass before this is fully resolved on the copper itself.
- **J9 (ISP header) repositioning: real placement win, routing implementation didn't capture it.** Traced U1's actual SPI pins precisely (via its 180°-rotation math, cross-checked against existing trace endpoints) and found them on the board's LEFT side near J2 -- meaning J9's original position (far right, diagonally opposite J2) was placed on the wrong side of the board entirely, not just "a bit far." Moved J9 from (35.5,33) to (10.5,10), directly adjacent to both J2 and U1's SPI pins. **However**: reconnecting the 5 nets touching J9 required routing around its own tightly-packed 2x3 pin footprint in a ~7mm-wide corridor between J2 and U1, and the detour paths needed added more length than the shorter straight-line distance saved -- actual measured result: D11/D12/D13 now run 69.9-74.9mm each, *longer* than the original 51-59mm, not shorter. The placement is genuinely better; the specific hand-routed implementation of the reconnection is not. This needs a real router (or manual GUI routing) to actually realize the benefit of the improved placement.
- **Mounting holes: added, sized for heat-staking per user's build method.** 3x NPTH holes (not 4 -- see below), 4.2mm diameter (oversized vs. standard M3 clearance ~3.2-3.4mm, per user's request for heat-stake boss flow margin), at (3,37), (37,37), (37,20). This diameter is a placeholder pending the ornament shell's actual boss dimensions (WP2.2, still open) -- flag for revision once that's designed. Only 3 holes fit with reasonable clearance: the board is dense enough at 40x40mm that a 4th corner (top-left, near J2's stacking socket) didn't have room without the hole sitting too close to J2's own footprint. DRC now reports some `hole_clearance`/`npth_inside_courtyard` findings (13 and 3 respectively) from the remaining tight spots -- real, not spurious, and will need final position tuning once routing is actually finished (hole positions were chosen against the current sparse/partial routing, not a final routed board).
- **Silkscreen reference designators: un-hidden, all 20.** Addresses the review's real hand-assembly-error risk (multiple visually-identical parts with no visible labels). Did not manually reposition each label to avoid every possible collision given time constraints -- DRC now reports some `silk_over_copper`/`silk_overlap` findings (12 and 5) from default label placement overlapping nearby copper/silkscreen. Real, needs a cleanup pass, but the core fix (labels exist and are visible) is done.
- **MFRC522/Y1 clearance: checked, lower risk than initially flagged.** J2 (MFRC522 socket) and Y1 (crystal) are 28.5mm apart center-to-center -- J2 sits at the board's far-left edge, Y1 sits centrally-right. Given a stacked MFRC522 module's header naturally faces away from the board's other components (toward the ornament's front/tap face), a genuine physical conflict with Y1 seems unlikely, but this can't be fully confirmed until the real module's dimensions and mounting orientation are known. Documented, not resolved -- same "measure once purchased" treatment as the RF TX module's footprint.

**Net DRC status after this pass: 202 violations (up from 139), and that increase is real, not a regression to hide.** Breakdown: 101 solder-mask-bridge, 27 shorting-items, 20 tracks-crossing, 13 hole-clearance, 12 silk-over-copper, 11 clearance, 5 silk-overlap, 4 items-not-allowed (keepout-related, see above), 3 npth-inside-courtyard, 3 lib-footprint-issues, 2 track-dangling, 1 silk-edge-clearance. The board genuinely has more real, physical work done on it (keepout, mounting holes, silkscreen, improved-but-not-yet-realized J9 placement) than before, but routing completeness has not improved and local density from the added holes/labels introduced new friction. **This board is further from fab-ready on a violation-count basis than the previous checkpoint, despite every individual fix being real** -- worth knowing plainly before deciding what happens next, rather than reading fewer completed checkboxes as more done.

**What's still needed, unchanged in kind from before**: a real autorouter (Specctra DSN -> FreeRouting -> re-import) or manual GUI routing. Also newly needed: a GUI zone-refill pass (ground pour), and manual silkscreen label repositioning to clear the new collisions.

Files: `board_top.svg`, `board_bottom.svg`, `drc_report.txt` (all re-exported/updated).

## 1.9 Handoff to GUI for Routing (2026-08-05)

**FreeRouting (headless autorouting) is not viable in this environment** -- confirmed, not just untried. Java + FreeRouting itself work fine (portable JRE installable without root), but `kicad-cli pcb export` has no Specctra DSN export subcommand in KiCad 10.0.5 -- that export is GUI-only (File -> Export -> Specctra DSN), so there's no way to hand a board to an autorouter headlessly here. User is doing final routing themselves in a real KiCad GUI session.

**Current DRC state at handoff: 202 violations.** Categorized so this isn't a cold, unexplained number when the GUI opens it:

*Will very likely resolve once real routing replaces the current scripted point-to-point tracks* -- not real design problems, artifacts of the routing method: `solder_mask_bridge` (101), `shorting_items` (27), `tracks_crossing` (20), `clearance` (11), `track_dangling` (2), `hole_clearance` (13).

*Needs explicit attention, won't auto-resolve from routing alone*:
- `npth_inside_courtyard` (3) -- **confirmed real**, not a false alarm: MH1 (3,37) overlaps J8's (barrel jack) courtyard, MH3 (37,20) overlaps both J6 and J7's (NeoPixel JST connectors) courtyards. Only MH2 is genuinely clear. Nudge MH1/MH3 visually in the GUI -- deliberately not fixed blindly via headless coordinate edit, this file has already had enough coordinate-math mistakes from that approach and a 40x40mm board benefits from visual placement judgment here.
- `silk_over_copper` / `silk_overlap` / `silk_edge_clearance` (18 total) -- leftover label-position collisions from un-hiding all 20 reference designators. Quick manual drags in the GUI.
- `items_not_allowed` (4) -- checked one instance: a 29.5mm `+3V3` track still sits inside the RF antenna keepout zone despite an earlier reroute attempt. **Matters for RF performance, not just DRC** -- explicitly re-verify nothing crosses that keepout zone before finalizing routing there.
- `lib_footprint_issues` (3) -- cosmetic, missing local footprint-library-table reference for "MountingHole." Doesn't affect the physical board.

**Do this early in the GUI session, before trusting what the ground copper looks like**: the GND zone's stored fill is stale relative to the antenna keepout zone added this round (no headless "refill zones" tool exists here) -- run a **Refill All Zones** pass first.

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
