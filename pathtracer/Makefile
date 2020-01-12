# Where to store executable and object files
BINDIR = bin
EXNAME = pathtracer

# Library modules configuration
LIBDIR  = ../lib
LIBOBJ  = $(patsubst ${LIBDIR}/%.cpp, ${LIBDIR}/${BINDIR}/%.o, $(wildcard ${LIBDIR}/*.cpp))
LIBOBJ := $(LIBOBJ) $(patsubst ${LIBDIR}/%.cpp,               \
							   ${LIBDIR}/${BINDIR}/%.o,       \
							   $(wildcard ${LIBDIR}/*/*.cpp))

# Project-specific files
PROJOBJ = $(patsubst ./%.cpp, ${BINDIR}/%.o, $(wildcard ./*.cpp))

# Compiler settings
CPPFLAGS = -std=c++11
LDFLAGS = -std=c++11 -pthread
CC = g++

.PHONY: all
all: CPPFLAGS += -O3
all: folders ${BINDIR}/${EXNAME}

.PHONY: debug
debug: CPPFLAGS += -ggdb -O0
debug: folders ${BINDIR}/${EXNAME}

.PHONY: clean
clean:
	rm -f ${LIBOBJ}
	rm -f ${BINDIR}/*

folders:
	mkdir -p ${BINDIR} ${LIBDIR}/${BINDIR} ${LIBDIR}/${BINDIR}/camera ${LIBDIR}/${BINDIR}/io ${LIBDIR}/${BINDIR}/math ${LIBDIR}/${BINDIR}/scene

${BINDIR}/${EXNAME}: $(LIBOBJ) $(PROJOBJ)
	${CC} $^ ${LDFLAGS} -I ${LIBDIR} -o $@
	
${LIBDIR}/${BINDIR}/%.o: ${LIBDIR}/%.cpp
	${CC} -c ${CPPFLAGS} -I ${LIBDIR} $< -o $@

${BINDIR}/%.o: %.cpp
	${CC} -c ${CPPFLAGS} -I ${LIBDIR} $< -o $@