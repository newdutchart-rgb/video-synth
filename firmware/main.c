/*
 * Video Synth - analoge PAL videosynthesizer
 * ATmega328P @ 16 MHz
 *
 * PD2..PD5 : 4-bit luma DAC (R-2R ladder, 16 grijstinten)
 * PD6      : sync-uitgang (1 k naar RCA; laag = sync, Hi-Z = vrij)
 * PC0..PC2 : potmeters (ADC0..ADC2) voor patroonmodulatie
 *
 * Quasi-PAL: 64 us per regel, 312 regels per veld, 50 Hz, non-interlaced.
 *
 * Patronen (RV2 / PC1 selecteert):
 *   0-3 : sinusgolven, scrollend (RV1 snelheid, RV3 fase)
 *   4-7 : klassiek testbeeld (schaakbord, grijstrap, raster, cirkel,
 *         zwart/wit-balken, diagonale ramp)
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define VIDEO_PORT  PORTD
#define VIDEO_DDR   DDRD
#define VIDEO_MASK   0x3C      /* PD2..PD5 */
#define SYNC_DDR     DDRD
#define SYNC_BIT     (1 << PD6)

#define LINES_PER_FIELD   312
#define VSYNC_LINES         5
#define ACTIVE_START       40      /* regels na vsync voor zichtbare regels */
#define ACTIVE_LINES      240

#define PIXELS              24
#define CYCLES_PER_PIXEL    32

/* sinustabel, 32 stappen, amplitude 16 (4-bit) */
static const uint8_t sin_lut[32] = {
    8, 9,11,12,13,14,15,15,
   15,15,14,13,12,11, 9, 8,
    6, 5, 3, 2, 1, 1, 0, 0,
    0, 0, 1, 1, 2, 3, 5, 6
};

/* testbeeld-cirkel: halve breedte dx bij dy (0..28), ellips a=10, b=28 */
static const uint8_t circle_lut[29] = {
   10,10,10,10,10,10,10,10,10, 9,
    9, 9, 9, 9, 8, 8, 8, 7, 7, 7,
    6, 6, 5, 5, 4, 3, 3, 2, 0
};

#define CIRCLE_CY   128     /* midden van de cirkel (band 3+4: regels 96..159) */
#define CIRCLE_XC   12      /* pixelmidden bij PIXELS=24 */

static volatile uint16_t line = 0;
static volatile uint16_t frame = 0;

/* patroonparameters, gevuld door de main loop (ADC) */
static volatile uint8_t mod_rate   = 8;   /* scrollsnelheid (RV1) */
static volatile uint8_t mod_wave   = 0;   /* golfvorm/patroonkeuze (RV2) */
static volatile uint8_t mod_phase  = 0;   /* fase/invert (RV3) */

static inline void sync_low(void)
{
    PORTD &= ~(uint8_t)SYNC_BIT;
    SYNC_DDR |= SYNC_BIT;
}

static inline void sync_release(void)
{
    SYNC_DDR &= ~(uint8_t)SYNC_BIT;   /* Hi-Z */
}

/* verticale sync: 2 brede pulsen per regel (15 us laag, 17 us hoog) */
static void emit_vsync_line(void)
{
    sync_low();
    __builtin_avr_delay_cycles(15 * 16 - 10);
    sync_release();
    __builtin_avr_delay_cycles(17 * 16);
    sync_low();
    __builtin_avr_delay_cycles(15 * 16 - 10);
    sync_release();
}

/* testbeeld: klassiek patroon in 8 horizontale banden van 30 regels */
static uint8_t testbeeld_px(uint8_t x, uint8_t y, uint8_t dxl)
{
    switch (y >> 5) {
    case 0:                                     /* schaakbord 2x2 px */
        return (((x >> 1) ^ (y >> 1)) & 1) ? 0x0F : 0x00;
    case 1:                                     /* grijstrap, 6 treden */
        return (x >> 2) * 3;
    case 2:                                     /* raster op middengrijs */
        return ((x & 3) == 0 || (y & 15) == 0) ? 0x0F : 0x08;
    case 3:                                     /* cirkel, bovenhelft */
    case 4: {                                   /* cirkel, onderhelft */
        uint8_t ax = (x < CIRCLE_XC) ? CIRCLE_XC - x : x - CIRCLE_XC;
        if (ax + 1 >= dxl && ax <= dxl + 1)
            return 0x0F;
        if (y == CIRCLE_CY && x > CIRCLE_XC - 4 && x < CIRCLE_XC + 4)
            return 0x0F;                        /* horizon lijn */
        return 0x08;
    }
    case 5:                                     /* zwart/wit-balken */
        return (x < PIXELS / 2) ? 0x0F : 0x00;
    case 6:                                     /* verticale grijstrap */
        return (y - 192) >> 1;
    default:                                    /* diagonale ramp */
        return ((x >> 1) + (y >> 1)) & 0x0F;
    }
}

/* horizontale regel: 4.7 us sync + actieve pixels */
static void emit_line(uint16_t ln)
{
    sync_low();
    __builtin_avr_delay_cycles(4 * 16);
    sync_release();
    __builtin_avr_delay_cycles(6 * 16);          /* back porch */

    uint8_t phase = (uint8_t)(ln * mod_rate >> 3) + mod_phase;
    uint8_t wave  = mod_wave;
    uint8_t y     = (uint8_t)(ln - ACTIVE_START);
    uint8_t x     = 0;

    /* per regel voorberekende cirkelbreedte (band 3/4) */
    uint8_t dy    = (y > CIRCLE_CY) ? y - CIRCLE_CY : CIRCLE_CY - y;
    uint8_t dxl   = (dy < 29) ? circle_lut[dy] : 0xFF;  /* buiten cirkel: geen outline */

    for (x = 0; x < PIXELS; x++) {
        uint8_t v;
        if (wave & 0x04)
            v = testbeeld_px(x, y, dxl);
        else {
            v = sin_lut[(x + phase) & 31];
            if (wave & 0x01) v ^= 0x0F;
            if (wave & 0x02) v = (x ^ ln) & 0x0F;
        }
        VIDEO_PORT = (VIDEO_PORT & ~VIDEO_MASK) | ((v & 0x0F) << 2);
        __builtin_avr_delay_cycles(CYCLES_PER_PIXEL);
    }
    VIDEO_PORT &= ~VIDEO_MASK;
}

ISR(TIMER1_COMPA_vect)
{
    if (line < VSYNC_LINES)
        emit_vsync_line();
    else
        emit_line(line);

    line++;
    if (line >= LINES_PER_FIELD) {
        line = 0;
        frame++;
    }
}

static void adc_init(void)
{
    ADMUX  = (1 << REFS0);                 /* AVcc, kanaal 0 */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);  /* prescaler /64 */
}

static uint8_t adc_read(uint8_t ch)
{
    ADMUX = (1 << REFS0) | (ch & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;
    return ADC >> 2;                       /* 8-bit */
}

int main(void)
{
    VIDEO_DDR |= VIDEO_MASK;
    adc_init();

    /* Timer1 CTC: 64 us per regel bij 16 MHz */
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10);   /* CTC, prescaler /1 */
    OCR1A  = 1023;
    TIMSK1 = (1 << OCIE1A);

    sei();

    uint8_t ch = 0;
    for (;;) {
        uint8_t v = adc_read(ch);
        switch (ch) {
        case 0: mod_rate  = v >> 4; break;
        case 1: mod_wave  = v >> 5; break;
        case 2: mod_phase = v;       break;
        }
        ch = (ch + 1) & 3;                 /* ADC3 onbenut -> volgende ronde */
    }
}
