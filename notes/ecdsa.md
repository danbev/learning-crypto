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
Notice what goes into this integer `s`, we have the hash `h`, the x coordinate
of the random point on the curve we generate using `k`, and we have the
private key (which like `k` is an integer).

Finally, (r,s) is returned as the output of the signature function and this
is the signature.
```console
$ openssl ecparam -name secp256k1 -genkey -noout -out ec-secp256k1-priv-key.pem
```
Lets take a closer look at the private key:
```console
$ cat ec-secp256k1-priv-key.pem 
-----BEGIN EC PRIVATE KEY-----
MHQCAQEEIMyUMS/Dr2gId03jsdbjYPx0aS06OBVhSAbGA2CbtRMAoAcGBSuBBAAK
oUQDQgAEbDBmX6Eq+xGna0+dIKD3ighEbWFRDITBrTaxT0T4oKBcz01LK0sHKP3F
VljanOa/ECwwia9DaMbu8TIquM9N7g==
-----END EC PRIVATE KEY-----
```
So this is a structure that holds the actual key and we can parse this into
its ASN1 format using:
```console
$ openssl asn1parse -i -in ec-secp256k1-priv-key.pem 
    0:d=0  hl=2 l= 116 cons: SEQUENCE          
    2:d=1  hl=2 l=   1 prim:  INTEGER           :01
    5:d=1  hl=2 l=  32 prim:  OCTET STRING      [HEX DUMP]:CC94312FC3AF6808774DE3B1D6E360FC74692D3A3815614806C603609BB51300
   39:d=1  hl=2 l=   7 cons:  cont [ 0 ]        
   41:d=2  hl=2 l=   5 prim:   OBJECT            :secp256k1
   48:d=1  hl=2 l=  68 cons:  cont [ 1 ]        
   50:d=2  hl=2 l=  66 prim:   BIT STRING 
```
Notce that the octet string contains 32 hex values:
```
CC 94 31 2F C3 AF 68 08 77 4D E3 B1 D6 E3 60 FC 74 69 2D 3A 38 15 61 48 06 C6 03 60 9B B5 13 00
```
And the decimal value of that is:
```console
$ echo "ibase=16;CC94312FC3AF6808774DE3B1D6E360FC74692D3A3815614806C603609BB51300" | env BC_LINE_LENGTH=0 bc -l
92533653949870085077092314185110289550130387703853847228490949558700703290112
```
And using the private key we can generate the public key using:
```console
$ openssl ec -in ec-secp256k1-priv-key.pem -pubout > ec-secp256k1-pub-key.pem
```
```console
$ cat ec-secp256k1-pub-key.pem 
-----BEGIN PUBLIC KEY-----
MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEbDBmX6Eq+xGna0+dIKD3ighEbWFRDITB
rTaxT0T4oKBcz01LK0sHKP3FVljanOa/ECwwia9DaMbu8TIquM9N7g==
-----END PUBLIC KEY-----
```
And looking at this we can/might be able to see that this is a general format,
there is not indication about what type of public key this is.
```console
$ openssl asn1parse -i  -in ec-secp256k1-pub-key.pem 
    0:d=0  hl=2 l=  86 cons: SEQUENCE          
    2:d=1  hl=2 l=  16 cons:  SEQUENCE          
    4:d=2  hl=2 l=   7 prim:   OBJECT            :id-ecPublicKey
   13:d=2  hl=2 l=   5 prim:   OBJECT            :secp256k1
   20:d=1  hl=2 l=  66 prim:  BIT STRING
```
```console
$ openssl ec -pubin -in ec-secp256k1-pub-key.pem -text
read EC key
Public-Key: (256 bit)
pub:
    04:6c:30:66:5f:a1:2a:fb:11:a7:6b:4f:9d:20:a0:
    f7:8a:08:44:6d:61:51:0c:84:c1:ad:36:b1:4f:44:
    f8:a0:a0:5c:cf:4d:4b:2b:4b:07:28:fd:c5:56:58:
    da:9c:e6:bf:10:2c:30:89:af:43:68:c6:ee:f1:32:
    2a:b8:cf:4d:ee
ASN1 OID: secp256k1
writing EC key
-----BEGIN PUBLIC KEY-----
MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEbDBmX6Eq+xGna0+dIKD3ighEbWFRDITB
rTaxT0T4oKBcz01LK0sHKP3FVljanOa/ECwwia9DaMbu8TIquM9N7g==
-----END PUBLIC KEY-----
```
Not the public key has 65 hex values if I'm not mistaken. So I'm thinking that
this is not in compressed form: 
```
04 6c 30 66 5f a1 2a fb 11 a7 6b 4f 9d 20 a0
f7 8a 08 44 6d 61 51 0c 84 c1 ad 36 b1 4f 44 
f8 a0 a0 5c cf 4d 4b 2b 4b 07 28 fd c5 56 58 
da 9c e6 bf 10 2c 30 89 af 43 68 c6 ee f1 32 
2a b8 cf 4d ee
```
To see the compressed form we can use:
```console
$ openssl ec -pubin -in ec-secp256k1-pub-key.pem -text -conv_form compressed
read EC key
Public-Key: (256 bit)
pub:
    02:6c:30:66:5f:a1:2a:fb:11:a7:6b:4f:9d:20:a0:
    f7:8a:08:44:6d:61:51:0c:84:c1:ad:36:b1:4f:44:
    f8:a0:a0
ASN1 OID: secp256k1
writing EC key
-----BEGIN PUBLIC KEY-----
MDYwEAYHKoZIzj0CAQYFK4EEAAoDIgACbDBmX6Eq+xGna0+dIKD3ighEbWFRDITB
rTaxT0T4oKA=
-----END PUBLIC KEY-----
```
Let create something that we can sign:
```
$ echo something > blob
```
And now create a signature for that file:
```console
$ openssl pkeyutl -sign -inkey ec-secp256k1-priv-key.pem -in blob > signature
```
So the signature 
```console
$ xxd signature 
00000000: 3046 0221 00c2 799e b372 68fa 807e f195  0F.!..y..rh..~..
00000010: 4531 e2dc c70f 67c9 89de c3de d3c1 6637  E1....g.......f7
00000020: 304e b08f 5c02 2100 9cb8 086f 7c07 4f12  0N..\.!....o|.O.
00000030: 7924 605d f565 dde0 528c c89c 302f cdfc  y$`].e..R...0/..
00000040: 404e aed1 8e3b ee95                      @N...;..
```
An ASN1 Sequence has the hex value of 30 so lets try parsing this as a ASN1
structure:
```console
$ openssl asn1parse -i  -in signature -inform der
  0:d=0  hl=2 l=  70 cons: SEQUENCE          
   2:d=1  hl=2 l=  33 prim:  INTEGER   :C2799EB37268FA807EF1954531E2DCC70F67C989DEC3DED3C16637304EB08F5C
   37:d=1  hl=2 l=  33 prim:  INTEGER  :9CB8086F7C074F127924605DF565DDE0528CC89C302FCDFC404EAED18E3BEE95
