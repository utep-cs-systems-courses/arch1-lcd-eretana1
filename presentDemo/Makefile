# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:presentDemo.elf

#additional rules for files
presentDemo.elf: ${COMMON_OBJECTS} presentDemo.o wdt_handler.o buzzer.o led.o switches.o stateMachines.o light25-c.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd 

load: presentDemo.elf
	msp430loader.sh $^

#additional rules for files (Assembly)
presentDemo-s.elf: ${COMMON_OBJECTS} presentDemo.o wdt_handler.o buzzer.o led.o switches.o stateMachines.o light25-s.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd 

load-s: presentDemo-s.elf
	msp430loader.sh $^

clean:
	rm -f *.o *.s~ *.s# *.elf

buzzer.o: buzzer.h
led.o: led.h 
light25-c.o: light25-assy.h led.h
light25-s.o: light25-assy.h led.h
presentDemo.o: presentDemo.h buzzer.h led.h switches.h stateMachines.h 
switches.o: switches.h led.h
stateMachines.o: stateMachines.h led.h light25-assy.h
