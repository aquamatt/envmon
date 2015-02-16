# this F_CPU gets carried through into the i2c library also
F_CPU = 8000000
BAUD = 38400
TEST = 0
MCU = atmega8a
AVRDUDE_MCU = atmega8
ISP = usbasp

#CFLAGS += -DDEBUG=1
CFLAGS += -DF_CPU=$(F_CPU)UL \
		  -DTEST=$(TEST) \
		  -DBAUD=$(BAUD)UL \
		  -mmcu=$(MCU) \
		  -Wall -Os

COMPILER = avr-gcc $(CFLAGS)
# options required if doing float operations and printing floats. Not
# recommended if you can avoid it.
FLOAT_OPTS =  -Wl,-u,vfprintf -lprintf_flt -lm
HEXER = avr-objcopy -j .text -j .data -O ihex
AVRDUDE = avrdude -c $(ISP) -p $(AVRDUDE_MCU)
I2CDEPS = i2c/twimaster.o
OBJS = sensors.o \
	   bmp180.o \
	   serial.o \
	   mydisplay.o

all: sensors.hex

clean:
	rm -f *.o *.hex *.elf

reallyclean: clean
	cd i2c; $(MAKE) -f makefile.twimaster clean

i2c/twimaster.o:
	cd i2c; $(MAKE) -f makefile.twimaster F_CPU=$(F_CPU) MCU=$(AVRDUDE_MCU)

%.o: %.c
	$(COMPILER) -o $@ -c $<

sensors.elf: $(OBJS) $(I2CDEPS)
	$(COMPILER) -o $@ $(OBJS) $(I2CDEPS)

%.hex: %.elf
	$(HEXER) $< $@

deploy: sensors.hex
	sudo $(AVRDUDE) -U flash:w:$<

fuseclock:
	# set external, high frequency oscilator, long SUT (ATMEGA8A)
	sudo $(AVRDUDE) -U lfuse:w:0xff:m -U hfuse:w:0xd9:m

fusedefault:
	# reset to ATMEGA8A defaults
	sudo $(AVRDUDE) -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m
