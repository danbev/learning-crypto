## gitsign


### Install gitsign
```console
$ go install github.com/sigstore/gitsign@latest
```


### Configure git
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

