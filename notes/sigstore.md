## sigstore
One of the main values of sigstore is to sign/validate/verify artifacts, like
thirdparty software dependencies, to avoid supply chain attacks. It has not been
very easy to sign code/modules/containers etc before which has led to people
simply not doing it. It is the software and also the maintaince of keys which
adds makes this difficult.

sigstore is a project under the CNFC and it goal is to provide a
software-signing equivalent to `Let's Encrypt`. It is not just one tool but a
collection of tools namely; `fulico`, `rekor`, and `cosign`. 

With sigstore we don't have to manage private keys, and it makes it simpler to
handle revocation

### Installation
```console
$ go install github.com/sigstore/cosign/cmd/cosign@latest
```

### Fulcio
Is a root CA for code signing certs and issues code-signing certificates.
Based on an OpenID Connect email address, Fulcio signs X.509 certificates valid
for 10 minutes.

## Rekor
Is the transparency log which is immutable, append only log which can be used
other parties to check what was signed and by whom.
It has a restful API server, https://rekor.sigstore.dev.
Example of retrieving a log entry can be done using:
```
$ curl -s https://rekor.sigstore.dev/api/v1/log/entries?logIndex=3321511 | jq
```

```console
$ go install -v github.com/sigstore/rekor/cmd/rekor-cli@latest
```

### cosign
Is a container signing tool and storage in an OCI registry.

Signing steps:
* A keypair for codesigning is generated.
* The user authenticates to an OpenID Connect Provider (OIDC) to verify the ownership of their email address.
* Upon successful authentication a code-signing cert is received.
* The code-signing cert is published to Rekor, the transparency log.
* User signs an artifact using the certificatate and the privat key from the keypair.
* The signature from the signed artifact is published to Rekor.
* The keypair used are deleted
* The signed artifact can be published.

#### Trust Root
To verify the identity of a system we need to ask that system to present us
with credentials. And we then need to verify those credentials. To do this we
need to have something at the beginning of this flow that we trust can verify
these credentials. In WebPKI there are Root Certificate Authorities (CA) which
are built into browsers and OS's. A browser will check that a website has the
identity it claims by checking that it can be chained back to one of these
Root CA's.
In sigstores case the root trust allows users and systems to automatically
retreive digital certificates that prove who they are, and they use these
certs to sign artifacts that they produce and distribute. The users of these
artifacts can verify the signatures and certificates against the trust root.

### OpenID Connect
This is used for authentication.

### Signing JavaScript
First we have to install https://github.com/sigstore/sigstore-js and npm link
it:
```console
$ git clone https://github.com/sigstore/sigstore-js
$ npm i && npm r build
$ npm link
```


