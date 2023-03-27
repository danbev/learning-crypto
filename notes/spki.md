## Subject Public Key Info (SPKI)
In section [Section 4.1.2.7](https://datatracker.ietf.org/doc/html/rfc5280#section-4-1-2-7)
we can find out that this is a type that contains the public key and the
algorithm the key is to be used with. This can be RSA, DSA, DH.

ASN1 definition:
```
SubjectPublicKeyInfo ::= SEQUENCE {
  algorithm AlgorithmIdentifier,
  publicKey BIT STRING
}

AlgorithmIdentifier  ::=  SEQUENCE  {
  algorithm  OBJECT IDENTIFIER,
  parameters ANY DEFINED BY algorithm OPTIONAL
}
```

Example:
```console
$ openssl ecparam -name prime256v1 -genkey -noout -out ec.pem
$ openssl ec -in ec.pem -pubout -out ec.pub
$ openssl ec -in ec.pem -pubout -outform der -out ec.pub.der
$ openssl ec -inform der -pubin -in ec.pub.der -noout -text
$ openssl pkcs8 -in ec.pem -outform der -out ec.pk8.der -topk8 -nocrypt
```
Now, if we inspect the public key we extracted above:
```console
$ cat ec.pub
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEaFsFXHjIjir1RcW4/ssV9CyyEuZO
S16AJx5H8DVuPJHSoFPcEdxUJBaw9I7bjreIh040NZGV6yO/RBdU3u4enQ==
-----END PUBLIC KEY-----
```
Notice that it starts with `BEGIN PUBLIC KEY` which indicates that this is a
x509 Subject Public Key Info. So we should be able check the generated ans.1
format for this using:
```console
$ openssl asn1parse -in ec.pub -inform pem -i
    0:d=0  hl=2 l=  89 cons: SEQUENCE          
    2:d=1  hl=2 l=  19 cons:  SEQUENCE          
    4:d=2  hl=2 l=   7 prim:   OBJECT            :id-ecPublicKey
   13:d=2  hl=2 l=   8 prim:   OBJECT            :prime256v1
   23:d=1  hl=2 l=  66 prim:  BIT STRING
```
This output matches the above.

```console
$ openssl ec -inform der -pubin -in ec.pub.der -noout -text
read EC key
Public-Key: (256 bit)
pub:
    04:68:5b:05:5c:78:c8:8e:2a:f5:45:c5:b8:fe:cb:
    15:f4:2c:b2:12:e6:4e:4b:5e:80:27:1e:47:f0:35:
    6e:3c:91:d2:a0:53:dc:11:dc:54:24:16:b0:f4:8e:
    db:8e:b7:88:87:4e:34:35:91:95:eb:23:bf:44:17:
    54:de:ee:1e:9d
ASN1 OID: prime256v1
NIST CURVE: P-256
```
We can check and compare the asn1 syntax for both the pem and the der public
key using:
```console
$ openssl asn1parse -in ec.pub -inform pem -i
    0:d=0  hl=2 l=  89 cons: SEQUENCE          
    2:d=1  hl=2 l=  19 cons:  SEQUENCE          
    4:d=2  hl=2 l=   7 prim:   OBJECT            :id-ecPublicKey
   13:d=2  hl=2 l=   8 prim:   OBJECT            :prime256v1
   23:d=1  hl=2 l=  66 prim:  BIT STRING        
$ openssl asn1parse -in ec.pub.der -inform der -i
    0:d=0  hl=2 l=  89 cons: SEQUENCE          
    2:d=1  hl=2 l=  19 cons:  SEQUENCE          
    4:d=2  hl=2 l=   7 prim:   OBJECT            :id-ecPublicKey
   13:d=2  hl=2 l=   8 prim:   OBJECT            :prime256v1
   23:d=1  hl=2 l=  66 prim:  BIT STRING
```
So when we see `spki`.
