objs := $(patsubst src/%.c, out/%,  $(wildcard src/*.c))

override CFLAGS += -Werror -g -lm -I./src

all: out/challenge_01 out/challenge_02 out/challenge_03 out/challenge_04 \
	out/freq_gen out/freq_analysis out/challenge_05 out/base64_decoder \
	out/hamming_distance out/ceasar_encrypt out/ceasar_decrypt

out/challenge_01: out/base64.o out/bin.o out/hex.o out/str.o \
	src/challenge_01.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/challenge_02: out/bin.o out/hex.o out/xor.o out/str.o src/challenge_02.c \
	| out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/challenge_03: out/bin.o out/hex.o out/xor.o out/dec.o out/str.o \
       	out/freq.o src/challenge_03.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/challenge_04: out/bin.o out/hex.o out/xor.o out/dec.o out/str.o \
       	out/freq.o src/challenge_04.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/challenge_05: out/bin.o out/hex.o out/xor.o out/dec.o out/freq.o \
       	out/str.o src/challenge_05.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/challenge_06: out/bin.o out/hex.o out/xor.o out/dec.o out/freq.o \
       	out/str.o src/challenge_06.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/freq_gen: src/freq_gen.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/freq_analysis: out/bin.o out/hex.o out/xor.o out/dec.o out/freq.o \
       	out/str.o src/freq_analysis.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/hamming_distance: out/ham.o out/dec.o out/str.o out/bin.o out/crypto_math.o \
       	src/hamming_distance.c | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/base64_decoder: src/base64_decoder.c out/base64.o out/bin.o out/hex.o \
       	out/dec.o out/str.o | out
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/ceasar_encrypt: src/ceasar_encrypt.c out/freq.o out/dec.o out/ceasar.o \
	out/crypto_math.o
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/ceasar_decrypt: src/ceasar_decrypt.c out/freq.o out/dec.o out/ceasar.o \
	out/crypto_math.o
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/vigenere_encrypt: src/vigenere_encrypt.c out/vigenere.o out/crypto_math.o
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/vigenere_decrypt: src/vigenere_decrypt.c out/vigenere.o out/crypto_math.o
	@${CC} ${CFLAGS} -o $@ ${DEPS} $^

out/bin.o: src/bin.c src/bin.h out/dec.o | out
	@${CC} ${CFLAGS} -c -o $@  $<

out/base64.o: src/base64.c src/base64.h out/hex.o out/str.o | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/hex.o: src/hex.c src/hex.h | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/dec.o: src/dec.c src/dec.h | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/xor.o: src/xor.c src/xor.h out/str.o | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/str.o: src/str.c src/str.h | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/crypto_math.o: src/crypto_math.c src/crypto_math.h | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/ham.o: src/ham.c src/ham.h out/crypto_math.o | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/freq.o: src/freq.c src/freq.h | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/ceasar.o: src/ceasar.c src/ceasar.h out/freq.o | out
	@${CC} ${CFLAGS} -c -o $@ $<

out/vigenere.o: src/vigenere.c src/vigenere.h out/freq.o | out
	@${CC} ${CFLAGS} -c -o $@ $<

out:
	@mkdir out

.PHONY: clean
clean: 
	@${RM} -rf out
