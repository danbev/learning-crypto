## Elliptic Curve Digital Signature Algorithm (ECDSA)
As it sounds this is a DSA based on Elliptic Curve. Recall that RSA can also be
used with DSA.

For some background on DSA see [dsa.md](./dsa.md).

```
Elliptic curve                    : y² = x³ + ax + b (mod p) 
Base point/Generator on the Curve : G
Private key                       : k  (integer)
Public key                        : Gᵏ (group '+' operation, so G + G + G + ... k times)
                                    This is a point on the curve, (x, y)
```

The actual parameters to be used can be one of many which means that there are
different values for the following parameters:
* a (which is one the cooffients of the ellipse curve)
* b (which is one the cooffients of the ellipse curve)
* n (which is the total number of points on the curve, this used for mod n)
* prime number

For secp256k1 the values would be:
```
a = 0
b = 7
So those values get plugged into:
  y² = x³ + ax + b (mod p) 

n = 115792089237316195423570985008687907852837564279074904382605163141518161494337 (prime number)
    (order of the curve)

Generator point G
{
  x = 55066263022277343669578718895168534326250603453777594175500187360389116729240,
  y = 32670510020758816978083085130507043184471273380659243275938904335757337482424
}
```
### Key generation
The key generation will look like this:
```
private key (integer): random value in [0..n-1]. 32 bytes, 256 bits

public key: private key * G  (G + G + G + ... n times)
            So the public key will be a point on the curve.
```
The public key can be compressed to just on coordinate, either x or y + a
parity bit.

### Signing
So lets take a look at signing
```
// signature consist of two values, r and s
let (r, s) = sign(message, private_key);
```
Where `r` and `s` are integers. More about these values can be found below.

Lets say we have our message, the first thing we do is to get a hash value of
the message using the hashing algorithm for the curve in question:
```
let h = hash(message);
```
Next, we generate a random value `k`:
```
let k = [1..n-1] | h + private_key;
```
The second option is called deterministic ECDSA (or something like that).
TODO: read up on deterministic ECDSA.
Notice that this is similar to the private key, but the range here does not
include 0.

Next we calculate a random point R on the curve:
```
let R = k * G;
```
Notice that this is very simliar to how we calculate the public key, but instead
of using the private_key as the scalar the random `k` value is used.
R, like the public key, will be a point on the curve. In this case we only take
the x coordinate value and save that as r:
```
let r = R.x;
```

Next, we calculate the signature proof:
```
let s = k⁻¹ * (h + r * private_key) (mod n)
```
_wip_

So we find the inverse of the random value k that we generated before. And this
step is using the hash `h` so it is including knowledge of the hash value. The
private key is also used which proves that the signer had the private key.

And the the signature is the value of r and s which are then returned.
The values r and s are integers both within the group, so in the range [1..n-1].

To verify the signature we go through the following steps:
```
 let valid: bool = verify(message, Signature {r, s}, public_key);
```
First verify calculates the hash of the message:
```
let hash = sha256sum(message);
```
```
let s¹ = s⁻¹ (mod 1)
```
```
  let r′ = (hash * s¹) * G + (r * s1) * public_key
  // r′ will be a point on the curve, {x,y}
  if r′ == r {
     Ok(true)
  } else {
     Ok(false)
  }
```



### asn.1 format
The following is in hex, which means that each character is 1 byte
```
0000 =  0
0001 =  0
1111 = 15
```

```text
3081872103013672A8648CE3D21682A8648CE3D31746D306B2114208088D96629582F61625C4BD8FE9DA5D3559D921CB2756038E152EDB81FE82A144342046FD342D3F4D276AA6734516D4D1A19B54AB3C8D58E9A519CBB132E5CE771762EDFFFDECE8A4FCACB394CF6B77C9A75B20FFC9BDDCD0AB83B937BA519A47E1
```
30 is tag for a SEQUENCE which is followed by the byte value for that sequence.
So we can see 30 as the first digit and then 8187 must be the length of this
sequence I think. This is the followed by 02 which is an Integer which and the
length of it is 1, and the content 00 in this case. Following that we have
another sequence (30) etc.

```text
DER: 308187020100301306072a8648ce3d020106082a8648ce3d030107046d306b020101042032447ff14359b76d0cea62fdc2bb07cb901de6a32288606cf8632ec174a0f5d8a14403420004d166c5b358bd2223b7429d54f954e8664bed0ef0dc39d3af1c367fa1269049ce55b09fe20cabfb51e315312839231f103b20afb1a98bf0680a35177213f75052

[U] SEQUENCE (30)
  [U] INTEGER (02): 0
  [U] SEQUENCE (30)
    [U] OBJECT (06): 1.2.840.10045.2.1 - ECC
    [U] OBJECT (06): 1.2.840.10045.3.1.7 - secp256r1
  [U] OCTET STRING: 0xb'306B020101042032447FF14359B76D0CEA62FDC2BB07CB901DE6A32288606CF8632EC174A0F5D8A14403420004D166C5B358BD2223B7429D54F954E8664BED0EF0DC39D3AF1C367FA1269049CE55B09FE20CABFB51E315312839231F103B20AFB1A98BF0680A35177213F75052'
  Private key:  306b020101042032447ff14359b76d0cea62fdc2bb07cb901de6a32288606cf8632ec174a0f5d8a14403420004d166c5b358bd2223b7429d54f954e8664bed0ef0dc39d3af1c367fa1269049ce55b09fe20cabfb51e315312839231f103b20afb1a98bf0680a35177213f75052

-----BEGIN PUBLIC KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgMkR/8UNZt20M6mL9wrsHy5Ad5qMiiGBs+GMuwXSg9dihRANCAATRZsWzWL0iI7dCnVT5VOhmS+0O8Nw5068cNn+hJpBJzlWwn+IMq/tR4xUxKDkjHxA7IK+xqYvwaAo1F3IT91BS
-----END PUBLIC KEY-----
```
The value of the ecc_oid is `1.2.840.10045.2.1` but when working on a task and
printing out the binary values I noticed that the values were:
```
[42, 134, 72, 206, 61, 2, 1] 
```
After reading this [doc](https://learn.microsoft.com/en-us/windows/win32/seccertenroll/about-object-identifier?redirectedfrom=MSDN) I learned the following:
```text
1.2.840.10045.2.1 ECC                                                    
  |
  ↓                                                                       
 1*40+2  The first node is multiplied by 40 and added to the second node. 
 ↓                                                                       
[42, 134, 72, 206, 61, 2, 1] 
```
So that is why it start with 42.
The next rule is that if a value is less than or equal to 127 then it will be
encoded into a single byte. This is not the case for 840 and it will get encoded
into two bytes like this:
```
Dec: 840
Hex: 00000110 1001000 
And we add 1 to the left most byte:
Hex: 10000110 1001000 
      (134)    (72)
```

