CC      =msp430-gcc
OBJCOPY =msp430-objcopy
SIZE    =msp430-size

MCU     =msp430x2274

#CFLAGS  +=-I../../msp-common/
CFLAGS +=-Wall -g

TARGET  =nolleblink10.hex
OBJS    =main.o spi.o led.o radio.o flash.o ledfont.o

MOJT_PROGRAMMER  =CDFF5D7F0B143D59
RADIO_PROGRAMMER =BFFF5D7F0B140F4C

MOJT_FLAGS  =-U `../../../usbfind/usbfind 0451 f432 $(MOJT_PROGRAMMER)`
RADIO_FLAGS =-U `../../../usbfind/usbfind 0451 f432 $(RADIO_PROGRAMMER)`

all: $(TARGET)

clean:
	rm -f *.o

veryclean: clean
	rm -f $(TARGET)

debug:
	mspdebug rf2500 $(MOJT_FLAGS) "gdb"

flash: $(TARGET)
	mspdebug rf2500 $(MOJT_FLAGS) "prog $(TARGET)"

flash_radio: $(TARGET)
	mspdebug rf2500 $(RADIO_FLAGS) "prog $(TARGET)"

reset:
	mspdebug rf2500 $(MOJT_FLAGS) "reset"

reset_radio:
	mspdebug rf2500 $(RADIO_FLAGS) "reset"

devices:
	../../../usbfind/usbfind 0451 f432 ?

size: $(TARGET)
	$(SIZE) $(TARGET).o

$(TARGET): $(OBJS)
	$(CC) -mmcu=$(MCU) -o $@.o $^
	$(SIZE) $@.o
	$(OBJCOPY) -O ihex $@.o $@

.c.o:
	$(CC) -c -mmcu=$(MCU) $(CFLAGS) -o $@ $^
