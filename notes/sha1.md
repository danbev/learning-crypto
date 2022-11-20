### SHA-1

```
           +-----------------------------------+
           |          Message M                |   (any length)
           +-----------------------------------+
                            |
                            ↓
           +-----------------------------------+
           \                                   /
            \            SHA-1(M)             /
             \                               /
              -------------------------------
                            |
                            ↓
                   +-----------------+
	           |   Hash value    |              160 bits
                   +-----------------+
```
SHA-1 uses a Merkle-Damngård construction:
```
x = (x₁....xn)
     ↓
 +--------------+
 | padding      | Output size if 512 bits
 +--------------+
     ↓    ↓-----------+
 +--------------+     |
 \ compression /      | This value is fed back into the compression function
  +-----------+       | with the xi. The size of this value is 160 bits
     ↓                |
     +----------------+
     | When xn inputs have been processed we continue
     ↓
   H(x)  the hash function. x is 160 bits

```
the padding and compression is the Merkle-Damgard construction.

Lets take a closer look at the compression function:
Compression:
```
      x_i (512 bits)          H_i-1 (160 bits)
        |                       |
        ↓                       ↓----------+
+----------------+  W₀     +----------+    |
| Msg scheduler  | ------> | Round 0  |    |
+----------------+  32 bit +----------+    |           80 rounds
        |                       ↓          |
        |           W₁     +----------+    |
        +----------------> | Round 1  |    ↓
        |           32 bit +----------+    |
        ...                    ...         |
        |                       ↓          |
        |           W₈₀    +----------+    |
        +----------------> | Round 79 |    |
                    32 bit +----------+    |
                                | 32       |
                               [+]<--------+  add modulo 2³²
                                |   32
                                ↓
```
`w` are sub messages of size 

H_i-1 which is 160 bits is divided into four groups of 40 bits each
```
   +------------------------------------+
   |                160                 |
   +------------------------------------+
     ↓       ↓       ↓        ↓      ↓
   +----+  +----+  +----+  +----+  +----+
   | A  |  | B  |  | C  |  | D  |  | E  |
   +----+  +----+  +----+  +----+  +----+
     32      32      32      32      32
   
```
These 32 bit (words) are called the state. Notice that for the first round there
nothing, that is H₀ so the initial state is provided as:
```
S[0] = 0x67452301;
S[1] = 0xefcdab89;
S[2] = 0x98badcfe;
S[3] = 0x10325476;
S[4] = 0xc3d2e1f0;
```

Rounds:
4x20 = 80 rounds
There are 4 stages:
```
stage t=1, round j=0...19
stage t=2, round j=20...39
stage t=3, round j=40...59
stage t=4, round j=60...79
```

Each round has 5x32 (160) bits inputs (A, B, C, D, E) plus the message
schedule word W_j, which is also 32 bits.
```
         A    B    C    D    E
        (32) (32) (32) (32) (32)
         ↓    ↓    ↓    ↓    ↓
       +--------------------------+
w_j -> |        Round j           |
(32)   +--------------------------+


       +--------------------------+
       |  A  |  B  | C  | D  | E  |
       +--------------------------+
          |     |    |    |    |
          |     ↓    ↓    ↓    |
          |    +-----------+   ↓
          |    | f(B, C, D)|->[+]
       +-----+ +-----------+   ↓
       |<<< 5|--------------->[+] 
       +-----+  |    |    |    ↓
          |     |    |    |   [+]<---- Wj message scheduler value (32 bits)
          |  +-----+ |    |    |
          |  |<<<30| |    |    |
          |  +-----+ |    |    |
          |     |    |    |    |
          |     +-+  |    |    |
          |       |  |    |    |
          +-----+ |  |    |    ↓
                | |  |    |   [+]<---- kt round constant
          +-----|--------------+
          |     | |  |    |
          |     | |  |    +----+
          |     | |  +----+    |
          |     | +--+    |    |
          ↓     ↓    ↓    ↓    ↓
       +--------------------------+
       |  A  |  B  | C  | D  | E  |
       +--------------------------+


```
The 4 round functions are:
```
f0(B, C, D) = (B & C) | (!B & D) 
f1(B, C, D) = B ^ C ^ D
f2(B, C, D) = (B & C) | (B & D) | (C & D)
f3(B, C, D) = B ^ C ^ D
```
And the 4 round constants are:
```
c0 = 5A827999
c1 = 6ED9EBA1
c2 = 8F1BBCDC
c3 = CA62C1D6
```
And finally we have the message scheduler values.


This can be compared to block ciphers and how they work:
```
        K                      m
+----------------+  k₀     +----------+
| Key scheduler  | ------> | Round 0  | 
+----------------+         +----------+
        |                       ↓ 
        |           k₁     +----------+
        +----------------> | Round 1  |
        |                  +----------+
       ...                     ...
        |                       ↓ 
        |           ks-1   +----------+
        +----------------> | Round s-1|
                           +----------+
  
```
