### Frequency analysis
This can be used to find out the frequency of letters in a language. For example
we can use the program [freq](../src/freq.c) to find the frequency of letter
in the book `The Great Gatsby`:
```console
$ ./out/freq gatsby.txt 
Lower case:
a: 17781
b: 3214
c: 4959
d: 10100
e: 26742
f: 4429
g: 4828
h: 12740
i: 13523
j: 290
k: 2062
l: 8593
m: 5537
n: 15039
o: 17045
p: 3322
q: 166
r: 12460
s: 13047
t: 19562
u: 6295
v: 1985
w: 5223
x: 315
y: 4683
z: 148

Upper case:
A: 397
B: 262
C: 206
D: 345
E: 243
F: 159
G: 495
H: 447
I: 1762
J: 148
K: 42
L: 148
M: 334
N: 216
O: 198
P: 173
Q: 5
R: 110
S: 401
T: 793
U: 61
V: 37
W: 512
X: 4
Y: 211
Z: 1
```
We can visually inspect this and see which letters occur most frequently. 

Now, if we are looking a cipher text we have no idea how the plain text was
encrypted, like was a mono
