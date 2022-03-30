# Cryptography notes and code
This repository contains notes and code for learning about various things
related to cryptography.
Notes are available in the [notes](./notes) directory and I'll try have one
file for each topic.

## Cryptopals challenges
This following are links to [cryptopals](https://cryptopals.com/) crypto
challenges.

Set 1:
* [Challenge 01](src/challenge_01.c)
* [Challenge 02](src/challenge_02.c)
* [Challenge 03](src/challenge_03.c)
* [Challenge 04](src/challenge_04.c)
* [Challenge 05](src/challenge_05.c)

## Frequency distribution generator
[freq_gen](./src/freq_gen.c) can be used to generate frequency distributions
for a given test and will output arrays that can be included in other programs.
For example:
```console
$ ./out/freq_gen gatsby.txt 
Total lowercase letters: 214088
Total uppercase letters: 7710
Lower case:
double lowercase[] = {
  0.083055, // a 
  0.015013, // b 
  0.023163, // c 
  0.047177, // d 
  0.124911, // e 
  0.020688, // f 
  0.022551, // g 
  0.059508, // h 
  0.063166, // i 
  0.001355, // j 
  0.009632, // k 
  0.040138, // l 
  0.025863, // m 
  0.070247, // n 
  0.079617, // o 
  0.015517, // p 
  0.000775, // q 
  0.058200, // r 
  0.060942, // s 
  0.091374, // t 
  0.029404, // u 
  0.009272, // v 
  0.024397, // w 
  0.001471, // x 
  0.021874, // y 
  0.000691, // z 
};
double uppercase[] = {
  0.051492, // A 
  0.033982, // B 
  0.026719, // C 
  0.044747, // D 
  0.031518, // E 
  0.020623, // F 
  0.064202, // G 
  0.057977, // H 
  0.228534, // I 
  0.019196, // J 
  0.005447, // K 
  0.019196, // L 
  0.043320, // M 
  0.028016, // N 
  0.025681, // O 
  0.022438, // P 
  0.000649, // Q 
  0.014267, // R 
  0.052010, // S 
  0.102853, // T 
  0.007912, // U 
  0.004799, // V 
  0.066407, // W 
  0.000519, // X 
  0.027367, // Y 
  0.000130, // Z 
};
```
