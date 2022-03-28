### Frequency analysis
This can be used to find out the frequency of letters in a language. For example
we can use the program [freq_gen](../src/freq_gen.c) to find the frequency of
letter in the book `The Great Gatsby`:
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
We can visually inspect this and see which letters occur most frequently. 

Now, if we are looking a cipher text we have no idea how the plain text was
encrypted, like was a mono

### Index of Coincidence
The goal here is to have a method of identifying if a cipher used a mono or
poly alphabet. While this is can be done manually by inspecting a graph of
letter frequency distributions we want to be able to do this mathematically
as well.

Lets say that we have the following frequency distributions taken from a text
of 1000 characters:
```
Mono                 Poly
A 73                 A 38
B  9                 B 39
C 30                 C 38
D 44                 D 39
...                  ...
```
Notice that the polyalphabetic distribution is smooth and the monoalphabetic
distribution is rough. Picking a single letter from either distribution will
give us a letter with 100% chance which may seem obvious and that does not
provide and information as to if the cipher was using a mono or poly alphabet.
But what about picking too of the same letters?  
For mono:
```
            73     72
Both A's = ---- * --- = 0,00526
           1000   999
```
For poly:
```
            38     38
Both A's = ---- * --- = 0,00144
           1000   999
```
If we do this for all letters and sum the up:
```
 Z     i    i-1
 ∑ = ---- * --------
i=A  1000   1000 - 1

Poly: 0,038
Mono: 0,066
```
So 0,038 is the probability of picking 2 A's or B's or C's, etc, from a
poly. distribution.

When I first saw this I was surpised and I thought it would be the other way
around. There is an example of using coin tosses that might help clarify this:
Fair coin (even distribution, poly):
```
P(H): 0,5                     P=Probability
P(T): 0,5                     H=Head
                              T=Tail

P(H or T) = P(H) + P(T) = 0,5 + 0,5 = 1

P(2H or 2T) = P(2H) + P(2T)
            = P(0.5 * 0.5) + P(0,5 * 0,5)
            = P(0.25) + P(0,25)
            = 0,5
```
Unfair coid (Uneven/rough distribution, mono)
```
P(H): 0,7                     P=Probability
P(T): 0,3                     H=Head
                              T=Tail

P(H or T) = P(H) + P(T) = 0,7 + 0,3 = 1

P(2H or 2T) = P(2H) + P(2T)
            = P(0.7 * 0.7) + P(0,3 * 0,3)
            = P(0.49) + P(0,09)
            = 0,58
```
So we can use the index of coincedence on a ciphertext and if the index is
around 0,04 the we know that we are dealing with a polyalphabetic cipher, and
if it is around 0.06 it is a monoalphabetic cipher.
