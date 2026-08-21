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
| C3, C4, C_AREF | Decoupling | 100nF, 0603 SMD | Generic — assorted kit covers this |
| C5 | Bulk decoupling | 1uF, 0805 SMD | Generic |
| C6 | Bulk decoupling | 10uF, 0603 SMD | Generic |
| C7 | Decoupling | 1.0uF, 0603 SMD | Generic |
| MH1-3 | Mounting | No separate hardware — 4.2mm holes sized for the ornament shell's own heat-stake bosses (see `3. Work Packages.md`, WP3.1) | Nothing to buy unless the shell design changes |

-----

## Notes

- **C1/C2 and C6's package sizes are intentionally mismatched from what you might expect from a "matched pair" or "bigger = bulk cap" assumption** — this reflects what's actually routed on the PCB (verified 2026-08-21, see Session Log), not an error. C1=0603/C2=0805 for the crystal caps, C6=0603 for the 10uF bulk cap. Buy to this table, not to intuition.
- **Generic passives** (R1/R2, C3/C4/C5/C7/C_AREF) are common enough values/packages that a general assorted 0603+0805 SMD resistor/capacitor kit is likely more practical than sourcing each individually — cross-check kit contents against the values above before assuming full coverage.
- Prices intentionally omitted for most rows above — pull current pricing at order time rather than trust a number captured here that will drift.
