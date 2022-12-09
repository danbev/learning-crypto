### Public-Key Cryptography Standards
Is not like I thought a standard but a complete set of standards all which all
have the same name but with different versions.

#### PKCS#1
Is for `RSA` encryption/decryption, encoding/padding, verifying/signing
signatures.

#### PKCS#2
Was withdrawn.

#### PKCS#3
Diffie-Hellman Key Agreement Standard.

#### PKCS#4
Was withdrawn.

#### PKCS#5
Password-based Encryption Standard like PBKDF1 and PBKDF2.

#### PKCS#6
Extended-Certificate Syntax Standard, which defines the old v1 X.509 and is now
obselete by v3.

#### PKCS#7
Cryptographic Message Syntax Standard which is used to sign and/or encrypt
messages under a Public Key Infrastructure.

#### PKCS#8
Private-Key Information Syntax Specifiction which is used to carry private
certificate keypairs (encrypted/unencrypted).

Is a standard for storing private key information and the key may be encrypted
with a passphrase using PKCS#5 above.
These private keys are typcially exchanged in PEM base-64-encoded format.

Spec: https://tools.ietf.org/html/rfc5208

#### PKCS#9
Selected Attribute Types

#### PKCS#10
Certificate Request Standard.

#### PKCS#11
Cryptographic Token Interface.


#### PKCS#12
Personal Information Exchange Syntax Standard

#### PKCS#13
Elliptic-Curve Cryptography Standard.
Abandoned?

