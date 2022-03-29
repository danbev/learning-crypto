objs := $(patsubst src/%.c, out/%,  $(wildcard src/*.c))

all: out/challenge_01 out/challenge_02 out/challenge_03 out/challenge_04 \
	out/freq_gen out/freq_analysis out/challenge_05

out/challenge_01: out/base64.o out/bin.o out/hex.o src/challenge_01.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/challenge_02: out/bin.o out/hex.o out/xor.o out/str.o src/challenge_02.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/challenge_03: out/bin.o out/hex.o out/xor.o out/dec.o out/str.o out/freq.o src/challenge_03.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/challenge_04: out/bin.o out/hex.o out/xor.o out/dec.o out/str.o out/freq.o src/challenge_04.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/challenge_05: out/bin.o out/hex.o out/xor.o out/dec.o out/freq.o out/str.o src/challenge_05.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/freq_gen: src/freq_gen.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/freq_analysis: out/bin.o out/hex.o out/xor.o out/dec.o out/freq.o out/str.o src/freq_analysis.c | out
	@${CC} ${CFLAGS} -g -lm -I./src -o $@ ${DEPS} $^

out/bin.o: src/bin.c src/bin.h out/dec.o | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/base64.o: src/base64.c src/base64.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/hex.o: src/hex.c src/hex.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/dec.o: src/dec.c src/dec.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/xor.o: src/xor.c src/xor.h out/str.o | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/str.o: src/str.c src/str.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out/freq.o: src/freq.c src/freq.h | out
	@${CC} ${CFLAGS} -g -c -o $@ -lm -I./src $<

out:
	@mkdir out

.PHONY: clean
clean: 
	@${RM} -rf out