```
So I'm thinking that the first integer is `r` and the second is `s`.

### Verifying
The verify function takes as input the message, the signature {r, s}, and the
public key (point on the curve).

1) The message is hashed:
```
let h = hash(message);
```

2) The modular inverse of s, mod n, is calculated:
```
let s1 = s⁻¹ (mod n)
```
Recall that s is an integer.

3) Recover the random point R used during the signing:
```
let R′ = (h * s1) * G + (r * s1) * pub_key
let r′ = R′.x;
```
But don't we already know the value of R, or at least the x coordinate, as it
is passed in as one of the values of the signature tuple?  
Well, we are passed that value but that is the value that we are going to check
against to make sure they are the same. So we need to calculate this and notice
that we are using the hash.

4) Compare r′ == r
Recall that r is only the x coordinate part of the random point.

So lets try to understand this. We start with how the signature value s is
created:
```
let private_key = random[0..n-1];
let public_key = private_key * G;
let k = random[1..n-1] | h + private_key;
let R = k * G;
let r = R.x;

let s = k⁻¹ * (h + r * private_key) (mod n)

let signature = Signature{r, s};
```
So this value encodes information about the hash of the message, the x
coordinate of the random point R, and the private key.

```
let R′ = (h * s1) * G + (r * s1) * pub_key
```
We can expand/replace the `pub_key` with how it was generated which was using
`private_key * G`:
```
let R′ = (h * s1) * G + (r * s1) * private_key * G
```
And we can rewrite this as:
```
let R′ = (h * s1) * G + (r * s1) * private_key * G
let R′ = (h + r * private_key) * s1 *  G;
```
And `s1` was calculated using which we can rewrite as:
```
let s = k⁻¹ * (h + r * private_key) (mod n)

let s1 = s⁻¹ (mod n)
// replace s with
let s1 = (k⁻¹ * (h + r * private_key))⁻¹ (mod n)
// Now we take the invers of k and the inverse of (h + r * private_key)
let s1 = k * (h + r * private_key)⁻¹ (mod n)
```
And we take this representation of `s` and replace `s1` in the following
expression:
```
let R′ = (h + r * private_key) * s1 *  G;
                
let R′ = (h + r * private_key) * k * (h + r * private_key)⁻¹ (mod n) *  G;
```
Recall that `x⁻¹ * x = 1`, so (h + r * private_key) and (h + r * private_key)⁻¹
will become 1
```
let R′ = 1 * k * (mod n) *  G;
let R′ = k * G (mod n);
```
Notice that is the same point that was calculated by the sign function.


We can verify using the following command:
```console
$ openssl pkeyutl -in blob -inkey ec-secp256k1-pub-key.pem -pubin -verify -sigfile signature
Signature Verified Successfully
```
If we make a change to the blob the verification will fail:
```console
$ openssl pkeyutl -in blob -inkey ec-secp256k1-pub-key.pem -pubin -verify -sigfile signature
Signature Verification Failure
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

