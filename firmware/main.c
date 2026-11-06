/*
 * Video Synth - analoge PAL videosynthesizer
 * ATmega328P @ 16 MHz
 *
 * PD2..PD5 : 4-bit luma DAC (R-2R ladder, 16 grijstinten)
 * PD6      : sync-uitgang (1 k naar RCA; laag = sync, Hi-Z = vrij)
 * PC0..PC2 : potmeters (ADC0..ADC2) voor patroonmodulatie
 *
 * Quasi-PAL: 64 us per regel, 312 regels per veld, 50 Hz, non-interlaced.
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

#define PIXELS              44
#define CYCLES_PER_PIXEL    17

/* sinustabel, 32 stappen, amplitude 16 (4-bit) */
static const uint8_t sin_lut[32] = {
    8, 9,11,12,13,14,15,15,
   15,15,14,13,12,11, 9, 8,
    6, 5, 3, 2, 1, 1, 0, 0,
    0, 0, 1, 1, 2, 3, 5, 6
};

static volatile uint16_t line = 0;
static volatile uint16_t frame = 0;

/* patroonparameters, gevuld door de main loop (ADC) */
static volatile uint8_t mod_rate   = 8;   /* scrollsnelheid (RV1) */
static volatile uint8_t mod_wave   = 0;   /* golfvormmix (RV2) */
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

/* verticale sync: brede pulsen (2 x 30 us laag per regel) */
static void emit_vsync_line(void)
{
    sync_low();
    __builtin_avr_delay_cycles(30 * 16 - 40);
    sync_release();
    __builtin_avr_delay_cycles(34 * 16);
    sync_low();
    __builtin_avr_delay_cycles(30 * 16 - 40);
    sync_release();
}

/* horizontale regel: 4.7 us sync + actieve pixels */
static void emit_line(uint16_t ln)
{
    sync_low();
    __builtin_avr_delay_cycles(4 * 16);
    sync_release();
    __builtin_avr_delay_cycles(6 * 16);          /* back porch */

    uint8_t phase  = (uint8_t)(ln * mod_rate >> 3) + mod_phase;
    uint8_t wave   = mod_wave;
    uint8_t x = 0;

    if (ln >= ACTIVE_START && ln < ACTIVE_START + ACTIVE_LINES) {
        for (x = 0; x < PIXELS; x++) {
            uint8_t v = sin_lut[(x + phase) & 31];
            if (wave & 0x01) v ^= 0x0F;
            if (wave & 0x02) v = (x ^ ln) & 0x0F;
            if (wave & 0x04) v = ((x & 8) ^ (ln & 8)) ? 0x0F : 0x00;
            VIDEO_PORT = (VIDEO_PORT & ~VIDEO_MASK) | ((v & 0x0F) << 2);
            __builtin_avr_delay_cycles(CYCLES_PER_PIXEL);
        }
        VIDEO_PORT &= ~VIDEO_MASK;
    } else {
        VIDEO_PORT &= ~VIDEO_MASK;          /* blanking: black-niveau */
    }
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
    ADMUX  = (1 << REFS0);                 /* AVcc, kanal 0 */
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
