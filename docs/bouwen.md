# Bouwhandleiding

## 1. Printplaat

1. Open `hardware/kicad/video-synth.kicad_pro` in KiCad 8.
2. Controleer het schema (`video-synth.kicad_sch`): alle netten hebben labels;
   voer ERC uit.
3. Open `video-synth.kicad_pcb`: outline (75 × 58 mm) en de
   volledige componentplaatsing staan erin. Route de sporen met de
   interactieve router (sneltoets X), 0.3 mm signaal, 0.8 mm voeding.
4. Groundplane: teken een fill-zone op B.Cu verbonden met GND.

Single-sided is haalbaar; route de paar lastige netten (SYNC, V0–V3) op de
onderkant of gebruik een jumperdraad.

## 2. Assemblage

1. Soldeer de lage onderdelen eerst (weerstanden, diode, IC-sokkels).
2. Daarna condensatoren, kristal, potmeters, jacks, DC-connector.
3. Gebruik sokkels voor U1 en U2 (niet direct solderen).
4. Sluit aan: 9–12 V DC-adapter (pluspunt binnen), gele RCA naar TV
   (composite-in), CV-bron naar J3–J5 (optioneel).

## 3. Firmware

```bash
cd firmware
make
make flash      # USBasp; zie Makefile voor andere programmers
```

Nadat de bootloader niet nodig is: flash via ISP-header J6.

## 4. Gebruik

- **RV1** (PC0): patroonfrequentie / scrollsnelheid
- **RV2** (PC1): golfvorm / patroonkeuze-mix
- **RV3** (PC2): fase / invertie-mix
- Externe CV (0–5 V, audio of LFO) op J3–J5 moduleert de helderheid mee.

## 5. Kalibratie / problemen

- Geen sync: controleer of PD6 tijdens sync laag wordt (scope of multimeter op
  de RCA-tip; ~15.6 kHz blokgolfflank).
- Te donker/beeld: verlaag R7 niet onder 75 Ω; controleer de TL074-voeding
  (pin 4 = VIN, pin 11 = GND).
- TL074 werkt vanaf ±5 V single-supply; bij 9 V in is de swing ruim voldoende
  voor 0–2 V video.
