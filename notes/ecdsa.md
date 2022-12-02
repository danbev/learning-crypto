## Elliptic Curve Digital Signature Algorithm (ECDSA)
As it sounds this is a DSA based on Elliptic Curve. Recall that RSA can also be
used with DSA.

For some background on DSA see [dsa.md](./dsa.md).


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

