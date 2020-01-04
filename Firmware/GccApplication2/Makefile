MCU=atmega8
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-std=c99 -Wall -g -Os -mmcu=${MCU} -I.
TARGET=flash
EEPROM=eeprom
SRCS=GccApplication2.c i2c_master.c i2c_master.h

all:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex

flash:
	avrdude -p ${MCU} -c usbasp -P usb  -B 375 -e -u  -U flash:w:${TARGET}.hex:i  -D -U eeprom:w:${EEPROM}.eep:i -U lfuse:w:0x24:m -U hfuse:w:0xD9:m

install:
	sudo apt install gcc-avr avr-libc avrdude	
	
clean:
	rm -f *.bin *.hex

