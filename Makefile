F_CPU = 8000000UL
BAUD = 9600UL
TEST = 0
MCU = atmega8a
ISP = usbasp
AVRDUDE_MCU = atmega8

CFLAGS += -DF_CPU=$(F_CPU) \
		  -DTEST=$(TEST) \
		  -DBAUD=$(BAUD)\
		  -mmcu=$(MCU) \
		  -Wall -Os
COMPILER = avr-gcc $(CFLAGS)
# options required if doing float operations and printing floats. Not
# recommended if you can avoid it.
FLOAT_OPTS =  -Wl,-u,vfprintf -lprintf_flt -lm
HEXER = avr-objcopy -j .text -j .data -O ihex
AVRDUDE = avrdude -c $(ISP) -p $(AVRDUDE_MCU)
I2CDEPS = i2c/twimaster.o
OBJS = temp.o \
	   bmp180.o \
	   serial.o \
	   mydisplay.o

all: temp.hex

clean:
	rm -f *.o *.hex *.elf
	cd i2c; $(MAKE) -f makefile.twimaster clean

i2c/twimaster.o:
	cd i2c; $(MAKE) -f makefile.twimaster

%.o: %.c
	$(COMPILER) -o $@ -c $<

temp.elf: $(OBJS) $(I2CDEPS)
	$(COMPILER) -o $@ $(OBJS) $(I2CDEPS)

%.hex: %.elf
	$(HEXER) $< $@

deploy: temp.hex
	sudo $(AVRDUDE) -U flash:w:$<

fuseclock:
	# set external, high frequency oscilator, long SUT (ATMEGA8A)
	sudo $(AVRDUDE) -U lfuse:w:0xff:m -U hfuse:w:0xd9:m

fusedefault:
	# reset to ATMEGA8A defaults
	sudo $(AVRDUDE) -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m
