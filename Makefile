objs := $(patsubst src/%.c, out/%,  $(wildcard src/*.c))

all: ${objs}

out/%: src/%.c out/util.o | out
	@if [ "$(@F)" != util ] && [ "$(@F)" != graphics ] ; then \
	  ${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} out/util.o $< ;\
	fi

out/util.o: src/util.c src/util.h | out
	${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out:
	@mkdir out

.PHONY: clean
clean: 
	@${RM} -rf out
