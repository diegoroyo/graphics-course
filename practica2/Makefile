BINDIR = bin
PIDIR = pngimage
MAIN = main

PIOBJ = $(patsubst ${PIDIR}/%.cpp, ${BINDIR}/%.o, $(wildcard ${PIDIR}/*.cpp))
PROJOBJ = $(patsubst ./%.cpp, ${BINDIR}/%.o, $(wildcard ./*.cpp))

NOMBREEXE = tonemapper

CPPFLAGS = -O3 -std=c++11
#CPPFLAGS = -ggdb -O0 -std=c++11
CC = g++

.PHONY: all
all: ${BINDIR}/${NOMBREEXE}

.PHONY: clean
clean:
	rm -rf ${BINDIR}/*

${BINDIR}/${NOMBREEXE}: $(PROJOBJ) $(PIOBJ)
	${CC} $^ ${CPPFLAGS} -o $@
	
${BINDIR}/%.o: ${PIDIR}/%.cpp
	${CC} -c ${CPPFLAGS} $< -o $@

${BINDIR}/%.o: %.cpp
	${CC} -c ${CPPFLAGS} $< -o $@