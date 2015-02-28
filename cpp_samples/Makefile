ISAL_PATH=../../src_2.12
CC = gcc
CFLAGS = -Wall -I${ISAL_PATH}/include
# make sure the file "libisa-l.a" must exist in correct folder
LIBS = -L${ISAL_PATH}/bin -lisa-l

samples = crc_sample.out

all: ${samples}

# $@ means target, $? means reprequisites
crc_sample.out : crc_sample.o
	${CC} ${CFLAGS} -o $@ $? ${LIBS}

crc_sample.o : crc_sample.cc
	g++ -c -I${ISAL_PATH}/include -o $@ $?

# .PHONY means clean is a psaudo target
.PHONY : clean

# "-rm" means run the command quietly
clean :
	-rm *.out *.o

