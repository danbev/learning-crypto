objs := $(patsubst src/%.c, out/%,  $(wildcard src/*.c))

all: out/challenge_01 out/challenge_02 out/challenge_03

out/challenge_01: out/base64.o out/util.o out/hex.o src/challenge_01.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/challenge_02: out/util.o out/hex.o out/xor.o src/challenge_02.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/challenge_03: out/util.o out/hex.o out/xor.o src/challenge_03.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/freq: src/freq.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/util.o: src/util.c src/util.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/base64.o: src/base64.c src/base64.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/hex.o: src/hex.c src/hex.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/xor.o: src/xor.c src/xor.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out:
	@mkdir out

.PHONY: clean
clean: 
	@${RM} -rf out
