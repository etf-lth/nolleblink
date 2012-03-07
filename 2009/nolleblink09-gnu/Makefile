CC      =msp430-gcc
OBJCOPY =msp430-objcopy

MCU     =msp430x2274

CFLAGS  +=-I../msp-common/

TARGET  =nolleblink09.hex
OBJS    =main.o radio.o leddisplay.o

all: $(TARGET)

clean:
	rm -f *.o

veryclean: clean
	rm -f $(TARGET)

flash: $(TARGET)
	mspdebug "prog $(TARGET)"

reset:
	mspdebug "reset"

$(TARGET): $(OBJS)
	$(CC) -mmcu=$(MCU) -o $@.o $^
	$(OBJCOPY) -O ihex $@.o $@

.c.o:
	$(CC) -c -mmcu=$(MCU) $(CFLAGS) -o $@ $^