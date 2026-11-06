MCU      = atmega328p
FREQ     = 16000000UL
AVRDUDE_PROG = usbasp

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(FREQ) -Os -Wall -Wextra -std=c11
TARGET = video-synth

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

$(TARGET).elf: main.c
	$(CC) $(CFLAGS) -o $@ $<

flash: $(TARGET).hex
	avrdude -c $(AVRDUDE_PROG) -p m328p -U flash:w:$(TARGET).hex:i

fuse:
	avrdude -c $(AVRDUDE_PROG) -p m328p -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

clean:
	rm -f $(TARGET).elf $(TARGET).hex
