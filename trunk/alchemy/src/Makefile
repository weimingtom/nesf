CC= gcc
FLAG= -Wall -O3 -swc
LIB= Screen.o RAM.o PPU.o Mapper.o M6502.o JoyPad.o NESCafe.o Wrapper.o

all:	nes.swc

nes.swc: ${LIB}
		${CC} -o $@ ${FLAG} ${LIB}
o:	${LIB}

clean:
	rm *.o
	rm *.exe
	rm *.swc
	del *.o
	del *.exe
	del *.swc