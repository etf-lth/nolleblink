CC      =msp430-gcc
OBJCOPY =msp430-objcopy
SIZE    =msp430-size

MCU     =msp430f2274

#CFLAGS  +=-I../../msp-common/
CFLAGS +=-Wall -g

TARGET  =nolleblink12.hex
OBJS    =main.o spi.o led.o radio.o flash.o ledfont.o uart.o

all: $(TARGET)

clean:
	rm -f *.o

veryclean: clean
	rm -f $(TARGET)

debug:
	mspdebug rf2500 "gdb"

flash: $(TARGET)
	mspdebug rf2500 "prog $(TARGET)"

reset:
	mspdebug rf2500 "reset"

size: $(TARGET)
	$(SIZE) $(TARGET).o

$(TARGET): $(OBJS)
	$(CC) -mmcu=$(MCU) -o $@.o $^
	$(SIZE) $@.o
	$(OBJCOPY) -O ihex $@.o $@

.c.o:
	$(CC) -c -mmcu=$(MCU) $(CFLAGS) -o $@ $^
