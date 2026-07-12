# RF Module Pinout — D-FLIFE 433MHz TX/RX Kit

*Captured: 2026-07-12*
*Source: [Amazon listing B0BZRRBBNK](https://www.amazon.com/dp/B0BZRRBBNK) — "D-FLIFE 5pcs 433mhz Wireless RF Transmitter and Receiver with Antenna Ask Remote Control Module DIY Kit for Arduino"*
*Captured from listing images + a customer review, not a datasheet — no official datasheet found for this specific board silkscreen.*

-----

## Pack Contents

- 5× TX modules, 5× RX modules (manufactured as one panel — boards must be snapped apart)
- 5× short spring antennas (TX)
- 5× longer spring antennas (RX)

-----

## TX Pinout (5 pins)

| Pin | Function |
|---|---|
| 1 | Antenna — **short** spring antenna |
| 2 | Enable — per listing's (broken-English) description: "connected to power supply, no function" — appears tied internally, not a usable control pin |
| 3 | Data |
| 4 | Power in — **3.3V** |
| 5 | GND |

## RX Pinout (6 pins)

| Pin | Function |
|---|---|
| 1 | Antenna — **long** spring antenna |
| 2 | GND |
| 3 | VIN |
| 4 | Data out |
| 5 | Data out (duplicate — two data pins) |
| 6 | GND |

-----

## Notes from Customer Review (10+ yr tinkerer, verified via RadioHead library on 2× Uno)

- Confirmed working as described, but essentially no documentation from the manufacturer
- Boards ship as one panel — snap apart carefully before use
- RX antenna pin reads as a dead short to GND on a multimeter (5/5 units tested) — **this is normal, not a defect**
- Don't bridge the antenna pin to the adjacent GND pin despite the short reading
- **TX runs on 3.3V.** Reviewer didn't test TX at 5V — treat as 3.3V-only until proven otherwise
- **RX only confirmed working at 5V** — reviewer didn't get it working at 3.3V
- Range covers a full basement (unquantified, informal)

-----

## Implication for the Wireless Ornament Build

The wireless-variant power architecture doesn't currently account for a device that needs 3.3V, other than the MFRC522 (which already runs off the Pro Mini's onboard 3.3V pin). If this specific module is the one that gets sourced for the wireless variant's TX:

- **TX shares the 3.3V rail with the MFRC522** — not the raw 5V bus assumed elsewhere in the BOM/firmware comments (`MagicBand_Wireless.ino` currently only documents the TX data pin, not its power pin, which needs correcting once this is confirmed as the sourced part)
- Add TX's active-transmit current draw to the 3.3V rail budget alongside the MFRC522's, not the 5V rail
- RX is bench-only tooling (Sniffer step) — 5V requirement is a non-issue there since the Sniffer setup runs on a full Uno/Nano at 5V anyway

Not yet folded into `BOM - Wireless Ornament Variant.md` or the firmware's pin-map comment — flagging here first since this is a capture, not a confirmed BOM change.
