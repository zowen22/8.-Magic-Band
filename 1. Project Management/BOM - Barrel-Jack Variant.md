# BOM — Barrel-Jack (Wall-Powered) Variant

*Created: 2026-08-21*
*Status: Active build. Board: `PCB/MagicBand_BarrelJack/`.*

-----

## What This Is

Sourcing list for the current active board — wall-powered via barrel jack, no battery. Every part below is either confirmed against a real, purchasable listing (with source link) or flagged as still generic/needs-a-decision. Nothing on this board ships pre-assembled — see `4. Technical Reference.md` / Session Log 2026-08-21 for why (DIP-28 MCU chosen specifically to stay hand-solderable).

-----

## Bill of Materials

| Ref | Component | Part | Notes |
|---|---|---|---|
| U1 | MCU | **ATMEGA328P-PU**, DIP-28 — fab-placed direct (no socket) | Official Microchip part, JLCPCB C33901, in stock. **Decided 2026-09-21: direct chip placement, not socketed** (see Assembly Service section) — no separate DIP socket to buy, nothing for you to solder here at all |
| Y1 | Crystal | **Euroquartz 16.000MHz HC49-4H/30/50/-40+85/18PF/ATF** ([Farnell 1640875](https://uk.farnell.com/euroquartz/16-000mhz-hc49-4h-30-50-40-18pf-atf/crystal-16mhz-18pf-through-hole/dp/1640875) / [Newark 40T4490](https://www.newark.com/euroquartz/16-000mhz-hc49-4h-30-50-40/crystal-16mhz-18pf-through-hole/dp/40T4490)) | **Resolved 2026-08-21** — see `PCB Design Plan.md` SS1.3. THT, low-profile HC49-4H can, 18pF CL |
| U2 | 3.3V regulator | **MCP1700T-3302E/TT**, SOT-23-3 ([Digikey 652676](https://www.digikey.com/en/products/detail/microchip-technology/MCP1700T-3302E-TT/652676)) | **Package suffix matters** — `MCP1700-3302E/TO` (no "T" prefix) is TO-92, not SOT-23. This board's footprint is SOT-23, so it has to be the `/TT` suffix specifically. Feeds MFRC522 + RF TX module's 3.3V rail |
| Q1 | Reverse-polarity MOSFET | **AO3401A**, SOT-23-3 ([Digikey, UMW brand](https://www.digikey.com/en/products/detail/umw/AO3401A/16705973), or original Alpha & Omega Semiconductor branded part) | Pinout already independently verified against the real datasheet image (see `PCB Design Plan.md` SS3.2) — any genuine "AO3401A" branded SOT-23-3 part from a reputable distributor is pin-compatible |
| J8 | Barrel jack | 5.5mm/2.1mm, **center-positive**, threaded panel-mount, pre-wired ([e.g. DaierTek pre-wired panel-mount jack](https://www.amazon.com/5-5x2-1MM-Pre-Wired-Connector-Waterproof-Appliances/dp/B0BD46CP5Y)) | **Not board-mounted** — bare flying leads hand-soldered to J8's 2 THT pads (see `PCB Design Plan.md` SS1.5). A pre-wired jack (leads already attached) is less assembly work than a solder-terminal one. **Center pin = +5V, sleeve = GND** — verify polarity on whatever you buy before wiring, this board's own history includes catching a real reverse-polarity risk from a mismatched symbol |
| J2 | RFID reader | MFRC522 module, any standard breakout (widely sold on Amazon — HiLetgo, SunFounder, etc.) | **Direct-solder plan, decided 2026-09-21** — no header/socket hardware needed at all; the module's own pins solder straight through J2's existing 8 through-holes for a permanent joint, per @user's preference for a more secure connection over a plug-in header. Nothing separate to buy for the board side. **Confirmed 2026-08-21: standard module size is 40x60mm** — the overhang risk flagged in `PCB Design Plan.md` SS1.2 is real, not hypothetical, for a 40x40mm board. Not a functional problem but the module will physically overhang by ~20mm in one dimension — factor that into shell/enclosure clearance |
| J3 | RF transmitter | D-FLIFE 433MHz ASK TX module ([Amazon B0BZRRBBNK](https://www.amazon.com/dp/B0BZRRBBNK), 5-pack) | **Already sourced/owned** — this is the exact module used on the bench across the RF debugging sessions (`PULSE_LEN=613` confirmed working against the real target outlet). **Direct-solder plan, decided 2026-09-21** — same as J2: module's own pins solder straight through J3's existing 4 through-holes, no separate header hardware needed |
| J6 | NeoPixel LED strip | WS2812B, 5V, count/length TBD (e.g. [BTF-LIGHTING 60/m](https://www.amazon.com/BTF-LIGHTING-WS2812B5M60LB30-BTF-LIGHTING-WS2812B-IC-RGB-5050SMD-Pure-Gold-Individual-Addressable-LED-Strip-High-Quality-16-4FT-300LED-60LED-m-Flexible-Full-Color-IP30-DC5V-for-DIY-Chasing-Color-Project-No-Adapter-or-Controller/dp/B01CDTEJBG), cut to length) | **Still needs a length/count decision** (flagged since 2026-08-06, still open). **Connector gap found 2026-08-21**: most raw strips ship with bare wire ends or a JST-SM connector, neither of which mates with J6's JST-PH 2.0mm footprint. Need a separate [JST-PH 3-pin pigtail](https://www.amazon.com/Micro-Connector-150mm-Cable-Female/dp/B01DUC1PW6) to crimp/solder onto the strip's cut end, or a strip specifically sold with a JST-PH terminated lead |
| J9 | ISP header | **XFCN BH254V-6P** ([JLCPCB/LCSC C492441](https://jlcpcb.com/partdetail/XFCN-BH254V6P/C492441)) | **Resolved 2026-09-05** — real shrouded 2x3 IDC box header, JLCPCB catalog part, THT, 2.54mm pitch, matches `IDC-Header_2x03_P2.54mm_Vertical` exactly. Fab-placed, nothing to buy separately. Visually confirmed by @user: keyed slot on one long edge. See "JLCPCB Live Part-Matching Session" below for the mating cable plan |
| R1, R2 | Resistors | 10k, 0603 SMD | Generic value/package — an assorted 0603 resistor kit covers this plus spares |
| C1, C2 | Load caps | 27pF, 0603 (C1) / 0805 (C2) | **Note the mismatched packages** (SS3 below) — buy one of each size, not a matched pair of the same package |
| C3, C4, C8 | Decoupling | 100nF, 0603 SMD | Generic — assorted kit covers this |
| C5 | Bulk decoupling | 1uF, 0805 SMD | Generic |
| C6 | Bulk decoupling | 10uF, 0603 SMD | Generic |
| C7 | Decoupling | 1.0uF, 0603 SMD | Generic |
| MH1-3 | Mounting | No separate hardware — 4.2mm holes sized for the ornament shell's own heat-stake bosses (see `3. Work Packages.md`, WP3.1) | Nothing to buy unless the shell design changes |

-----

## Notes

- **C1/C2 and C6's package sizes are intentionally mismatched from what you might expect from a "matched pair" or "bigger = bulk cap" assumption** — this reflects what's actually routed on the PCB (verified 2026-08-21, see Session Log), not an error. C1=0603/C2=0805 for the crystal caps, C6=0603 for the 10uF bulk cap. Buy to this table, not to intuition.
- **Generic passives** (R1/R2, C3/C4/C5/C7/C8) are common enough values/packages that a general assorted 0603+0805 SMD resistor/capacitor kit is likely more practical than sourcing each individually — cross-check kit contents against the values above before assuming full coverage.
- Prices intentionally omitted for most rows above — pull current pricing at order time rather than trust a number captured here that will drift.

-----

## Assembly Service (PCBA) — What Can Be Fab-Placed

User's call (2026-08-21): use a PCBA/assembly service for everything that can be, minimize hand-assembly to what genuinely can't be fab-placed.

**Revised 2026-08-25**: while matching parts during the actual JLCPCB BOM upload, user decided J2 and J3 should be hand-soldered rather than fab-placed — both are mechanically-loaded connectors (modules get plugged/unplugged, cables tugged), and solder-joint reliability there matters more than assembly convenience. J6 (JST-PH, keyed housing) and J9 (shrouded IDC header, keyed) stay fab-placed — no similar mechanical-stress/reliability concern since their mating cables are pre-built and keyed, not friction-fit or hand-wired.

**Revised again 2026-09-21 (morning)**: user tried moving J2 (MFRC522 socket) back to fab-placed. The mechanical-reliability reasoning from 2026-08-25 was a preference, not a technical blocker (mixed THT assembly always supported it fine), and a plain 1x8 female 2.54mm socket is genuinely generic. Un-flagged `dnp` on J2 in both the schematic and PCB, verified ERC/DRC unchanged (129/7, 0 unconnected). Real catalog part found: **2.54MM 1X8P RED, JLCPCB C9900138233**, Extended tier, THT, 1x8/2.54mm — confirmed matches `PinSocket_1x08_P2.54mm_Vertical` exactly.

**Reversed back the same day**: C9900138233 hit a part shortage. A well-stocked fallback was found and verified (**BOOMELE 2.54-1×8P, JLCPCB/LCSC C27438**, confirmed female 1x8/2.54mm/THT, 128,200 in stock, $0.10-0.11/unit — kept here for reference in case fab-placing J2 is ever reconsidered), but once the shortage came up, @user reconsidered and decided to just hand-solder **both** J2 and J3 rather than keep chasing/re-verifying fab-placed sockets for connectors that only exist to receive a plug-in module anyway. J2 flagged `dnp yes` again in both the schematic and PCB (exact reverse of the morning's change), ERC/DRC re-verified unchanged (129/7, 0 unconnected — third verification pass on this same flag today, same baseline every time), `MagicBand_BarrelJack_BOM.csv` regenerated with J2 back to `DNP`. **Net result: J2 and J3 are both hand-solder, same as the original 2026-08-25 decision** — today's excursion into fab-placing J2 is documented for the record but the board ends up right back where it started.

**Fab-placeable (16 refs)**: R1, R2, C1-C8 (all passives), Q1, U2 (SMD ICs), Y1 (crystal, THT), U1 (MCU, THT — direct chip, not socketed, decided 2026-09-21), J6/J9 (connectors, THT — keyed/pre-built mating cables). JLCPCB and similar services support mixed SMT+THT assembly in one order (SMD via reflow, THT via wave/selective soldering) — confirmed via their own docs, not assumed.

**Cannot be fab-placed, stays hand-assembly regardless**:
- **J8 (barrel jack)** — no real part to place. It's bare THT pads by design (see SS1.5 in `PCB Design Plan.md`), so this is the one connector with genuinely nothing for a fab to populate. Marked `DNP` (Do Not Populate) in the schematic and PCB 2026-08-21 specifically so an assembly order doesn't try to place a generic 2-pin header there — confirmed via BOM/CPL diff that it now excludes correctly.
- **The modules themselves** (MFRC522 board, RF TX module, NeoPixel strip) — these plug into J2/J3/J6 *after* the board comes back from assembly. A fab can place the socket/header/connector, but has no way to source or place external modules it doesn't manufacture. This is true regardless of assembly vs. hand-solder.

**Hand-assembly by choice, not necessity**:
- **J2 (MFRC522 socket)** and **J3 (RF TX header)** — both `DNP` in the schematic and PCB. Both *could* be fab-placed (mixed THT assembly supports them fine, and a real catalog part exists for J2 — see reversal note above), but @user settled on hand-soldering both.
- **Clarified 2026-09-21 (later): no separate connector hardware needed for either, at all.** @user's actual plan is to solder each module's own pins **directly through the board's existing through-holes** for a permanent, more secure joint — not populate a header/socket component that the module then plugs into. A female-socket footprint and a male-header footprint use identical hole size/pitch (both sized for a standard 2.54mm pin), so J2's and J3's existing pads work as a direct pass-through with zero PCB/footprint changes needed. This eliminates the "generic header strip not yet purchased" gap entirely — there's nothing left to buy for J2 or J3's board-side hardware. Only real prerequisite: confirm the MFRC522 module and D-FLIFE TX module actually have straight male pins (8-pin and 4-pin respectively) when they arrive, not bare leads or an unusual pin form — standard breakout modules almost always do. **J9 is explicitly NOT part of this** — it stays a real shrouded connector (not direct-solder) since it's repeatedly plugged/unplugged across multiple boards via the Nano ISP cable, not a permanent module attachment.

**U1 — decided 2026-09-21: direct chip placement, not socketed.** Same THT footprint either way (no board difference either way), but a real tradeoff was on the table: direct placement is more hands-off, socketing preserves the rework-without-desoldering benefit that was the original reason DIP was chosen over TQFP. User confirmed direct chip placement is fine — the live JLCPCB BOM export (2026-09-21) already reflects this, matching U1 straight to the ATMEGA328P-PU chip (C33901), no socket in the order.

**Files generated 2026-08-21** (in `PCB/MagicBand_BarrelJack/`, committed to the repo alongside the board files — regenerate from the `.kicad_sch`/`.kicad_pcb` if the board changes rather than trusting a stale copy):
- `MagicBand_BarrelJack_BOM.csv` — grouped by value+footprint, includes a DNP column (J2, J3, J8 flagged as of 2026-08-25 — see revised assembly-service section above)
- `MagicBand_BarrelJack_CPL.csv` — placement positions/rotations for every non-DNP part, ready to upload alongside the BOM to a quote tool. **Format is JLCPCB-specific, not kicad-cli's raw output** — see note below, first upload attempt failed on this exact file
- `MagicBand_BarrelJack_Gerbers.zip` — Gerber X2 (F/B Cu, Paste, Silkscreen, Mask, Edge.Cuts) + separate PTH/NPTH Excellon drill files, all at the zip root (not nested in a subfolder — a common upload gotcha). This is the bare-board fab order's file, uploaded first/separately from the BOM/CPL

**Found and fixed while preparing these files** (not new since they existed before, just previously undetected):
- **C_AREF renamed to C8** — this reference was never run through KiCad's normal annotation flow (non-numeric suffix), which the BOM exporter flagged as an annotation warning. Real risk if left alone: BOM (schematic-derived) and CPL (PCB-derived) reference strings have to match exactly for an assembly house's placement system to correlate them — an unusual designator isn't guaranteed to survive that round-trip cleanly. Fixed in both files, verified via ERC/DRC diff (129/7, exact baseline both times).
- One edit attempt broke the schematic's parenthesis balance (a dropped closing paren) — caught immediately via a balance check before trusting the result, fixed, re-verified. No bad state was ever exported or committed.

**CPL upload failure, fixed 2026-08-21**: `kicad-cli pcb export pos`'s raw CSV output (`Ref, Val, Package, PosX, PosY, Rot, Side`) is not JLCPCB's expected format and the first upload attempt failed. User supplied JLCPCB's own sample template (`JLCSMT_Sample_CPL1.xlsx`, downloaded from their site, not committed to this repo) to diagnose against. JLCPCB expects exactly **`Designator, Mid X, Mid Y, Layer, Rotation`**, with:
- Coordinates as `mm`-suffixed strings (e.g. `14.0000mm`), not bare numbers
- **Y coordinate negated** from kicad-cli's raw export — KiCad's position file uses a Y-down convention, JLCPCB's is Y-up. Confirmed correct (not just following a forum post) by checking the negated values all land inside the board's actual 0-40mm bounds, matching the sample template's all-positive values
- `Layer` capitalized (`Top`/`Bottom`, not `top`/`bottom`)
- `Rotation` normalized to 0-360 (kicad-cli emits e.g. `-90` for U2, converted to `270`)
- Extra columns (`Val`, `Package`) dropped — not part of JLCPCB's expected format

`MagicBand_BarrelJack_CPL.csv` now has the corrected format directly (kicad-cli's raw output was post-processed via a script, not committed separately). Cross-checked against the community-documented conversion (same renames, same Y-negation, same mm-suffix convention) rather than trusting the sample template alone.

**Second upload failure, fixed 2026-08-21**: JLCPCB flagged "J8 designator don't exist in the CPL file." The first fix pass had (correctly per KiCad convention, incorrectly per JLCPCB's actual parser) excluded J8 from the CPL entirely via `--exclude-dnp`, on the assumption DNP status only needs to live in the BOM. **JLCPCB's uploader instead expects every BOM designator to have a matching CPL row** -- DNP is signaled by the BOM's DNP column, not by the part's absence from the CPL. Regenerated the CPL without `--exclude-dnp` (all 19 refs, J8 included at its real board position) and re-ran the same format conversion. J8's DNP flag stays set in the BOM only -- that's still the correct place for it, this was purely a "did it need to also appear in the CPL" question, not a DNP-marking question.

-----

## Pre-Matched BOM — Skipping the Flaky Auto-Matcher (2026-09-21)

**Problem found**: re-uploading a fresh `MagicBand_BarrelJack_BOM.csv` (prompted by investigating a scattered JLCPCB placement-preview render) reset every prior manual "Select by Customer" part choice back to raw "Select by System" auto-matching. The auto-matcher **regressed to the exact same wrong-package parts already caught and rejected weeks earlier** — U2 back to `MCP1700-3302E/TO` (TO-92, wrong mount type for our SOT-23 footprint) and Y1 back to an SMD crystal (wrong mount type for our THT footprint) — and failed to auto-match J6, J9, and Q1 at all ("No matches"). This is what actually caused the scattered 3D render: wrong-package 3D models get rendered at otherwise-correct CPL coordinates, which looks like chaos even though the underlying position data was never wrong (independently re-verified fresh from the PCB and confirmed byte-identical to the committed CPL before this was diagnosed).

**Fix**: JLCPCB's BOM uploader supports an explicit **`JLCPCB Part #`** column (confirmed via their own BOM File Format Guide — required headers are `Designator`, `Comment`, `Footprint`, `JLCPCB Part #`) that pre-selects an exact catalog part by C-number and skips automatic matching entirely for that row. Built **`MagicBand_BarrelJack_BOM_prematched.csv`** with every confirmed C-number locked in for all 16 non-DNP designators (not just the two that regressed — locked all of them down, since today showed the auto-matcher can't be trusted to stay put across re-uploads):

| Ref | JLCPCB Part # |
|---|---|
| C1 | C107045 |
| C2 | C1808 |
| C3,C4,C8 | C14663 |
| C5 | C28323 |
| C6 | C19702 |
| C7 | C15849 |
| J6 | C158004 |
| J9 | C492441 |
| Q1 | C15127 |
| R1,R2 | C25804 |
| U1 | C33901 |
| U2 | C41381676 |
| Y1 | C295075 |

J2/J3/J8 left with a blank part number (DNP, no part should ever be matched to them).

**Use `MagicBand_BarrelJack_BOM_prematched.csv` for future uploads instead of the plain `_BOM.csv`** to avoid this regression recurring. It's a separate, hand-maintained file (not regenerated by `kicad-cli sch export bom`, since that tool has no concept of a JLCPCB part number) — **if the schematic ever changes** (new part, different footprint, a ref renumbered), this file needs its C-number mapping re-checked/updated by hand, it won't pick up changes automatically the way `_BOM.csv` does.

-----

## JLCPCB Live Part-Matching Session (2026-09-02/04)

While matching parts during the actual JLCPCB BOM upload, two of the "Select by System" auto-matches were real footprint mismatches, not just cosmetic warnings -- caught by cross-checking JLCPCB's matched-part footprint column against our own BOM's footprint column, part by part, rather than trusting the automated match:

- **U2 (MCP1700-3302, needs SOT-23)**: system auto-matched `MCP1700-3302E/TO`, a **TO-92-3** part -- completely different mount type. Genuine Microchip `MCP1700T-3302E/TT` (the correct SOT-23 suffix, already specified above) turned out to be out of stock at JLCPCB. Evaluated 3 clone alternatives by manufacturer + footprint field before picking: `MCP1700T-3302E/TT(MS)` (MSKSEMI) and `TPMCP1700T-3302E/TT` (TECH PUBLIC, footprint listed as `SOT-23L` -- ambiguous, not verified) were passed over in favor of **`HMCP1700T-3302E/TT`, HXY MOSFET, JLCPCB C41381676** -- footprint field is plain `SOT-23` (matches our footprint exactly, confirmed against the actual `SOT-23.kicad_mod` pad geometry: 1.9mm pin1-pin2 pitch, 1.875mm row span), and specs line up with the original part (3.3V fixed positive LDO, 250mA output, 1.5µA quiescent vs. genuine part's ~1.6µA, -40 to +85°C).
- **Y1 (16MHz, needs THT `Crystal_HC49-4H_Vertical`)**: system auto-matched `NX2520SA-16MHZ-STD-CSW-5`, an `SMD2520-4P` part -- SMD vs. our THT footprint, not placeable at all. Replacement found and verified against its actual datasheet (user supplied the TAITIEN PDF, not just the JLCPCB listing text): **`XIHCELNANF-16MHZ`, TAITIEN Elec, JLCPCB C295075**, `HC-49S` package. Datasheet confirms 10.8 x 4.5 x 3.4mm body, **4.88mm pin pitch** -- exact match to our footprint's pad coordinates (pulled directly from `Crystal_HC49-4H_Vertical.kicad_mod`: pads at (0,0) and (4.88,0)). Functionally: 16MHz (exact match, required for firmware timing), 20pF rated load capacitance vs. the 18pF our C1/C2 (27pF each) were sized around -- a small load-cap mismatch that pulls resonant frequency by a few ppm at most, comfortably inside this part's own ±30ppm tolerance; not worth re-sizing the caps over. -40 to +85°C and drive-level/ESR specs both comfortably compatible with the ATmega328P's oscillator circuit.
- **C7, R1/R2, U1** "Unconfirmed" warnings reconfirmed as false positives (JLCPCB flagging its own matched manufacturer part number string against our BOM's plain value/generic-name comment, not an actual electrical or footprint mismatch) -- safe to accept as-is.
- **J2, J3, J8** correctly show no match (`DNP`, hand-soldered by design, nothing to place).
- **J6, J9 resolved 2026-09-05** — real catalog matches found for both, closing out the last open item before checkout:
  - **J6 (JST-PH 3-pin, `JST_PH_B3B-PH-K_1x03_P2.00mm_Vertical`)**: our footprint already has the real manufacturer part number baked into its name. **JST B3B-PH-K(LF)(SN), JLCPCB C565982**, Extended tier, "Plugin" (THT), confirmed 1x3P / 2mm pitch / PH-series — exact match, genuine part, not a clone.
    - **2026-09-05 update: C565982 hit a part shortage.** @user found a real substitute: **JST B3B-PH-KL(LF)(SN), JLCPCB C158004** — same PH-series 3-pin/2mm-pitch/THT housing family, footprint-compatible. Confirmed the only actual difference is the "L" suffix, a documented JST variant meaning **low insertion force** (easier to plug/unplug the NeoPixel strip's mating connector, marginally less vibration resistance than plain "K") — a reasonable tradeoff for a connector mated by hand, not a compromise. 7,435 in stock on LCSC as of this check, listed on JLCPCB. **Use C158004 if C565982 is still out of stock at order time**, otherwise either works.
  - **J9 (shrouded 2x3 IDC, `IDC-Header_2x03_P2.54mm_Vertical`)**: earlier attempts (this session and 2026-09-02/04) kept surfacing unshrouded parts, wrong pin counts, or too-low-stock dead ends via general web search — JLCPCB's search UI is JS-rendered and doesn't return results through a plain page fetch, so this took searching by category/naming-convention instead (LCSC's "IDC Connectors" category specifically, vs. the "Pin Headers, Male" category the bad candidates kept coming from). Found **XFCN BH254V-6P, JLCPCB/LCSC C492441** — 2x3P, 2.54mm pitch, THT, 25,160+ in stock. Visually confirmed by @user against the real part photo: genuine box header with a keyed slot on one long edge.
  - **Mating cable plan (for repeated multi-board ISP flashing, not just this one board)**: user is flashing multiple boards via a spare Arduino Nano running the ArduinoISP sketch, not a dedicated USBasp. Buy one generic 6-pin 2x3 IDC ribbon cable with female sockets on both ends (e.g. [uxcell 5-pack](https://www.amazon.com/uxcell-2-54mm-Pitch-Female-Ribbon/dp/B013FMQ3VY) or [this JTAG/ISP-labeled cable](https://www.amazon.com/2x3-Pin-IDC-JTAG-Cable/dp/B00PGT7WGA)) — cut the connector off one end, strip and solder those 6 wires directly to the Nano's D10/D11/D12/D13/5V/GND pins once; the other end's factory female IDC socket plugs onto every board's J9 going forward, keyed so it can't go in misaligned. See `4. Technical Reference.md`'s new ISP Programming section for the full pin mapping and wiring gotchas.
