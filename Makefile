TARGET  = test
CC		= avr-gcc
HEX		= avr-objcopy
OPT		= -Os -mmcu=attiny85 -DF_CPU=16500000
AVR_INCLUDE 	= /usr/lib/avr/include
DIGI_INCLUDE	= ../DigistumpArduino/digistump-avr/cores/tiny
WIRING_SRCS 	= 						\
	$(DIGI_INCLUDE)/wiring.c 			\
	$(DIGI_INCLUDE)/wiring_pulse.c		\
	$(DIGI_INCLUDE)/wiring_shift.c		\
	$(DIGI_INCLUDE)/wiring_digital.c 	\
	$(DIGI_INCLUDE)/pins_arduino.c
USB_SRCS 		= 	\
	usb/usbdrvasm.S	\
	usb/usbdrv.c	\
	usb/osccal.c	\
	usb/oddebug.c

INCLUDES = -I. -Iusb -I$(AVR_INCLUDE) -I$(DIGI_INCLUDE)
SRCS =	$(USB_SRCS) 	\
		$(WIRING_SRCS)	\
		main.c			\
		joycon.c

all:
	$(CC) $(OPT) -o $(TARGET) $(INCLUDES) $(SRCS)
	$(HEX) -R .eeprom -O ihex $(TARGET) $(TARGET).hex

.PHONY: clean hex flash
clean:
	-rm $(TARGET)*

hex:
	avr-objcopy -R .eeprom -O ihex $(TARGET) $(TARGET).hex

flash:
	../commandline/micronucleus --run $(TARGET).hex
