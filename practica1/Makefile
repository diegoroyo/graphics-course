BINDIR = bin
MAIN = main

PROJOBJ = $(patsubst ./%.cpp, ${BINDIR}/%.o, $(wildcard ./*.cpp))

NOMBREEXE = main

CPPFLAGS = -O3 -std=c++11
CC = g++

#CPPFLAGS = -ggdb -O0 -std=c++11
#CC = x86_64-w64-mingw32-g++

.PHONY: all
all: ${BINDIR}/${NOMBREEXE}

.PHONY: clean
clean:
	rm -rf ${BINDIR}/*

${BINDIR}/${NOMBREEXE}: $(PROJOBJ)
	${CC} $^ ${CPPFLAGS} -o $@

${BINDIR}/%.o: %.cpp
	${CC} -c ${CPPFLAGS} $< -o $@