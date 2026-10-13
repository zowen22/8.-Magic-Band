# MagicBand_BarrelJack — Revision 2

*Created: 2026-10-13. Copied from `PCB/MagicBand_BarrelJack/` (rev 1, fab-ready, two boards fabricated/assembled and in bring-up) — that folder stays untouched/frozen as the as-fabricated reference. This folder is where rev 2 design changes happen.*

Only the KiCad source files were copied (`.kicad_pro/.kicad_sch/.kicad_pcb/.kicad_prl`) — not rev 1's BOM/CPL/gerbers/SVGs/DRC-ERC reports, since those are rev 1-specific fab outputs that would be misleading here until rev 2 actually has changes and gets its own export pass.

## Scope (decided 2026-10-13)

Cleanup pass on the existing wall-powered architecture **plus** a real second-LED-strip connector. **Not** a revisit of the paused battery-powered/IRQ-wake variant — that stays paused pending its own separate decision (see `2. Project Overview.md` Decisions Log, 2026-08-04 entry).

## Backlog — see `3. Work Packages.md` WP4.3 for the authoritative, maintained list

Summary as of creation (check WP4.3 directly for current status, don't trust this snapshot):
1. Silkscreen: per-pin signal labels near J2/J3/J8/J9
2. J6 wire-color documentation/sourcing fix (pigtail kit colors inverted from NeoPixel convention)
3. J6 pin order vs. actual LED strip pad order
4. J8 through-hole size vs. actual barrel-jack wire gauge
5. **New, real connector for second LED strip** (rev 1 only has a hand-bodged wire to U1's D2 leg + a J8 power tap — see `Build Instructions - Barrel-Jack Variant.md` Step 5a — rev 2 should give this its own header + proper trace sizing, not a bodge)
6. +5V trace width to the LED strip(s) — rev 1's 0.2mm trunk sizing was a judgment call, not computed from hard math; revisit with real numbers once rev 2's actual LED part (a new, smaller/lower-draw 2.7mm-width LED, BOM link pending from @user) is confirmed, especially now that it's feeding two strips instead of one
7. RF antenna/J3 keepout — only fix if board 1/2 testing actually shows reduced 433MHz range, don't fix pre-emptively
8. Cosmetic-only: R1/Y1 silkscreen overlap, 3 starved-thermal pour-island flags — bundle in only if doing a placement pass anyway

## What's NOT done yet

No schematic/PCB edits have been made in this folder — it's an exact copy of rev 1 as of 2026-10-13. Placement/routing/silkscreen work in this project has always been a GUI + user session (see `4. Technical Reference.md`'s FreeRouting/placement workflow), not something done headlessly — next step is a live KiCad session against this folder once the second-strip connector's footprint/pin-out is picked.
