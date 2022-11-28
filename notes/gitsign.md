## gitsign
This document contains steps for setting up
[gitsign](https://github.com/sigstore/gitsign).


### Install gitsign
```console
$ go install github.com/sigstore/gitsign@latest
```
Or using brew:
```console
$ brew install sigstore/tap/gitsign
```

### Configure git
The collowing will configure signing for the current project:
```console
#!/bin/bash

# Sign all commits
git config --local commit.gpgsign true

# Sign all tags
git config --local tag.gpgsign true

# Use gitsign for signing
git config --local gpg.x509.program gitsign

# gitsign expects x509 args
git config --local gpg.format x509 
```
To configure for all projects, `global` use:
```console
#!/bin/bash

# Sign all commits
git config --global commit.gpgsign true

# Sign all tags
git config --global tag.gpgsign true

# Use gitsign for signing
git config --global gpg.x509.program gitsign

# gitsign expects x509 args
git config --global gpg.format x509 
```

### Commit
When commiting now gitsign will be used which will start an Open ID Connect
(OIDC) flow giving us the choice of a OIDC provider to use:
```console
$ git ci -v
Your browser will now be opened to:
https://oauth2.sigstore.dev/auth/auth?access_type=online&client_id=sigstore&code_challenge=eQvdw56pTgXnkj76Cab-4ZWaKk8XFM6UFFBdayKQX1Y&code_challenge_method=S256&nonce=2GmBDq86TMNuz8VhMUixMxiPSe2&redirect_uri=http%3A%2F%2Flocalhost%3A39617%2Fauth%2Fcallback&response_type=code&scope=openid+email&state=2GmBDlYDps5Ywd8dX4Ebwo4VnQL
[master 4292869] Add initial Oniro notes
 1 file changed, 10 insertions(+)
 create mode 100644 notes/oniro.md
```
Note that on github this commit will be marked as `Unverified` which is because
the sigstore Certificate Authority root is not part of Githubs trust root and
also because validation needs to be done using Rekor to verify that the
certificate was valid at the time this commit was signed.

To avoid having to choose a auth flow to use can be annoying and it can be
specified using an environment variable, for example:
```console
$ export GITSIGN_CONNECTOR_ID=https://github.com/login/oauth
```


### Verifying a commit
A commit can be verified using:
```console
$ git verify-commit 4292869
tlog index: 6058402
gitsign: Signature made using certificate ID 0xb073e00bfabd4fb9988b9e1e0896dcfc1527fcdb | CN=sigstore-intermediate,O=sigstore.dev
gitsign: Good signature from [daniel.bevenius@gmail.com]
Validated Git signature: true
Validated Rekor entry: true
```


### Inspect commit signature
```console
$ ./cat-gitsignature.sh 562022f
PKCS7: 
  type: pkcs7-signedData (1.2.840.113549.1.7.2)
  d.sign: 
    version: 1
    md_algs:
        algorithm: sha256 (2.16.840.1.101.3.4.2.1)
        parameter: <ABSENT>
    contents: 
      type: pkcs7-data (1.2.840.113549.1.7.1)
      d.data: <ABSENT>
    cert:
        cert_info: 
          version: 2
          serialNumber: 0x366CF888E30926DD7853B32EB1F96E638494E5B9
          signature: 
            algorithm: ecdsa-with-SHA384 (1.2.840.10045.4.3.3)
            parameter: <ABSENT>
          issuer: O=sigstore.dev, CN=sigstore-intermediate
          validity: 
            notBefore: Oct 28 19:15:59 2022 GMT
            notAfter: Oct 28 19:25:59 2022 GMT
          subject: 
          key: 
            algor: 
              algorithm: id-ecPublicKey (1.2.840.10045.2.1)
              parameter: OBJECT:prime256v1 (1.2.840.10045.3.1.7)
            public_key:  (0 unused bits)
              0000 - 04 14 60 4a 3b 32 3c b4-9a 1b e4 1c 2e 35   ..`J;2<......5
              000e - 41 8e 42 bc b6 7d 6a 9a-2f 8f 25 c5 14 0d   A.B..}j./.%...
              001c - e8 0d a6 10 ce 37 19 67-b8 f5 c1 ac 9a 35   .....7.g.....5
              002a - a9 e9 01 fb 0d ec 20 b2-40 05 24 37 eb 28   ...... .@.$7.(
              0038 - bb f2 53 be 6e 48 ce 4e-14                  ..S.nH.N.
          issuerUID: <ABSENT>
          subjectUID: <ABSENT>
          extensions:
              object: X509v3 Key Usage (2.5.29.15)
              critical: TRUE
              value: 
                0000 - 03 02 07 80                              ....

              object: X509v3 Extended Key Usage (2.5.29.37)
              critical: BOOL ABSENT
              value: 
                0000 - 30 0a 06 08 2b 06 01 05-05 07 03 03      0...+.......

              object: X509v3 Subject Key Identifier (2.5.29.14)
              critical: BOOL ABSENT
              value: 
                0000 - 04 14 17 08 bc 78 d0 83-ce 8d 73 92 6c   .....x....s.l
                000d - 32 4b 75 b6 02 29 c7 96-59               2Ku..)..Y

              object: X509v3 Authority Key Identifier (2.5.29.35)
              critical: BOOL ABSENT
              value: 
                0000 - 30 16 80 14 df d3 e9 cf-56 24 11 96 f9   0.......V$...
                000d - a8 d8 e9 28 55 a2 c6 2e-18 64 3f         ...(U....d?

              object: X509v3 Subject Alternative Name (2.5.29.17)
              critical: TRUE
              value: 
                0000 - 30 1b 81 19 64 61 6e 69-65 6c 2e 62 65   0...daniel.be
                000d - 76 65 6e 69 75 73 40 67-6d 61 69 6c 2e   venius@gmail.
                001a - 63 6f 6d                                 com

              object: undefined (1.3.6.1.4.1.57264.1.1)
              critical: BOOL ABSENT
              value: 
                0000 - 68 74 74 70 73 3a 2f 2f-67 69 74 68 75   https://githu
                000d - 62 2e 63 6f 6d 2f 6c 6f-67 69 6e 2f 6f   b.com/login/o
                001a - 61 75 74 68                              auth

              object: CT Precertificate SCTs (1.3.6.1.4.1.11129.2.4.2)
              critical: BOOL ABSENT
              value: 
                0000 - 04 79 00 77 00 75 00 08-60 92 f0 28 52   .y.w.u..`..(R
                000d - ff 68 45 d1 d1 6b 27 84-9c 45 67 18 ac   .hE..k'..Eg..
                001a - 16 3d c3 38 d2 6d e6 bc-22 06 36 6f 72   .=.8.m..".6or
                0027 - 00 00 01 84 20 05 c4 52-00 00 04 03 00   .... ..R.....
                0034 - 46 30 44 02 20 57 ab 3f-4f c7 d2 4d c8   F0D. W.?O..M.
                0041 - 19 5d 7d 92 f6 fc ad 9c-4e 72 82 51 3e   .]}.....Nr.Q>
                004e - 07 9a 08 59 90 fe 82 95-fe 76 e3 02 20   ...Y.....v.. 
                005b - 62 13 d3 17 10 ff 18 2b-30 06 6f bf c0   b......+0.o..
                0068 - f7 e3 84 6a b7 d5 af 1a-39 39 c4 59 f7   ...j....99.Y.
                0075 - 24 e8 3b 4e d0 3d                        $.;N.=
        sig_alg: 
          algorithm: ecdsa-with-SHA384 (1.2.840.10045.4.3.3)
          parameter: <ABSENT>
        signature:  (0 unused bits)
          0000 - 30 66 02 31 00 92 49 5b-cd a2 ac c0 f1 b5 e4   0f.1..I[.......
          000f - 88 aa c6 a4 48 3b f3 71-0a 8b f6 d2 9b 45 d7   ....H;.q.....E.
          001e - d4 87 c0 46 5a 37 ed 67-d8 b7 06 75 80 f3 a8   ...FZ7.g...u...
          002d - 12 ff b3 db 55 78 f8 36-02 31 00 95 ac 53 e0   ....Ux.6.1...S.
          003c - 8a c8 4a 77 3f ca 8a f1-b5 6c 57 4d 2c b3 7a   ..Jw?....lWM,.z
          004b - ba 42 86 45 bf 91 c0 b2-e4 9a c9 1a 44 5d 14   .B.E........D].
          005a - 7f 10 26 69 67 15 a9 e9-41 90 6d 7a 9b ae      ..&ig...A.mz..
    crl:
      <ABSENT>
    signer_info:
        version: 1
        issuer_and_serial: 
          issuer: O=sigstore.dev, CN=sigstore-intermediate
          serial: 0x366CF888E30926DD7853B32EB1F96E638494E5B9
        digest_alg: 
          algorithm: sha256 (2.16.840.1.101.3.4.2.1)
          parameter: <ABSENT>
        auth_attr:
            object: contentType (1.2.840.113549.1.9.3)
            set:
              OBJECT:pkcs7-data (1.2.840.113549.1.7.1)

            object: signingTime (1.2.840.113549.1.9.5)
            set:
              UTCTIME:Oct 28 19:15:59 2022 GMT

            object: messageDigest (1.2.840.113549.1.9.4)
            set:
              OCTET STRING:
                0000 - 1f 6c 5a 8d c7 81 42 ab-74 f6 f0 fb 44   .lZ...B.t...D
                000d - 5d 30 f3 72 ec 47 57 4c-05 07 a1 7e ea   ]0.r.GWL...~.
                001a - f4 30 d2 93 ea 89                        .0....
        digest_enc_alg: 
          algorithm: ecdsa-with-SHA256 (1.2.840.10045.4.3.2)
          parameter: <ABSENT>
        enc_digest: 
          0000 - 30 44 02 20 12 c8 06 f4-f2 f1 dd fd ec 65 41   0D. .........eA
          000f - cb 52 18 31 08 2d a9 d2-19 0f 8c d0 89 3d 1c   .R.1.-.......=.
          001e - f9 fe fb e7 c6 92 02 20-31 c1 ba d0 6a ef 62   ....... 1...j.b
          002d - dd 19 70 83 16 3c b2 33-1c 93 b4 2d 1d 1d 58   ..p..<.3...-..X
          003c - c7 8f e9 5c 23 a6 dd 4a-99 bc                  ...\#..J..
        unauth_attr:
          <ABSENT>
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number:
            36:6c:f8:88:e3:09:26:dd:78:53:b3:2e:b1:f9:6e:63:84:94:e5:b9
        Signature Algorithm: ecdsa-with-SHA384
        Issuer: O=sigstore.dev, CN=sigstore-intermediate
        Validity
            Not Before: Oct 28 19:15:59 2022 GMT
            Not After : Oct 28 19:25:59 2022 GMT
        Subject: 
        Subject Public Key Info:
            Public Key Algorithm: id-ecPublicKey
                Public-Key: (256 bit)
                pub:
                    04:14:60:4a:3b:32:3c:b4:9a:1b:e4:1c:2e:35:41:
                    8e:42:bc:b6:7d:6a:9a:2f:8f:25:c5:14:0d:e8:0d:
                    a6:10:ce:37:19:67:b8:f5:c1:ac:9a:35:a9:e9:01:
                    fb:0d:ec:20:b2:40:05:24:37:eb:28:bb:f2:53:be:
                    6e:48:ce:4e:14
                ASN1 OID: prime256v1
                NIST CURVE: P-256
        X509v3 extensions:
            X509v3 Key Usage: critical
                Digital Signature
            X509v3 Extended Key Usage: 
                Code Signing
            X509v3 Subject Key Identifier: 
                17:08:BC:78:D0:83:CE:8D:73:92:6C:32:4B:75:B6:02:29:C7:96:59
            X509v3 Authority Key Identifier: 
                keyid:DF:D3:E9:CF:56:24:11:96:F9:A8:D8:E9:28:55:A2:C6:2E:18:64:3F

            X509v3 Subject Alternative Name: critical
                email:daniel.bevenius@gmail.com
            1.3.6.1.4.1.57264.1.1: 
                https://github.com/login/oauth
            CT Precertificate SCTs: 
                Signed Certificate Timestamp:
                    Version   : v1 (0x0)
                    Log ID    : 08:60:92:F0:28:52:FF:68:45:D1:D1:6B:27:84:9C:45:
                                67:18:AC:16:3D:C3:38:D2:6D:E6:BC:22:06:36:6F:72
                    Timestamp : Oct 28 19:15:59.698 2022 GMT
                    Extensions: none
                    Signature : ecdsa-with-SHA256
                                30:44:02:20:57:AB:3F:4F:C7:D2:4D:C8:19:5D:7D:92:
                                F6:FC:AD:9C:4E:72:82:51:3E:07:9A:08:59:90:FE:82:
                                95:FE:76:E3:02:20:62:13:D3:17:10:FF:18:2B:30:06:
                                6F:BF:C0:F7:E3:84:6A:B7:D5:AF:1A:39:39:C4:59:F7:
                                24:E8:3B:4E:D0:3D
    Signature Algorithm: ecdsa-with-SHA384
         30:66:02:31:00:92:49:5b:cd:a2:ac:c0:f1:b5:e4:88:aa:c6:
         a4:48:3b:f3:71:0a:8b:f6:d2:9b:45:d7:d4:87:c0:46:5a:37:
         ed:67:d8:b7:06:75:80:f3:a8:12:ff:b3:db:55:78:f8:36:02:
         31:00:95:ac:53:e0:8a:c8:4a:77:3f:ca:8a:f1:b5:6c:57:4d:
         2c:b3:7a:ba:42:86:45:bf:91:c0:b2:e4:9a:c9:1a:44:5d:14:
         7f:10:26:69:67:15:a9:e9:41:90:6d:7a:9b:ae
-----BEGIN CERTIFICATE-----
MIICqDCCAi2gAwIBAgIUNmz4iOMJJt14U7MusfluY4SU5bkwCgYIKoZIzj0EAwMw
NzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl
cm1lZGlhdGUwHhcNMjIxMDI4MTkxNTU5WhcNMjIxMDI4MTkyNTU5WjAAMFkwEwYH
KoZIzj0CAQYIKoZIzj0DAQcDQgAEFGBKOzI8tJob5BwuNUGOQry2fWqaL48lxRQN
6A2mEM43GWe49cGsmjWp6QH7DewgskAFJDfrKLvyU75uSM5OFKOCAUwwggFIMA4G
A1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUFwi8
eNCDzo1zkmwyS3W2AinHllkwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y
ZD8wJwYDVR0RAQH/BB0wG4EZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTAsBgor
BgEEAYO/MAEBBB5odHRwczovL2dpdGh1Yi5jb20vbG9naW4vb2F1dGgwgYkGCisG
AQQB1nkCBAIEewR5AHcAdQAIYJLwKFL/aEXR0WsnhJxFZxisFj3DONJt5rwiBjZv
cgAAAYQgBcRSAAAEAwBGMEQCIFerP0/H0k3IGV19kvb8rZxOcoJRPgeaCFmQ/oKV
/nbjAiBiE9MXEP8YKzAGb7/A9+OEarfVrxo5OcRZ9yToO07QPTAKBggqhkjOPQQD
AwNpADBmAjEAkklbzaKswPG15IiqxqRIO/NxCov20ptF19SHwEZaN+1n2LcGdYDz
qBL/s9tVePg2AjEAlaxT4IrISnc/yorxtWxXTSyzerpChkW/kcCy5JrJGkRdFH8Q
JmlnFanpQZBtepuu
-----END CERTIFICATE-----
```

* How can we enforce the that PRs are signed
* Can we resign ammed commits for users pull requests.


### gitsign-credential-cache
First install `gitsign-credential-cache` if it is not already installed:
```console
$ go install github.com/sigstore/gitsign/cmd/gitsign-credential-cache@latest
```

Create a file named `~/.config/systemd/user/gitsign.service`:
```console
[Unit]
Description=Gitsign Credentials Cache
Documentation=https://github.com/sigstore/gitsign

[Service]
Type=simple
ExecStart=%h/go/bin/gitsign-credential-cache

Restart=on-failure

[Install]
WantedBy=default.target 
```
This service can then be enabled using:
```console
$ systemctl --user daemon-reload
$ systemctl --user enable gitsign.service 
Created symlink /home/danielbevenius/.config/systemd/user/default.target.wants/gitsign.service → /home/danielbevenius/.config/systemd/user/gitsign.service.
```
And we can start it manually using:
```console
$ systemctl --user start gitsign.service
```

Check that it has started successfully:
```console
$ systemctl --user status gitsign.service 
● gitsign.service - Gitsign Credentials Cache
     Loaded: loaded (/home/danielbevenius/.config/systemd/user/gitsign.service; enabled; vendor preset: disabled)
     Active: active (running) since Mon 2022-11-28 11:27:47 CET; 2min 35s ago
       Docs: https://github.com/sigstore/gitsign
   Main PID: 177444 (gitsign-credent)
     CGroup: /user.slice/user-1000.slice/user@1000.service/app.slice/gitsign.service
             └─ 177444 /home/danielbevenius/go/bin/gitsign-credential-cache

Nov 28 11:27:47 localhost.localdomain systemd[1295]: Started Gitsign Credentials Cache.
Nov 28 11:27:47 localhost.localdomain gitsign-credential-cache[177444]: /home/danielbevenius/.cache/.sigstore/gitsig>
```
And we then need to add the following environment variable:
```console
$ export GITSIGN_CREDENTIAL_CACHE=~/.cache/.sigstore/gitsign/cache.sock
```
After this we should be able to commit a first time and have our credentials
stored and the following commit should be done without a browser popup.
