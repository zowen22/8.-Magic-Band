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
| U1 | MCU | **ATMEGA328P-PU**, DIP-28 ([Digikey](https://www.digikey.com/en/products/detail/microchip-technology/ATMEGA328P-PU/1914589)) | Official Microchip part, in stock, ships same-day. Also grab a generic **28-pin DIP IC socket (0.3"/300mil)** — cheap, widely available, keeps the chip pullable/programmable outside the board per the design's whole rationale for choosing DIP |
| Y1 | Crystal | **Euroquartz 16.000MHz HC49-4H/30/50/-40+85/18PF/ATF** ([Farnell 1640875](https://uk.farnell.com/euroquartz/16-000mhz-hc49-4h-30-50-40-18pf-atf/crystal-16mhz-18pf-through-hole/dp/1640875) / [Newark 40T4490](https://www.newark.com/euroquartz/16-000mhz-hc49-4h-30-50-40/crystal-16mhz-18pf-through-hole/dp/40T4490)) | **Resolved 2026-08-21** — see `PCB Design Plan.md` SS1.3. THT, low-profile HC49-4H can, 18pF CL |
| U2 | 3.3V regulator | **MCP1700T-3302E/TT**, SOT-23-3 ([Digikey 652676](https://www.digikey.com/en/products/detail/microchip-technology/MCP1700T-3302E-TT/652676)) | **Package suffix matters** — `MCP1700-3302E/TO` (no "T" prefix) is TO-92, not SOT-23. This board's footprint is SOT-23, so it has to be the `/TT` suffix specifically. Feeds MFRC522 + RF TX module's 3.3V rail |
| Q1 | Reverse-polarity MOSFET | **AO3401A**, SOT-23-3 ([Digikey, UMW brand](https://www.digikey.com/en/products/detail/umw/AO3401A/16705973), or original Alpha & Omega Semiconductor branded part) | Pinout already independently verified against the real datasheet image (see `PCB Design Plan.md` SS3.2) — any genuine "AO3401A" branded SOT-23-3 part from a reputable distributor is pin-compatible |
| J8 | Barrel jack | 5.5mm/2.1mm, **center-positive**, threaded panel-mount, pre-wired ([e.g. DaierTek pre-wired panel-mount jack](https://www.amazon.com/5-5x2-1MM-Pre-Wired-Connector-Waterproof-Appliances/dp/B0BD46CP5Y)) | **Not board-mounted** — bare flying leads hand-soldered to J8's 2 THT pads (see `PCB Design Plan.md` SS1.5). A pre-wired jack (leads already attached) is less assembly work than a solder-terminal one. **Center pin = +5V, sleeve = GND** — verify polarity on whatever you buy before wiring, this board's own history includes catching a real reverse-polarity risk from a mismatched symbol |
| J2 | RFID reader | MFRC522 module, any standard breakout (widely sold on Amazon — HiLetgo, SunFounder, etc.) | **Confirmed 2026-08-21: standard module size is 40x60mm** — the overhang risk flagged in `PCB Design Plan.md` SS1.2 is real, not hypothetical, for a 40x40mm board. Not a functional problem (it stacks as a shield via the 1x08 socket, doesn't need to fit *within* the board outline) but the module will physically overhang by ~20mm in one dimension — factor that into shell/enclosure clearance |
| J3 | RF transmitter | D-FLIFE 433MHz ASK TX module ([Amazon B0BZRRBBNK](https://www.amazon.com/dp/B0BZRRBBNK), 5-pack) | **Already sourced/owned** — this is the exact module used on the bench across the RF debugging sessions (`PULSE_LEN=613` confirmed working against the real target outlet). No further sourcing needed |
| J6 | NeoPixel LED strip | WS2812B, 5V, count/length TBD (e.g. [BTF-LIGHTING 60/m](https://www.amazon.com/BTF-LIGHTING-WS2812B5M60LB30-BTF-LIGHTING-WS2812B-IC-RGB-5050SMD-Pure-Gold-Individual-Addressable-LED-Strip-High-Quality-16-4FT-300LED-60LED-m-Flexible-Full-Color-IP30-DC5V-for-DIY-Chasing-Color-Project-No-Adapter-or-Controller/dp/B01CDTEJBG), cut to length) | **Still needs a length/count decision** (flagged since 2026-08-06, still open). **Connector gap found 2026-08-21**: most raw strips ship with bare wire ends or a JST-SM connector, neither of which mates with J6's JST-PH 2.0mm footprint. Need a separate [JST-PH 3-pin pigtail](https://www.amazon.com/Micro-Connector-150mm-Cable-Female/dp/B01DUC1PW6) to crimp/solder onto the strip's cut end, or a strip specifically sold with a JST-PH terminated lead |
| J9 | ISP header | 2x3 shrouded IDC box header, 2.54mm — generic, any standard part | Only needed while flashing firmware, not a permanent functional part. Low-stakes, any keyed 2x3 shroud works |
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

**Fab-placeable (18 refs)**: R1, R2, C1-C8 (all passives), Q1, U2 (SMD ICs), Y1 (crystal, THT), U1 (MCU, THT), J2/J3/J6/J9 (all connectors/sockets/headers, THT). JLCPCB and similar services support mixed SMT+THT assembly in one order (SMD via reflow, THT via wave/selective soldering) — confirmed via their own docs, not assumed.

**Cannot be fab-placed, stays hand-assembly regardless**:
- **J8 (barrel jack)** — no real part to place. It's bare THT pads by design (see SS1.5 in `PCB Design Plan.md`), so this is the one connector with genuinely nothing for a fab to populate. Marked `DNP` (Do Not Populate) in the schematic and PCB 2026-08-21 specifically so an assembly order doesn't try to place a generic 2-pin header there — confirmed via BOM/CPL diff that it now excludes correctly.
- **The modules themselves** (MFRC522 board, RF TX module, NeoPixel strip) — these plug into J2/J3/J6 *after* the board comes back from assembly. A fab can place the socket/header/connector, but has no way to source or place external modules it doesn't manufacture. This is true regardless of assembly vs. hand-solder.

**U1 — socket vs. direct chip is a real open choice, not yet decided**: same THT footprint either way (no board difference), but a real tradeoff. Direct chip placement is more hands-off (nothing left for you to insert) but means a bad joint is a desolder job instead of a two-second socket swap — the whole reason DIP was chosen over TQFP in the first place. Check actual cost delta between the two at your fab's live BOM quote tool (not something reliably web-searchable — depends on their parts catalog/quantity pricing in real time).

**Files generated 2026-08-21** (in `PCB/MagicBand_BarrelJack/`, committed to the repo alongside the board files — regenerate from the `.kicad_sch`/`.kicad_pcb` if the board changes rather than trusting a stale copy):
- `MagicBand_BarrelJack_BOM.csv` — grouped by value+footprint, includes a DNP column (J8 flagged)
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
