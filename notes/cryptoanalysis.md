## Frequency analysis

### Frequency distribution generator
This can be used to find out the frequency of letters in a language.
[freq_gen](./src/freq_gen.c) can be used to generate frequency distributions
for a given test and will output arrays that can be included in other programs.
For example we can use it to find the frequency of letter in the book
`The Great Gatsby`:
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


### Breaking the Vigenere cipher
Recall that the Vigenere cipher uses a keyword instead of a single key like
the Ceasar cipher. 

There is a very nice and clear [video](https://www.youtube.com/watch?v=QgHnr8-h0xI) 
which I've taken notes below so that I can understand and implement this. 

To break this cipher we need to start with figuring out the length of this
keyword.

Alphabet/number reference:
```
a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t  u  v  w  x  y  z
0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
```

Take the following example were we have only repeating plaintext values A and
a keyword of ABC. Notice that the same plaintext letter can map to different
ciphertext letters, which is not the case in the Ceasar cipher:
```
A A A A A A A A A A A A A A A A A
A B C A B C A B C A B C A B C A B
↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
0 1 2 0 1 2 0 1 2 0 1 2 0 1 2 0 1
↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
A B A A B C A B C A B C A B C A B
```
The frequency distribution for this is much more uniform, if we imagine a graph
then there won't be any large spikes or low vallys.

We can group these into groups of letters that have been shifted by 0, 1, and
two.

Now, we start by thinking about our plaintext as a number of positions where
our english letter can go:
```
E = English language letter (standard frequency distibution)
    the actual letter any letter from a-z.
S = Shifted (non-standard frequency distribution)

                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Plaintext:       |E|E|E|E|E|E|E|E|E|E|E|E|E|E|E|E|E|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Key:             |A|C|A|C|A|C|A|C|A|C|A|C|A|C|A|C|A|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                     ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Ciphertext:      |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
Now, we can create a vector of the standard probabilites, the following
was generated by [freq_gen.c](../src/freq_gen.c):
```c
double e[] = {
  0.077432, // a 
  0.014022, // b 
  0.026657, // c 
  0.049208, // d 
  0.134645, // e 
  0.025036, // f 
  0.017007, // g 
  0.057198, // h 
  0.062948, // i 
  0.001268, // j 
  0.005085, // k 
  0.037062, // l 
  0.030277, // m 
  0.071253, // n 
  0.073800, // o 
  0.017513, // p 
  0.000950, // q 
  0.061072, // r 
  0.061263, // s 
  0.087605, // t 
  0.030427, // u 
  0.011137, // v 
  0.021681, // w 
  0.001988, // x 
  0.022836, // y 
  0.000629, // z 
};
```
The probability that one of the E boxes above is an A is simply `e[0]`, and
the probability of B is `e[1]` etc.
If we want to find the probablity of two E boxes both being a:
```
double prob = e[0] * e[0];
```
This would produce `0.005996`.

And we can also ask what the probability of two of the boxes being the same,
not just A and A, but any pair like B and B, C and C, etc.
```c
  double dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    dot_product += e[i] * e[i];
  }
  printf("%f\n", dot_product);
```
This would produce `0.065956`.

Now, if we create a second array with the frequency distributions for the
shifted boxes we will have values in different places. In our example we are
shifting only by C which is 3
```c
double s[] = {
  0.001988, // x 
  0.022836, // y 
  0.000629, // z 
  0.077432, // a 
  0.014022, // b 
  0.026657, // c 
  0.049208, // d 
  0.134645, // e 
  0.025036, // f 
  0.017007, // g 
  0.057198, // h 
  0.062948, // i 
  0.001268, // j 
  0.005085, // k 
  0.037062, // l 
  0.030277, // m 
  0.071253, // n 
  0.073800, // o 
  0.017513, // p 
  0.000950, // q 
  0.061072, // r 
  0.061263, // s 
  0.087605, // t 
  0.030427, // u 
  0.011137, // v 
  0.021681, // w 
};
```
But notice that we get the same result for our the dot product as the order
does not matter.

But how about comparing  `a[i]` to `s[i]`?
```c
  double combined_dot_product = 0.0;
  for (int i = 0; i < freq_len; i++) {
    combined_dot_product += sampled_freq[i] * s[i];
  }
  printf("%f\n", combined_dot_product);
```
This will produce `0.033782`. 
To summaries the outputs from above:
```console
standard_freq dot_product: 0.065956
shifted freq dot_product: 0.065956
sampled_freq dot shifted_freq: 0.033782
```
When we take dot products with the same vector, v . v, the we get a larger
value than when do it with a shifted vector.
With that knowledge lets take a copy of the ciphertext from above and shift
it one position:
```
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Ciphertext:      |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Copy:            |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                         dot product larger

                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Ciphertext:      |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Shifted by one:    |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                         dot product smaller

                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Ciphertext:      |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+   
   Shifted by two:      |E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|S|E|
                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                         dot product larger
```
If we now take the dot product we would expect to get a smaller value compared
to when we compared the aligned ciphertext copy. In this case the key
was only one character but if it was multiple characters we would still see
the same behaviour namely that when we have an alignment the dot product will be
larger. If we could the number of shifts between these the larger dot product
that should give us the key length.

```console
$ ./out/vigenere_encrypt "some where over the rainbow, sun shine and cares are gone and everyone is happy" bajja
keyword: bajja
ciphertext: tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
```
So, we now take the ciphertext and take a copy. This is then shifted on
position to the right and we then cound the number of matches we find:
```
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
1tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpy
                                                       ↑         1
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
2 tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbp
                  ↑                                ↑             2
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
2  tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhb
                                      ↑           ↑              2
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
4   tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbh
                         ↑                                       1
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
5    tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerb
        ↑  ↑↑                                           ↑        4
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
6     tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooer
          ↑                                         ↑      ↑     3
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
7      tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooe
                                            ↑ ↑         ↑        3
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh
8       tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahoo
         ↑                    ↑     ↑↑           ↑               5  
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh         
9        tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweaho         
                                                                 0       
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh    <---+
10        tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweah        |
                            ↑↑                     ↑           ↑ 4      |
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh        |
11         tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmewea        |
                   ↑                                             1      |
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh        |
12          tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmewe        |
                     ↑      ↑      ↑                             3      |
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh        |
13           tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmew        |
                                                    ↑            1      |
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh        |
14            tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawme        |
                                  ↑     ↑                        2      |
tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawmeweahooerbhbpyh        |
15             tovnwieanoweachfrjrncofbuosqrnfawmcbrnbasepxnfawm        |
                              ↑             ↑↑↑↑↑                6 <----+
```

