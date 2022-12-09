## JSON Web Encryption

### Structure
The JWE contains 5 sections, a header, JWE Encrypted Key, an IV, cipher text,
and an authenictaion tag.

```
base64(header).base64(Encrypted Key).base64(IV).base64(Cipher Text).base64(authentication tag)
                     [-------------]                   [----------]
                      Encrypted with assymetric key   Encrypted with symmectric key
                                                      from the Encrypted Key field
```
Notice that the header is not encrypted but the JWE Encrypted Key is. The
header will provide information about the algorithm used and the encoding. So
the actual key in the JWE is a symmectric key but it is protected by an
assymetric key.


Key encryption options:
* RSA with PKCS #1 v1.5 padding   (asymetrical encryption)
* RSA with OAEP padding           (asymetrical encryption)
* ECHD-ES (Ephemeral Static)      (asymetrical encryption)
* AES-GCM                         (symetrical encryption)

ECHD-ES is vulnerable to [invalid curve attacks] which allows an attacker to
send JWE tokens and because they can specify both the x and y coordinates
somehow they can steel the private key. TODO: read up on how this actually
works.


[invalid curve attacks]: https://nvd.nist.gov/vuln/detail/CVE-2017-16007
