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

#### Example of building and running:
```console
$ make out/challenge_01
```
All binary files are located in the `out`directory.
The challenge can then be run using:
```console
$ ./out/challenge_01
Cryptopals Set 1, Challenge 1 hex to base64 conversion
Input:     49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d
Expected : SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t
Actual   : SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t
```
This works in the same way for all challenges.

## Frequency distribution generator
[Frequency distribution generator](./notes/cryptoanalysis.md#frequency-distribution-generator)
