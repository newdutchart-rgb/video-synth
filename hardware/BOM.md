# BOM — Video Synth

| Ref | Onderdeel | Waarde / type | Aantal |
|-----|-----------|---------------|--------|
| U1  | ATmega328P-PU | microcontroller (DIP-28) | 1 |
| U2  | TL074 | quad opamp (DIP-14) | 1 |
| U3  | LM7805 | linear regulator (TO-220) | 1 |
| X1  | Kristal | 16 MHz (HC-49) | 1 |
| D1  | Diode | 1N4007 | 1 |
| C1, C2 | Keramisch | 22 pF | 2 |
| C3  | Elektrolyt | 470 µF / 16 V | 1 |
| C4  | Elektrolyt | 100 µF / 16 V | 1 |
| C5–C7 | Keramisch | 100 nF | 3 |
| R1–R4 | Weerstand 20 kΩ (2R, R-2R ladder) | 0207 | 4 |
| R11 | Weerstand 20 kΩ (ladder-terminator) | 0207 | 1 |
| R5  | Weerstand 1 kΩ (sync) | 0207 | 1 |
| R6  | Weerstand 10 kΩ (DAC → summer) | 0207 | 1 |
| R7  | Weerstand 75 Ω (video-serie, naar RCA) | 0207 | 1 |
| R8–R10 | Weerstand 10 kΩ (ladder-rungs) | 0207 | 3 |
| R12–R14 | Weerstand 100 kΩ (CV-ingangen) | 0207 | 3 |
| R18 | Weerstand 10 kΩ (reset-pullup) | 0207 | 1 |
| RV1–RV3 | Potentiometer 10 kΩ lineair | 3386P | 3 |
| J1  | DC-jack | 2.1 mm barrel | 1 |
| J2  | RCA-chassisbus | geel (composite out) | 1 |
| J3–J5 | 3.5 mm jack | mono (CV-in) | 3 |
| J6  | Pinheader | 2×3 (ISP) | 1 |
| SW1 | Drukknop | 6×6 mm (reset) | 1 |
| —   | IC-sokkels | DIP-28 + DIP-14 | 1 + 1 |

Voeding: 9–12 V DC, minimaal 100 mA, pluspunt in de jack (D1 beveiligd tegen omgekeerde polariteit).
