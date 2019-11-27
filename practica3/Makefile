BINDIR = bin
LIBDIR = ../lib
MAIN = main

LIBOBJ = $(patsubst ${LIBDIR}/%.cpp, ${LIBDIR}/${BINDIR}/%.o, $(wildcard ${LIBDIR}/*.cpp))
PROJOBJ = $(patsubst ./%.cpp, ${BINDIR}/%.o, $(wildcard ./*.cpp))

NOMBREEXE = pathtracer

CPPFLAGS = -O3 -std=c++11 -pthread
#CPPFLAGS = -ggdb -O0 -std=c++11
CC = g++

.PHONY: all
all: ${BINDIR}/${NOMBREEXE}

.PHONY: clean
clean:
	rm -rf ${LIBOBJ}
	rm -rf ${BINDIR}/*

${BINDIR}/${NOMBREEXE}: $(LIBOBJ) $(PROJOBJ)
	${CC} $^ ${CPPFLAGS} -I ${LIBDIR} -o $@
	
${LIBDIR}/${BINDIR}/%.o: ${LIBDIR}/%.cpp
	${CC} -c ${CPPFLAGS} -I ${LIBDIR} $< -o $@

${BINDIR}/%.o: %.cpp
	${CC} -c ${CPPFLAGS} -I ${LIBDIR} $< -o $@