Next we have to generate the artifact to be signed:
```console
$ cd sigstore/npm-example
$ npm link sigstore
$ npm pack
```
This will generate a file named something like `npm-example-1.0.0.tgz`.
We can now sign this artifact using `sigstore`:
```console
$ which sigstore 
~/.nvm/versions/node/v18.4.0/bin/sigstore

$ sigstore sign ./npm-example-1.0.0.tgz > signature
```
The above command will open a browser and ask for the Sigstore OAuth page. I
used GitHub as the identity provider to be used.
We can now inspect the generated `signature` file that was the output of the
above command:
```console
$ cat signature | jq
{
  "attestationType": "attestation/blob",
  "attestation": {
    "payloadHash": "4f2fb5c9d049ebad8d3d25f55ffdf49b290f7f84f56b89fe3d6663b48b856ad1",
    "payloadHashAlgorithm": "sha256",
    "signature": "MEYCIQDPnJgOdxFnx4Sb6XZRBnnThGfTGGFBgA6ZnQ/YE15GygIhAI9mlq+gAvSfSv/ilA0l90ogR2HSxntOQrd8FIsKMrS5"
  },
  "certificate": "LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNwekNDQWk2Z0F3SUJBZ0lVYlltS0wranVScVRjV2Y5bjRCdnErMUZnM3kwd0NnWUlLb1pJemowRUF3TXcKTnpFVk1CTUdBMVVFQ2hNTWMybG5jM1J2Y21VdVpHVjJNUjR3SEFZRFZRUURFeFZ6YVdkemRHOXlaUzFwYm5SbApjbTFsWkdsaGRHVXdIaGNOTWpJd09ESTVNRGt3TURNeFdoY05Nakl3T0RJNU1Ea3hNRE14V2pBQU1Ga3dFd1lICktvWkl6ajBDQVFZSUtvWkl6ajBEQVFjRFFnQUVNWWtwT0NibFRIUURxT2tqeDNwRFo1QnRIZkZPRlFCeittcnkKeGpseDRyb2xpTU0vVGd6ZlVoUkhrSE9BUTJDL2NHZzRwSGRVeWRib0lnMGZzK2RSMTZPQ0FVMHdnZ0ZKTUE0RwpBMVVkRHdFQi93UUVBd0lIZ0RBVEJnTlZIU1VFRERBS0JnZ3JCZ0VGQlFjREF6QWRCZ05WSFE0RUZnUVU5bXlHCmZOWjErRzNJcysvcnJYaDVMOENabk5zd0h3WURWUjBqQkJnd0ZvQVUzOVBwejFZa0VaYjVxTmpwS0ZXaXhpNFkKWkQ4d0p3WURWUjBSQVFIL0JCMHdHNEVaWkdGdWFXVnNMbUpsZG1WdWFYVnpRR2R0WVdsc0xtTnZiVEFzQmdvcgpCZ0VFQVlPL01BRUJCQjVvZEhSd2N6b3ZMMmRwZEdoMVlpNWpiMjB2Ykc5bmFXNHZiMkYxZEdnd2dZb0dDaXNHCkFRUUIxbmtDQkFJRWZBUjZBSGdBZGdBSVlKTHdLRkwvYUVYUjBXc25oSnhGWnhpc0ZqM0RPTkp0NXJ3aUJqWnYKY2dBQUFZTG8xTHNmQUFBRUF3QkhNRVVDSVFENktGd1dOcE0yNS8ySDFCVExsZnhXU3owVDBFS0g5cGZFRVFkTQpQdEQ2OVFJZ1hLZnR0OXhQNjdBaUJKOHhlY3kwOFpHZUlwdzJubkl1M083K1B0WDZObUV3Q2dZSUtvWkl6ajBFCkF3TURad0F3WkFJd0VFM3NWVFhXNFBWeWZtMGx6dlZnVG1pclNMSzJzZmRtdzdrTmhKVVY5YjNTTVJ3N250UDgKVElJNDZSWHBIVHJWQWpCbTZCVGg2MTNHZVBzeXdHNUtDMkwyVjBRZS8wK2p0MkJuQjQ4M3JwTW5XcnVoaVNxYwpBVnR1UU95SlBWSko0anc9Ci0tLS0tRU5EIENFUlRJRklDQVRFLS0tLS0KLS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNHakNDQWFHZ0F3SUJBZ0lVQUxuVmlWZm5VMGJySmFzbVJrSHJuL1VuZmFRd0NnWUlLb1pJemowRUF3TXcKS2pFVk1CTUdBMVVFQ2hNTWMybG5jM1J2Y21VdVpHVjJNUkV3RHdZRFZRUURFd2h6YVdkemRHOXlaVEFlRncweQpNakEwTVRNeU1EQTJNVFZhRncwek1URXdNRFV4TXpVMk5UaGFNRGN4RlRBVEJnTlZCQW9UREhOcFozTjBiM0psCkxtUmxkakVlTUJ3R0ExVUVBeE1WYzJsbmMzUnZjbVV0YVc1MFpYSnRaV1JwWVhSbE1IWXdFQVlIS29aSXpqMEMKQVFZRks0RUVBQ0lEWWdBRThSVlMveXNIK05PdnVEWnlQSVp0aWxnVUY5TmxhcllwQWQ5SFAxdkJCSDFVNUNWNwo3TFNTN3MwWmlING5FN0h2N3B0UzZMdnZSL1NUazc5OExWZ016TGxKNEhlSWZGM3RIU2FleExjWXBTQVNyMWtTCjBOL1JnQkp6LzlqV0NpWG5vM3N3ZVRBT0JnTlZIUThCQWY4RUJBTUNBUVl3RXdZRFZSMGxCQXd3Q2dZSUt3WUIKQlFVSEF3TXdFZ1lEVlIwVEFRSC9CQWd3QmdFQi93SUJBREFkQmdOVkhRNEVGZ1FVMzlQcHoxWWtFWmI1cU5qcApLRldpeGk0WVpEOHdId1lEVlIwakJCZ3dGb0FVV01BZVg1RkZwV2FwZXN5UW9aTWkwQ3JGeGZvd0NnWUlLb1pJCnpqMEVBd01EWndBd1pBSXdQQ3NRSzREWWlaWURQSWFEaTVIRktuZnhYeDZBU1NWbUVSZnN5bllCaVgyWDZTSlIKblpVODQvOURaZG5GdnZ4bUFqQk90NlFwQmxjNEovMER4dmtUQ3FwY2x2emlMNkJDQ1BuamRsSUIzUHUzQnhzUApteWdVWTdJaTJ6YmRDZGxpaW93PQotLS0tLUVORCBDRVJUSUZJQ0FURS0tLS0tCi0tLS0tQkVHSU4gQ0VSVElGSUNBVEUtLS0tLQpNSUlCOXpDQ0FYeWdBd0lCQWdJVUFMWk5BUEZkeEhQd2plRGxvRHd5WUNoQU8vNHdDZ1lJS29aSXpqMEVBd013CktqRVZNQk1HQTFVRUNoTU1jMmxuYzNSdmNtVXVaR1YyTVJFd0R3WURWUVFERXdoemFXZHpkRzl5WlRBZUZ3MHkKTVRFd01EY3hNelUyTlRsYUZ3MHpNVEV3TURVeE16VTJOVGhhTUNveEZUQVRCZ05WQkFvVERITnBaM04wYjNKbApMbVJsZGpFUk1BOEdBMVVFQXhNSWMybG5jM1J2Y21Vd2RqQVFCZ2NxaGtqT1BRSUJCZ1VyZ1FRQUlnTmlBQVQ3ClhlRlQ0cmIzUFFHd1M0SWFqdExrMy9PbG5wZ2FuZ2FCY2xZcHNZQnI1aSs0eW5CMDdjZWIzTFAwT0lPWmR4ZXgKWDY5YzVpVnV5SlJRK0h6MDV5aStVRjN1QldBbEhwaVM1c2gwK0gyR0hFN1NYcmsxRUM1bTFUcjE5TDlnZzkyagpZekJoTUE0R0ExVWREd0VCL3dRRUF3SUJCakFQQmdOVkhSTUJBZjhFQlRBREFRSC9NQjBHQTFVZERnUVdCQlJZCndCNWZrVVdsWnFsNnpKQ2hreUxRS3NYRitqQWZCZ05WSFNNRUdEQVdnQlJZd0I1ZmtVV2xacWw2ekpDaGt5TFEKS3NYRitqQUtCZ2dxaGtqT1BRUURBd05wQURCbUFqRUFqMW5IZVhacCsxM05XQk5hK0VEc0RQOEcxV1dnMXRDTQpXUC9XSFBxcGFWbzBqaHN3ZU5GWmdTczBlRTd3WUk0cUFqRUEyV0I5b3Q5OHNJa29GM3ZaWWRkMy9WdFdCNWI5ClROTWVhN0l4L3N0SjVUZmNMTGVBQkxFNEJOSk9zUTR2bkJISgotLS0tLUVORCBDRVJUSUZJQ0FURS0tLS0t",
  "signedEntryTimestamp": "MEUCIQCUmBbbcNVQZheORGqGbdz4PBajob1+j0p8nFbie5mUYAIgaL2tFFnMyGoFecxAnmbW1ZTFRGe1VjuUH4dh/c7fdQE=",
  "integratedTime": 1661763632,
  "logID": "c0d23d6ad406973f9559f3ba2d1ca01f84147d8ffc5b8445c224f98b9591801d",
  "logIndex": 3305621
}


$ cat signature | jq --raw-output '.certificate' | base64 -d | openssl x509 \
  -text -out signingcert.pem
```

Verify the artifact:
```console
$ sigstore verify npm-example-1.0.0.tgz signature signingcert.pem
Verified OK
```

Now, in the sigstore-js README.md there is a reference to the Rekor url
that looks like it was "supposed" to be in the sign commands output. I was not
able to find this and opened https://github.com/sigstore/sigstore-js/pull/68 to
address this.

But if we have the url we can get entry inforation from Rekor:
```console

$ curl --silent https://rekor.sigstore.dev/api/v1/log/entries/c0d23d6ad406973f9559f3ba2d1ca01f84147d8ffc5b8445c224f98b9591801d \
  | jq --raw-output '.[].body' \
  | base64 --decode \
  | jq
```

_work in progress_
