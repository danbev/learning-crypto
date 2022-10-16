## Digital Certificates
So certificates are basically a way to create a datastructure consisting of a
name and a public key, linking/mapping a name to a public key:
```rust
struct Certificate {
  name: String,
  pub_key: [u8],
}
```
The actual format will be in [ans.1](./asn1.md) but that is not important at the
moment. But if that was then only thing that a certificate consisted of then
I would be able to construct one my self claiming that some public key that
does not belong to me is mine. This would be like me creating my own passport
or drivers licence which can both be used to prove identity. So what happens
is that the data structure is signed by a thridparty, like a passport is issued
by the police, at least that is the case in Sweden.
The issuer/certificate authority is the entity that signes the data structure.
What this means is they would use their private key to sign this structure. If
we create a self signed certificate then we use our own private key with then
signature algorithm. A certifiate authority (CA) is similar in that it also
has a private key and a certificate, and it signs part of the certificate with
its private key, and the CA's certificate, the public key, can be used to
verify. The CA's certificate needs to be included in the trust stores and this
is how someone can verify the signature.

x.500 was first created in 1988 and is a directory service originally intendend
for phone books. So that spec was for mapping names to phone numbers. This was
so that telecommunication companies to have a global phone book thingy.
When we create a certificate, for example:
```console
$ openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365
```
We will be promted to enter a Distinguished Name (DN) like Country Name,
State or Province Name, Locality Name (city), Organization Name (company),
Comman Name(host/domain name), and email address. Things like Country Name,
and Locality are leftovers from the original spec and is why they have these
names.
The cert.pem file created is in Privacy Enhanced EMail format which is a base64
encoded payload between a header and a footer.
We can read this using openssl as well:
```console
$ openssl x509 -in cert.pem -inform=pem -text
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number:
            4e:ca:f7:5f:ca:8f:b8:86:ff:a0:52:19:d4:60:f5:38:d1:5d:51:db
        Signature Algorithm: sha256WithRSAEncryption
        Issuer: C = SE, L = Stockholm, CN = example.com, emailAddress = daniel.bevenius@gmail.com
        Validity
            Not Before: Oct 15 11:14:11 2022 GMT
            Not After : Oct 15 11:14:11 2023 GMT
        Subject: C = SE, L = Stockholm, CN = example.com, emailAddress = daniel.bevenius@gmail.com
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                RSA Public-Key: (4096 bit)
                Modulus:
                    00:c2:d4:75:12:1f:3a:a9:fd:42:9e:d5:00:8c:60:
                    d5:be:03:85:42:5f:c6:1e:44:1f:bb:9e:a8:f0:ee:
                    5b:53:1b:af:eb:9b:07:7c:88:6c:c0:a1:3c:d0:3b:
                    ec:33:b7:50:39:b5:24:82:c8:07:26:72:dc:86:13:
                    53:bb:1f:fa:6b:16:1a:e1:8d:5f:ac:63:ad:a8:fb:
                    2a:ab:e2:63:14:a0:e6:e9:41:1a:ba:50:f7:5f:af:
                    82:33:d8:6b:b5:8c:91:d5:0f:6f:a8:02:ac:6b:f1:
                    d3:ed:77:73:c7:f6:48:09:80:89:81:2e:df:a4:34:
                    e1:3f:7d:fc:1c:e5:b3:44:01:fa:b1:4c:01:b4:ba:
                    88:ed:a1:5f:e8:88:74:79:8d:c7:76:d9:26:39:d8:
                    4c:cd:a5:18:59:45:52:11:59:86:4f:06:93:cb:99:
                    ab:44:57:49:ea:79:b0:12:43:11:7b:ae:90:2d:76:
                    48:16:e9:2b:dd:e4:69:c6:2a:84:f9:9e:e5:23:a1:
                    56:a8:77:dd:0d:cd:2b:69:e7:42:56:bf:c9:d1:af:
                    37:c3:ac:d9:a2:a1:7c:d0:a1:59:53:5b:3c:a9:51:
                    b6:28:50:92:14:41:fc:81:3f:53:1f:bf:9e:2f:c2:
                    c1:1f:0e:2e:65:e4:82:77:0c:95:c2:6f:47:0f:48:
                    4e:36:b5:a2:9b:fe:d1:49:8d:6e:6a:17:c3:39:1f:
                    a6:e0:a9:2b:b1:bd:b9:36:56:0b:7d:20:0f:89:d4:
                    87:e4:e7:f0:f7:b6:24:4d:a1:cf:1d:6b:5c:64:91:
                    c8:08:02:91:d1:08:96:48:74:b7:88:5b:95:2a:7f:
                    d4:fb:0b:61:4e:d9:ba:7d:da:59:94:13:84:ad:82:
                    d5:39:63:ba:d4:58:f1:4a:11:0e:18:cc:f2:26:db:
                    13:82:2b:6b:7a:3e:c2:8e:70:18:a9:f3:43:f2:64:
                    33:20:61:ee:1a:37:cf:06:e2:8f:f8:37:06:dc:61:
                    30:77:60:50:8e:da:a5:df:41:99:a9:9a:2e:26:6e:
                    cd:ce:c4:56:f8:74:c9:c0:7a:5d:75:af:8f:22:f8:
                    46:ee:0f:39:5d:01:1c:8c:47:cc:76:cd:af:9c:53:
                    fe:8c:c4:f2:e0:ca:9e:86:14:76:9e:a4:d9:00:bb:
                    2a:4b:cb:4f:24:be:74:e5:a9:fc:3e:f6:b6:cf:88:
                    b0:bd:9b:ce:56:3c:8e:4b:c2:cc:9d:6d:dd:f9:f2:
                    78:37:f3:85:bb:4f:08:45:44:5a:eb:5a:20:75:30:
                    fa:24:0d:6f:c0:58:5a:71:45:87:6d:45:28:4a:b5:
                    cc:32:98:e5:e4:28:c3:95:41:cf:30:4a:51:7e:8f:
                    d1:43:77
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Subject Key Identifier: 
                D8:F6:8F:BB:A4:F8:79:2C:B5:F5:76:6C:02:35:5A:1E:0D:BE:3F:CF
            X509v3 Authority Key Identifier: 
                keyid:D8:F6:8F:BB:A4:F8:79:2C:B5:F5:76:6C:02:35:5A:1E:0D:BE:3F:CF

            X509v3 Basic Constraints: critical
                CA:TRUE
    Signature Algorithm: sha256WithRSAEncryption
         58:90:db:c2:6a:bf:82:93:89:f2:56:b6:25:c6:a2:12:bd:6e:
         f0:6a:be:d5:4c:10:60:7e:f4:75:da:4a:f5:a5:83:0b:26:98:
         e2:96:52:bf:9a:42:df:4b:17:3e:f2:f0:f3:ff:79:cf:a0:cb:
         34:45:c5:fa:fc:7c:36:2b:48:ab:0e:cb:ed:bf:c6:59:eb:d9:
         21:c8:ea:0d:38:b4:55:38:6f:b2:14:50:84:a5:d2:dd:fc:1e:
         0a:f4:a0:f1:aa:4f:3a:4b:9e:cf:b4:32:59:1e:e0:ac:93:34:
         5e:ea:9e:5d:5d:bc:84:ac:7c:d7:9b:7b:84:4c:98:22:98:9b:
         8f:6b:f9:b8:cd:3f:54:6c:a2:74:19:41:1a:e3:f0:7a:29:ac:
         74:d1:9b:7a:7e:f7:e8:f4:a6:c6:1c:f5:34:2b:87:72:50:e6:
         e6:0e:1b:12:20:78:a8:fb:f2:68:c7:6f:47:5f:52:e3:c5:49:
         4d:13:7e:9d:1a:6a:6c:e4:20:35:09:d3:cb:db:4d:8b:da:b8:
         09:82:6c:a2:58:c1:bf:d1:05:60:d7:85:cc:91:d7:25:36:9c:
         b3:f0:26:44:8f:80:8e:cb:10:7c:c2:3b:02:a1:fa:00:e9:ca:
         69:99:73:51:31:99:61:70:68:d1:34:68:99:7b:26:20:d7:f1:
         e5:57:78:2c:71:bf:d0:d1:3c:db:fd:0a:63:74:ae:5f:33:8e:
         2b:0d:7c:78:e3:61:f5:d4:cd:9b:c8:83:e3:8f:92:7c:16:93:
         59:11:2f:7f:72:2e:9d:14:5b:f5:13:ce:8a:ef:c1:e4:18:50:
         9e:28:d7:97:6b:20:d0:b2:81:67:99:72:dc:ee:b1:62:26:03:
         bc:64:04:9e:b6:d9:ea:98:1d:4b:2d:da:56:fb:d9:16:9b:e0:
         be:d0:a8:46:8f:c5:e7:70:1f:ed:ca:68:35:38:81:73:f1:a0:
         c8:f5:1d:2f:47:f8:ca:bc:08:bb:6b:9e:76:d5:90:6c:a7:72:
         be:c9:a4:3e:13:ec:44:b7:f3:7d:1f:7a:2c:d1:b0:24:b0:ca:
         50:72:76:9f:34:9a:a7:09:f0:12:b5:23:71:37:9b:23:4c:9e:
         db:43:d3:78:0a:9f:6f:2c:78:9c:ff:d5:17:88:9f:62:63:85:
         79:18:15:61:90:6e:d5:00:5b:69:7a:cd:88:72:95:e2:8c:20:
         af:f2:aa:6a:aa:ad:c4:0f:e8:a5:0a:c5:15:c8:d0:49:c6:f3:
         46:bc:c6:12:7f:75:3d:f1:72:bc:14:1a:df:18:4f:dd:c0:dd:
         a0:57:a5:4e:16:15:2c:dc:24:65:8f:52:55:5e:25:35:09:ee:
         cb:be:21:ff:83:a3:30:37
-----BEGIN CERTIFICATE-----
MIIFozCCA4ugAwIBAgIUTsr3X8qPuIb/oFIZ1GD1ONFdUdswDQYJKoZIhvcNAQEL
BQAwYTELMAkGA1UEBhMCU0UxEjAQBgNVBAcMCVN0b2NraG9sbTEUMBIGA1UEAwwL
ZXhhbXBsZS5jb20xKDAmBgkqhkiG9w0BCQEWGWRhbmllbC5iZXZlbml1c0BnbWFp
bC5jb20wHhcNMjIxMDE1MTExNDExWhcNMjMxMDE1MTExNDExWjBhMQswCQYDVQQG
EwJTRTESMBAGA1UEBwwJU3RvY2tob2xtMRQwEgYDVQQDDAtleGFtcGxlLmNvbTEo
MCYGCSqGSIb3DQEJARYZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTCCAiIwDQYJ
KoZIhvcNAQEBBQADggIPADCCAgoCggIBAMLUdRIfOqn9Qp7VAIxg1b4DhUJfxh5E
H7ueqPDuW1Mbr+ubB3yIbMChPNA77DO3UDm1JILIByZy3IYTU7sf+msWGuGNX6xj
raj7KqviYxSg5ulBGrpQ91+vgjPYa7WMkdUPb6gCrGvx0+13c8f2SAmAiYEu36Q0
4T99/Bzls0QB+rFMAbS6iO2hX+iIdHmNx3bZJjnYTM2lGFlFUhFZhk8Gk8uZq0RX
Sep5sBJDEXuukC12SBbpK93kacYqhPme5SOhVqh33Q3NK2nnQla/ydGvN8Os2aKh
fNChWVNbPKlRtihQkhRB/IE/Ux+/ni/CwR8OLmXkgncMlcJvRw9ITja1opv+0UmN
bmoXwzkfpuCpK7G9uTZWC30gD4nUh+Tn8Pe2JE2hzx1rXGSRyAgCkdEIlkh0t4hb
lSp/1PsLYU7Zun3aWZQThK2C1TljutRY8UoRDhjM8ibbE4Ira3o+wo5wGKnzQ/Jk
MyBh7ho3zwbij/g3BtxhMHdgUI7apd9BmamaLiZuzc7EVvh0ycB6XXWvjyL4Ru4P
OV0BHIxHzHbNr5xT/ozE8uDKnoYUdp6k2QC7KkvLTyS+dOWp/D72ts+IsL2bzlY8
jkvCzJ1t3fnyeDfzhbtPCEVEWutaIHUw+iQNb8BYWnFFh21FKEq1zDKY5eQow5VB
zzBKUX6P0UN3AgMBAAGjUzBRMB0GA1UdDgQWBBTY9o+7pPh5LLX1dmwCNVoeDb4/
zzAfBgNVHSMEGDAWgBTY9o+7pPh5LLX1dmwCNVoeDb4/zzAPBgNVHRMBAf8EBTAD
AQH/MA0GCSqGSIb3DQEBCwUAA4ICAQBYkNvCar+Ck4nyVrYlxqISvW7war7VTBBg
fvR12kr1pYMLJpjillK/mkLfSxc+8vDz/3nPoMs0RcX6/Hw2K0irDsvtv8ZZ69kh
yOoNOLRVOG+yFFCEpdLd/B4K9KDxqk86S57PtDJZHuCskzRe6p5dXbyErHzXm3uE
TJgimJuPa/m4zT9UbKJ0GUEa4/B6Kax00Zt6fvfo9KbGHPU0K4dyUObmDhsSIHio
+/Jox29HX1LjxUlNE36dGmps5CA1CdPL202L2rgJgmyiWMG/0QVg14XMkdclNpyz
8CZEj4COyxB8wjsCofoA6cppmXNRMZlhcGjRNGiZeyYg1/HlV3gscb/Q0Tzb/Qpj
dK5fM44rDXx442H11M2byIPjj5J8FpNZES9/ci6dFFv1E86K78HkGFCeKNeXayDQ
soFnmXLc7rFiJgO8ZASettnqmB1LLdpW+9kWm+C+0KhGj8XncB/tymg1OIFz8aDI
9R0vR/jKvAi7a5521ZBsp3K+yaQ+E+xEt/N9H3os0bAksMpQcnafNJqnCfAStSNx
N5sjTJ7bQ9N4Cp9vLHic/9UXiJ9iY4V5GBVhkG7VAFtpes2IcpXijCCv8qpqqq3E
D+ilCsUVyNBJxvNGvMYSf3U98XK8FBrfGE/dwN2gV6VOFhUs3CRlj1JVXiU1Ce7L
viH/g6MwNw==
-----END CERTIFICATE-----
```

### Critical/Non-Critical
You might see this certificates, for example:
```
Certificate Request:
        Data:
                Version: 0 (0x0)
                Subject: CN=test
                Subject Public Key Info:
                       Public Key Algorithm: rsaEncryption
                               Public-Key: (2048 bit)

        Attributes:
        Requested Extensions:
                X509v3 Key Usage: critical
                        Digital Signature
        X509v3 Extended Key Usage: critical
                Code Signing
``` 
Critical tells implementations that don't recognize the extension in question to
reject it, and non-critical is used if it is alright for the extension to be
ignore if the implementation does not recognize the extension.
