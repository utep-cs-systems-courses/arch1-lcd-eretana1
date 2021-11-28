# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:presentDemo.elf

#additional rules for files
presentDemo.elf: ${COMMON_OBJECTS} presentDemo.o wdt_handler.o buzzer.o led.o switches.o stateMachines.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd 

load: presentDemo.elf
	msp430loader.sh $^

clean:
	rm -f *.o *.elf

buzzer.o: buzzer.h
led.o: led.h
presentDemo.o: presentDemo.h
switches.o: switches.h
stateMachines.o: stateMachines.h
