# Video Synth — analoge PAL-videosynthesizer

Een open, bouwbare videosynthesizer die composite PAL-video genereert en
moduleerbaar is met potmeters en externe control-voltage-ingangen (CV).

## Architectuur

Hybride ontwerp:

| Blok | Functie |
|------|---------|
| **ATmega328P @ 16 MHz** | Genereert PAL-sync (4,7 µs per regel, 64 µs regelfrequentie, 15.625 Hz) en 4-bit luminantie-patronen |
| **4-bit R-2R ladder (PD2–PD5)** | 16 grijstinten, direct uit de MCU-poort |
| **TL074 opamps** | Sommeren van DAC-signaal + 3 externe CV-ingangen, buffer naar 75 Ω |
| **Sync-uitgang (PD6, 1 kΩ)** | Klassieke weerstandsmenging: trekt de videolijn naar sync-niveau (onder black) |
| **LM7805 + DC-jack** | Voeding 9–12 V DC in, 5 V logica |
| **3 potmeters (PC0–PC2)** | ADC-modulatie van patronen (fase, snelheid, golfvorm) |

```
  DC-jack 9-12V ──► LM7805 ──► +5V
                                   │
  16MHz xtal ──► ATmega328P ──► PD2..PD5 ──► R-2R ladder ──┐
                                   │                      ├──► TL074A (som) ─► TL074B (inverter) ─► 75Ω ─► RCA (video)
  CV-jack 1..3 ──► 100k ──────────┼──────────────────────┘
                                   │
  Pot 1..3 ──► PC0..PC2 (ADC)      └──► PD6 ──► 1k ──────────────────────────────────────────────► RCA (sync)
```

## Signaal (PAL, progressive)

- 64 µs per regel (15.625 kHz), 312 regels per veld, 50 Hz — non-interlaced
  ("quasi-PAL", zoals TVout en de meeste hobby-videosynths; praktisch elke TV
  en capture-kaart accepteert dit).
- Sync: 4,7 µs actief laag per regel; vereenvoudigde vertical sync.
- Luma: 16 niveaus (4-bit DAC), 0 V (black) t/m ~2 V (white) bronweerstand 75 Ω.

## Repository-inhoud

```
hardware/kicad/   KiCad 8 project (.kicad_pro / .kicad_sch / .kicad_pcb)
hardware/BOM.md   Componentenlijst
firmware/         AVR C-firmware (avrdude-flashbaar)
docs/             Blokschema en bouwhandleiding
```

## Firmware bouwen en flashen

```bash
cd firmware
make            # avr-gcc build
make flash      # avrdude via USBasp; pas AVRDUDE_PROG in Makefile aan
```

Patronen worden gemoduleerd door de drie potmeters: frequentie/scrollsnelheid,
golfvorm/patroonkeuze en fase/invertie (zie docs/bouwen.md).

**Ingebouwd klassiek testbeeld** (RV2 volledig open): schaakbord, grijstrap,
raster, cirkel met horizonlijn, zwart/wit-balken, verticale grijstrap en
diagonale ramp — een knipoog naar het oude omroep-testbeeld, nu in 24×240
pixels en 16 grijstinten.

## Printplaatroostering

De `.kicad_pcb` bevat de outline en de volledige componentplaatsing
(DIP-onderdelen, 2.54 mm raster, single-sided routeerbaar). Route de sporen in
KiCad met de interactieve router (X-toets); alle netten staan in het schema.

## Licentie

MIT — vrij te bouwen, aan te passen en te delen.
