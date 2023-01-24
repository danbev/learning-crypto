## Key formats
This document tries to go through how different key formats can be identified.

## Public-Key Cryptography Standard 8 (pkcs8) format
This is format for private keys, not a specific private key but any type of
private key.

For example, if we generate a key using cosign the private key will be in
pkcs8 format:
```console
$ cat ec-private.pkcs8
-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgWN0kLDouTHVU9i12
duKFEqgsVvfrPMBzReAInUHn+g+hRANCAARjrZ8Y7dPsPr0ea7nz79BwMBcHfRbO
HziRdEE2+ICQ3JR/mFVVdwf8XAW0npLs/uyesH0GuEtZEgbtfo84XYIn
-----END PRIVATE KEY-----
```

And we can inspect this format usign openssl asn1parse command:
```
$ openssl asn1parse -in ec-private.pkcs8
    0:d=0  hl=3 l= 135 cons: SEQUENCE          
    3:d=1  hl=2 l=   1 prim: INTEGER           :00
    6:d=1  hl=2 l=  19 cons: SEQUENCE          
    8:d=2  hl=2 l=   7 prim: OBJECT            :id-ecPublicKey
   17:d=2  hl=2 l=   8 prim: OBJECT            :prime256v1
   27:d=1  hl=2 l= 109 prim: OCTET STRING      [HEX DUMP]:306B020101042058DD242C3A2E4C7554F62D7676E28512A82C56F7EB3CC07345E0089D41E7FA0FA1440342000463AD9F18EDD3EC3EBD1E6BB9F3EFD0703017077D16CE1F3891744136F88090DC947F9855557707FC5C05B49E92ECFEEC9EB07D06B84B591206ED7E8F385D8
```
This is in [pkcs8](https://datatracker.ietf.org/doc/html/rfc5958)
format: 
```
PrivateKeyInfo ::= SEQUENCE {
  version                   Version,
  privateKeyAlgorithm       PrivateKeyAlgorithmIdentifier,
  privateKey                PrivateKey,
  attributes           [0]  IMPLICIT Attributes OPTIONAL
}

PrivateKeyAlgorithmIdentifier ::= AlgorithmIdentifier
PrivateKey ::= OCTET STRING
Attributes ::= SET OF Attribute

AlgorithmIdentifier ::= SEQUENCE {
  algorithm       OBJECT IDENTIFIER,
  parameters      ANY DEFINED BY algorithm OPTIONAL
}
```
The version is `00` which the spec says must be 0 for this version.
The PrivateKeyAlgorithmIdentifier is expanded to something that identifies the
type of private key in this sequence/struct. pkcs8 can be used with many
different types of private keys which is why this is here.

The `id-ecPublicKey` is  an identifier/name for the Object Identifier
1.2.840.10045.2.1.

We can inspect this key using:
```console
$ openssl ec -in ec-private.pkcs8 --text --noout
read EC key
Private-Key: (256 bit)
priv:
    58:dd:24:2c:3a:2e:4c:75:54:f6:2d:76:76:e2:85:
    12:a8:2c:56:f7:eb:3c:c0:73:45:e0:08:9d:41:e7:
    fa:0f
pub:
    04:63:ad:9f:18:ed:d3:ec:3e:bd:1e:6b:b9:f3:ef:
    d0:70:30:17:07:7d:16:ce:1f:38:91:74:41:36:f8:
    80:90:dc:94:7f:98:55:55:77:07:fc:5c:05:b4:9e:
    92:ec:fe:ec:9e:b0:7d:06:b8:4b:59:12:06:ed:7e:
    8f:38:5d:82:27
ASN1 OID: prime256v1
NIST CURVE: P-256
```

## EC Private Key format
This is a format specific to EC private keys and is defined in
[RFC-5915](https://www.rfc-editor.org/rfc/rfc5915).

An example of such a key is the following:
```console
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIFjdJCw6Lkx1VPYtdnbihRKoLFb36zzAc0XgCJ1B5/oPoAoGCCqGSM49
AwEHoUQDQgAEY62fGO3T7D69Hmu58+/QcDAXB30Wzh84kXRBNviAkNyUf5hVVXcH
/FwFtJ6S7P7snrB9BrhLWRIG7X6POF2CJw==
-----END EC PRIVATE KEY-----
```
And the format can be seen using:
```console
$ openssl asn1parse -i -in ec-private.pem 
    0:d=0  hl=2 l= 119 cons: SEQUENCE          
    2:d=1  hl=2 l=   1 prim:  INTEGER           :01
    5:d=1  hl=2 l=  32 prim:  OCTET STRING      [HEX DUMP]:58DD242C3A2E4C7554F62D7676E28512A82C56F7EB3CC07345E0089D41E7FA0F
   39:d=1  hl=2 l=  10 cons:  cont [ 0 ]        
   41:d=2  hl=2 l=   8 prim:   OBJECT            :prime256v1
   51:d=1  hl=2 l=  68 cons:  cont [ 1 ]        
   53:d=2  hl=2 l=  66 prim:   BIT STRING  
```
The format 
```
ECPrivateKey ::= SEQUENCE {
     version        INTEGER { ecPrivkeyVer1(1) } (ecPrivkeyVer1),
     privateKey     OCTET STRING,
     parameters [0] ECParameters {{ NamedCurve }} OPTIONAL,
     publicKey
```
The domain parameters are addressed in [spki.md](./spki.md).

We can inspect it using:
```console
$ openssl ec -in ec-private.pem --text --noout
read EC key
Private-Key: (256 bit)
priv:
    58:dd:24:2c:3a:2e:4c:75:54:f6:2d:76:76:e2:85:
    12:a8:2c:56:f7:eb:3c:c0:73:45:e0:08:9d:41:e7:
    fa:0f
pub:
    04:63:ad:9f:18:ed:d3:ec:3e:bd:1e:6b:b9:f3:ef:
    d0:70:30:17:07:7d:16:ce:1f:38:91:74:41:36:f8:
    80:90:dc:94:7f:98:55:55:77:07:fc:5c:05:b4:9e:
    92:ec:fe:ec:9e:b0:7d:06:b8:4b:59:12:06:ed:7e:
    8f:38:5d:82:27
ASN1 OID: prime256v1
NIST CURVE: P-256
```
It can be useful to be able to identify the bytes for these types of keys when
debugging so I'm adding this as a reference:
```console
$ xxd ec-private.pem 
00000000: 2d2d 2d2d 2d42 4547 494e 2045 4320 5052  -----BEGIN EC PR
00000010: 4956 4154 4520 4b45 592d 2d2d 2d2d 0a4d  IVATE KEY-----.M
00000020: 4863 4341 5145 4549 466a 644a 4377 364c  HcCAQEEIFjdJCw6L
00000030: 6b78 3156 5059 7464 6e62 6968 524b 6f4c  kx1VPYtdnbihRKoL
00000040: 4662 3336 7a7a 4163 3058 6743 4a31 4235  Fb36zzAc0XgCJ1B5
00000050: 2f6f 506f 416f 4743 4371 4753 4d34 390a  /oPoAoGCCqGSM49.
00000060: 4177 4548 6f55 5144 5167 4145 5936 3266  AwEHoUQDQgAEY62f
00000070: 474f 3354 3744 3639 486d 7535 382b 2f51  GO3T7D69Hmu58+/Q
00000080: 6344 4158 4233 3057 7a68 3834 6b58 5242  cDAXB30Wzh84kXRB
00000090: 4e76 6941 6b4e 7955 6635 6856 5658 6348  NviAkNyUf5hVVXcH
000000a0: 0a2f 4677 4674 4a36 5337 5037 736e 7242  ./FwFtJ6S7P7snrB
000000b0: 3942 7268 4c57 5249 4737 5836 504f 4632  9BrhLWRIG7X6POF2
000000c0: 434a 773d 3d0a 2d2d 2d2d 2d45 4e44 2045  CJw==.-----END E
000000d0: 4320 5052 4956 4154 4520 4b45 592d 2d2d  C PRIVATE KEY---
000000e0: 2d2d 0a                                  --.
```
And in decimal:
```console
$ od -t u1 ec-private.pem 
0000000  45  45  45  45  45  66  69  71  73  78  32  69  67  32  80  82
0000020  73  86  65  84  69  32  75  69  89  45  45  45  45  45  10  77
0000040  72  99  67  65  81  69  69  73  70 106 100  74  67 119  54  76
0000060 107 120  49  86  80  89 116 100 110  98 105 104  82  75 111  76
0000100  70  98  51  54 122 122  65  99  48  88 103  67  74  49  66  53
0000120  47 111  80 111  65 111  71  67  67 113  71  83  77  52  57  10
0000140  65 119  69  72 111  85  81  68  81 103  65  69  89  54  50 102
0000160  71  79  51  84  55  68  54  57  72 109 117  53  56  43  47  81
0000200  99  68  65  88  66  51  48  87 122 104  56  52 107  88  82  66
0000220  78 118 105  65 107  78 121  85 102  53 104  86  86  88  99  72
0000240  10  47  70 119  70 116  74  54  83  55  80  55 115 110 114  66
0000260  57  66 114 104  76  87  82  73  71  55  88  54  80  79  70  50
0000300  67  74 119  61  61  10  45  45  45  45  45  69  78  68  32  69
0000320  67  32  80  82  73  86  65  84  69  32  75  69  89  45  45  45
0000340  45  45  10
0000343
```

We can take the base64 encoded key, which are the characters between the header
and the footer, and base64 decode that and store it in a file:
```console
$ cat ec-private.raw | base64 -d - > ec-private.der
```
We can then read this using openssl:
```console
$ openssl ec -inform der -in ec-private.der -noout -text
read EC key
Private-Key: (256 bit)
priv:
    58:dd:24:2c:3a:2e:4c:75:54:f6:2d:76:76:e2:85:
    12:a8:2c:56:f7:eb:3c:c0:73:45:e0:08:9d:41:e7:
    fa:0f
pub:
    04:63:ad:9f:18:ed:d3:ec:3e:bd:1e:6b:b9:f3:ef:
    d0:70:30:17:07:7d:16:ce:1f:38:91:74:41:36:f8:
    80:90:dc:94:7f:98:55:55:77:07:fc:5c:05:b4:9e:
    92:ec:fe:ec:9e:b0:7d:06:b8:4b:59:12:06:ed:7e:
    8f:38:5d:82:27
ASN1 OID: prime256v1
NIST CURVE: P-256
```

### EC Subject Public Key Information (spki)

### BEGIN PUBLIC KEY
If a public key starts with this is is of type X.509 SubjectPublicKeyInfo. 
See [spki.md](./spki.md) form more information.

## Rust example
An example of parsing EC keys can be found in [ec-parse.rs](https://github.com/danbev/learning-rust/blob/master/crypto/src/ec-parse.rs)

### Encrypted EC Private Key
When private keys are stored on disk it is advisable to encrypt them.
As an example, when using cosign's generate-key-pair command the private key
will be in pkcs8 der format, which is then encrypted using
[nacl/secretbox](./naci_secretbox.md) and a passphrase scrypt for the key
derivation function. This is then placed in a file using the PEM format:
```
$ cat cosign.key 
-----BEGIN ENCRYPTED COSIGN PRIVATE KEY-----
eyJrZGYiOnsibmFtZSI6InNjcnlwdCIsInBhcmFtcyI6eyJOIjozMjc2OCwiciI6
OCwicCI6MX0sInNhbHQiOiJYUXUxU2s0ZHVadU1zQ3JqU3Y5aGZpc3EzZXNoZC9Z
SC9JMW9FaUNiNHU0PSJ9LCJjaXBoZXIiOnsibmFtZSI6Im5hY2wvc2VjcmV0Ym94
Iiwibm9uY2UiOiJFaVI1Wko2SThNbVRESjJPeVdBZXRZT3MvQmN6TnVTRiJ9LCJj
aXBoZXJ0ZXh0IjoiZG11TW1oOSs2cW4wQ1JzK3huRlp0TE9tOGtkVEtIVUVMbzZC
NElUakJJb2QwdWVHb3RLSW9VeDdTNGh0Vk5zNkRudmF1MmFGblJQVGpNN3pyVERT
SlYvZ05SVS80UHV0czlVZ04xc011bjRDdWUydnVFeEFxNmRTVmQ1dHhSTzIwVkE0
MEE5S3RBMlJPUUUrVTlPYUIzY0R0Y0x4THFxN2ljMndJR2FSQXlmSU1JTmJFVGFk
OC9qeE1BNTBwajZyTUtZdDRDQlpjcmRNcmc9PSJ9
-----END ENCRYPTED COSIGN PRIVATE KEY-----
```

## Ed25519 formats
One thing that I've run into is Ed25519 keys in pkcs8 version 1, which is
specified in [RFC-5208](https://www.rfc-editor.org/rfc/rfc5208), and version 2
[RFC-5958](https://www.rfc-editor.org/rfc/rfc5958). The issue is that OpenSSL
currently only supports version 1 (RFC-5208) and the openssl tools will not be
able to parse keys in the version 2 format (RFC-5958). The Rust ring crate uses
version 2 and trying to check those keys with openssl will not work.
Example of trying to use a version 2 formatted Ed25519 key:
```console
$ openssl pkey -inform der -in ed25519-1 -pubout
Could not read key from ed25519-1
```


