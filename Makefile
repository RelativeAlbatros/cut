PREFIX := /usr/local
SRC := src/*.c
CCFLAGS := -std=c99 -Wall -Wextra -pedantic

options:
	@echo cut build options:
	@echo "LDFLAGS    = ${LDFLAGS}"
	@echo "CCFLAGS   = ${CCFLAGS}"
	@echo "CC        = ${CC}"

cut: ${SRC}
	mkdir bin/ -p
	${CC} -o bin/$@ $? ${CCFLAGS}

all: options cut

debug: ${SRC}
	${CC} -g -o bin/$@ $? ${CCFLAGS}

clean:
	rm -f bin/*

install: all
	cp bin/cut ${PREFIX}/bin

.PHONY: clean install debug run options all