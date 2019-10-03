BINDIR = bin
MAIN = main

PROJOBJ = $(patsubst ./%.cpp, ${BINDIR}/%.o, $(wildcard ./*.cpp))

NOMBREEXE = main

#CPPFLAGS = -O3 -std=c++11
CPPFLAGS = -ggdb -O0 -std=c++11
CC = g++

.PHONY: all
all: ${BINDIR}/${NOMBREEXE}

.PHONY: clean
clean:
	rm -rf ${BINDIR}/*

${BINDIR}/${NOMBREEXE}: $(PROJOBJ)
	${CC} $^ ${CPPFLAGS} -o $@

${BINDIR}/%.o: %.cpp
	${CC} -c ${CPPFLAGS} $< -o $@