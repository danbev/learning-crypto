## Signing and verifying firmware
The purpose of this document is to describe how firmware can be signed and
verified using Drogue IoT.

### Background
Currently firmware can be stored in a OCI registry but is not currently signed
in Drogue IoT. So we first need to sign the firmware before/after it has been
pushed to an OCI registry.

We also need to take into consideration that verifying a normal artifacts like
a container usually reaches out to transparency log (Rekor) to perform the
verification process. This is probably not going to work for many types of
constrained IoT devices and is something we need to take into consideration.

Using "stapled inclusion proofs" we verify that an object is present in the
transparency log without having to contact the transparency log iself. But the
signer needs to collect the information from the log and present it along with
the artifact and signature.

### Firmware Signing
So we have to create a Sigstore/Cosign `bundle`, which contains all the
information required for "stapled inclusion proofs", to be added to stored
with/in the artifact in the OCI repository.

Lets say that we want to sign a binary firmware file, but in this example
we will just be using a text file for testing purposes:
```console
$ echo "firmware..." > firmware.txt
$ shasum -a 256 firmware.txt 
a23356201583455aa0d7c7340c3fbad04587285ddd89a3e5110c071998733233  firmware.txt
```

Next, we can sign the file and tell cosign to generate a `bundle` for it:
```console
$ COSIGN_EXPERIMENTAL=1 cosign sign-blob --bundle=firmware.bundle \
     --output-certificate=firmware.crt \
     --output-signature=firmare.sig firmware.txt
Using payload from: firmware.txt
Generating ephemeral keys...
Retrieving signed certificate...

        Note that there may be personally identifiable information associated with this signed artifact.
        This may include the email address associated with the account with which you authenticate.
        This information will be used for signing this artifact and will be stored in public transparency logs and cannot be removed later.
        By typing 'y', you attest that you grant (or have permission to grant) and agree to have this information stored permanently in transparency logs.

Are you sure you want to continue? (y/[N]): y
Your browser will now be opened to:
https://oauth2.sigstore.dev/auth/auth?access_type=online&client_id=sigstore&code_challenge=uYlNB90xX0ghydmExLgE2bm31-4D70IdHP_pG0tiyQA&code_challenge_method=S256&nonce=2H2UVhi4asnPXj8uQDokAaWoDtb&redirect_uri=http%3A%2F%2Flocalhost%3A33099%2Fauth%2Fcallback&response_type=code&scope=openid+email&state=2H2UVnD08KC1yNTkUMC06KIfUvO
Successfully verified SCT...
using ephemeral certificate:
-----BEGIN CERTIFICATE-----
MIICqDCCAi2gAwIBAgIUC9rsTS/JaO7KF9uYKhVUxJhM4vswCgYIKoZIzj0EAwMw
NzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl
cm1lZGlhdGUwHhcNMjIxMTAzMTMzODIzWhcNMjIxMTAzMTM0ODIzWjAAMFkwEwYH
KoZIzj0CAQYIKoZIzj0DAQcDQgAE2B7YGgF/Pq13/zzUYlsxpxa5ag21HEBOI1sZ
EBTurQE4TGCKwS5LJFtpzYyPJ+RUt0skbspgwHJv2ziGrOuXPqOCAUwwggFIMA4G
A1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQU8rqv
jaMJk7A1jDX0fnyfGc6XOfgwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y
ZD8wJwYDVR0RAQH/BB0wG4EZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTAsBgor
BgEEAYO/MAEBBB5odHRwczovL2dpdGh1Yi5jb20vbG9naW4vb2F1dGgwgYkGCisG
AQQB1nkCBAIEewR5AHcAdQDdPTBqxscRMmMZHhyZZzcCokpeuN48rf+HinKALynu
jgAAAYQ9ttaBAAAEAwBGMEQCIHFA2mvgiu3kwWbui38o6mMrODYpEmQ90LhSjhfc
7p6yAiAkwUqo5ssLbTHb968JCmOIbOzGaRhI1AENjme9vl1GvzAKBggqhkjOPQQD
AwNpADBmAjEAl53sWJai3xVbsT4udfI4aGMd1Y/2zIgaS1YaRpsYSV6q3zX21GuQ
6GrWyj2B0nyXAjEAt+TobKHLYkNjO/oDYbKy0as0tt0P52DuVzJA6v/yMsmYgbje
EyrnO4K+7WqEPsIL
-----END CERTIFICATE-----

tlog entry created with index: 6425825
Bundle wrote in the file firmware.bundle
Signature wrote in the file firmare.sig
Certificate wrote in the file firmware.crt
```

And to make verify using this information:
```console
$ cosign verify-blob --bundle=firmware.bundle firmware.txt
tlog entry verified offline
Verified OK
```

The size of the bundle is around 4K:
```console
$ ls -lh firmware.bundle 
-rw-------. 1 danielbevenius danielbevenius 3.8K Nov  3 14:38 firmware.bundle
```
Is the size reasonable to be downloaded and stored on the device?

When we create the container image that our firmware is part of we can add a
layer to this image. This new layer will contain the bundle. By doing this
I think the bundle will be available to the bootloader application on the
device which can then use that information to verify the firmware itself.

Another approach would be to attach the bundle, or just the signature, to the
container image in the OCI registry. 

We will explore both options.


### Adding the bundle to the container image
This section will take a look at the option of adding the bundle as a layer in
the container image.

First we start a registry that we can push to:
```console
$ cd firmware-project
$ make start-registry
```

Next we are going to create a container image that will contain our firmware
binary. In the directory [firmware-project](../firmware-project) there is
`firmware.bin` file.

```console
$ cd firmare-project
$ make sign
env COSIGN_EXPERIMENTAL=1 cosign sign-blob  \
	--bundle=firmware.bundle \
       	--output-certificate=firmware.crt \
	--output-signature=firmware.sig firmware.bin
Using payload from: firmware.bin
Generating ephemeral keys...
Retrieving signed certificate...

        Note that there may be personally identifiable information associated with this signed artifact.
        This may include the email address associated with the account with which you authenticate.
        This information will be used for signing this artifact and will be stored in public transparency logs and cannot be removed later.
        By typing 'y', you attest that you grant (or have permission to grant) and agree to have this information stored permanently in transparency logs.

Are you sure you want to continue? (y/[N]): y
Your browser will now be opened to:
https://oauth2.sigstore.dev/auth/auth?access_type=online&client_id=sigstore&code_challenge=mlqDCbFmSVpR8byKQpPII-uoHQQDJichLQyKKk2KdwQ&code_challenge_method=S256&nonce=2HiCXP82DnDvPCgitAu93l8LNUV&redirect_uri=http%3A%2F%2Flocalhost%3A46181%2Fauth%2Fcallback&response_type=code&scope=openid+email&state=2HiCXQE2GeqtivAEKteGKUfSuzo
Successfully verified SCT...
using ephemeral certificate:
-----BEGIN CERTIFICATE-----
MIICqTCCAi6gAwIBAgIUDtSHi+Koa+UyC0uQlYYS+RTLie0wCgYIKoZIzj0EAwMw
NzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl
cm1lZGlhdGUwHhcNMjIxMTE4MDgwMjQwWhcNMjIxMTE4MDgxMjQwWjAAMFkwEwYH
KoZIzj0CAQYIKoZIzj0DAQcDQgAEOBX0ubvUeRWfN08IfAneAiTmH+0Jf47T5Twf
NhtHC3zeLtgJyo2Hwye5ejp7mVExD7qDV+d0iwtyXgc6TIp6+6OCAU0wggFJMA4G
A1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUZ6mm
52qHlbjQKWDUn44x6Xhm8UMwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y
ZD8wJwYDVR0RAQH/BB0wG4EZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTAsBgor
BgEEAYO/MAEBBB5odHRwczovL2dpdGh1Yi5jb20vbG9naW4vb2F1dGgwgYoGCisG
AQQB1nkCBAIEfAR6AHgAdgDdPTBqxscRMmMZHhyZZzcCokpeuN48rf+HinKALynu
jgAAAYSJwt2uAAAEAwBHMEUCIQDZB0FPXO209VgD3xKK3so0J8ltKk/UyZXCEI+O
nVr0JAIgIN3yoVkBKG6zoWckJcnnIFbCKgSEZmrjcqzbJFDdRwQwCgYIKoZIzj0E
AwMDaQAwZgIxAO+waCgYSjjNBF4mO0AB3xWFunhJOtlzI73X4p0z7nsldmIe9qU0
87rzATciGvts/wIxAN/uq4reVujKsqx1VhPugJlXw+JXgk8NyUOWILRmwAQUQ4Hz
o2aYGtPDZ5Jk39+7hw==
-----END CERTIFICATE-----

tlog entry created with index: 7328763
Bundle wrote in the file firmware.bundle
Signature wrote in the file firmware.sig
Certificate wrote in the file firmware.crt
```

And lets make sure that we can verify this the binary:
```console
$ make verify
env COSIGN_EXPERIMENTAL=1 cosign verify-blob --bundle=firmware.bundle \
	firmware.bin
tlog entry verified offline
Verified OK
```

Now we build an example firmware image:
```console
$ make image
podman build -t firmware-project -f Dockerfile .
STEP 1/3: FROM scratch
STEP 2/3: COPY firmware.bin /firmware
--> Using cache db77fc7243dad74547df0ce57943c544c53765b7c42b418439120dae7fe2e4c7
--> db77fc7243d
STEP 3/3: COPY firmware.json /metadata.json
--> Using cache c0c5aa8c939bf20d6bbfd1424f6bc0e6106e6565ff51b56449740fb914bf5d5d
COMMIT firmware-project
--> c0c5aa8c939
Successfully tagged localhost/firmware-project:latest
c0c5aa8c939bf20d6bbfd1424f6bc0e6106e6565ff51b56449740fb914bf5d5d
```

Then we can push the firmware.bin, firmware.json, and firmware.bundle to
the resistry using oras:
```console
$ make push
oras push -v localhost:5000/firmware-project:latest \
       firmware.bin:application/octet-stream \
       firmware.json:application/octet-stream \
       firmware.bundle:application/json
Preparing firmware.bin
Preparing firmware.json
Preparing firmware.bundle
Uploading b8d96f286798 firmware.bin
Uploading 073c6a85e1ce firmware.json
Uploading 61f31d217518 firmware.bundle
Uploading 44136fa355b3 application/vnd.unknown.config.v1+json
Uploaded  b8d96f286798 firmware.bin
Uploaded  073c6a85e1ce firmware.json
Uploaded  44136fa355b3 application/vnd.unknown.config.v1+json
Uploaded  61f31d217518 firmware.bundle
Uploading 7089bbcc00ba application/vnd.oci.image.manifest.v1+json
Uploaded  7089bbcc00ba application/vnd.oci.image.manifest.v1+json
Pushed localhost:5000/firmware-project:latest
Digest: sha256:7089bbcc00bab84bb4744a21bf70ac150283059c1513aa16f5fa77ca01f8fa2d
```

We can fetch the manifest using:
```console
$ make fetch 
oras manifest fetch localhost:5000/firmware-project:latest | jq
{
  "schemaVersion": 2,
  "mediaType": "application/vnd.oci.image.manifest.v1+json",
  "config": {
    "mediaType": "application/vnd.unknown.config.v1+json",
    "digest": "sha256:44136fa355b3678a1146ad16f7e8649e94fb4fc21fe77e8310c060f61caaff8a",
    "size": 2
  },
  "layers": [
    {
      "mediaType": "application/octet-stream",
      "digest": "sha256:b8d96f286798d368d00b2a9ebee5efbf4f37b0a4928a0d1e2926c4fd09e4d43d",
      "size": 122700,
      "annotations": {
        "org.opencontainers.image.title": "firmware.bin"
      }
    },
    {
      "mediaType": "application/octet-stream",
      "digest": "sha256:073c6a85e1cef707187e20ad6aafcff2d40c0ccb2c7fc32ce876cafea8170adb",
      "size": 127,
      "annotations": {
        "org.opencontainers.image.title": "firmware.json"
      }
    },
    {
      "mediaType": "application/json",
      "digest": "sha256:61f31d217518ab7a6a6a659079cef347d76df241f6fd862c53e2a28334d91464",
      "size": 3906,
      "annotations": {
        "org.opencontainers.image.title": "firmware.bundle"
      }
    }
  ]
}
```
This would make the cosign bundle needed for verification a layer in the image.

And we can pull the image and see what what it contains:
```console
$ make pull
oras pull -o pulled-images localhost:5000/firmware-project:latest
Downloading b8d96f286798 firmware.bin
Downloading 61f31d217518 firmware.bundle
Downloading 073c6a85e1ce firmware.json
Downloaded  073c6a85e1ce firmware.json
Downloaded  61f31d217518 firmware.bundle
Downloaded  b8d96f286798 firmware.bin
Pulled localhost:5000/firmware-project:latest
Digest: sha256:7089bbcc00bab84bb4744a21bf70ac150283059c1513aa16f5fa77ca01f8fa2d

Pulled images to pulled-images/
firmware.bin  firmware.bundle  firmware.json
```


### Attaching the bundle to the image
I've tried to attach the the bundle using oras but running into an issue when
trying to do this. 

First we push a new image, this time just the firmware.bin:
```console
$ make push-single 
oras push -v localhost:5000/firmware-project-single:latest \
       firmware.bin:application/octet-stream
Preparing firmware.bin
Uploading b8d96f286798 firmware.bin
Uploading 44136fa355b3 application/vnd.unknown.config.v1+json
Uploaded  b8d96f286798 firmware.bin
Uploaded  44136fa355b3 application/vnd.unknown.config.v1+json
Uploading cf2b20c1fcff application/vnd.oci.image.manifest.v1+json
Uploaded  cf2b20c1fcff application/vnd.oci.image.manifest.v1+json
Pushed localhost:5000/firmware-project-single:latest
Digest: sha256:cf2b20c1fcff5f5734c1df31634caa40420ad76b7b619e723509053f37289c68
```
And we can verify that this container image only contains the firmware.bin:
```console
$ make fetch-single 
oras manifest fetch localhost:5000/firmware-project-single:latest | jq
{
  "schemaVersion": 2,
  "mediaType": "application/vnd.oci.image.manifest.v1+json",
  "config": {
    "mediaType": "application/vnd.unknown.config.v1+json",
    "digest": "sha256:44136fa355b3678a1146ad16f7e8649e94fb4fc21fe77e8310c060f61caaff8a",
    "size": 2
  },
  "layers": [
    {
      "mediaType": "application/octet-stream",
      "digest": "sha256:b8d96f286798d368d00b2a9ebee5efbf4f37b0a4928a0d1e2926c4fd09e4d43d",
      "size": 122700,
      "annotations": {
        "org.opencontainers.image.title": "firmware.bin"
      }
    }
  ]
}
```

And the we try to attach the bundle:
```
$ make attach-bundle 
oras attach localhost:5000/firmware-project-single:latest \
       --artifact-type=application/json firmware.bundle
Uploading 61f31d217518 firmware.bundle
Uploaded  61f31d217518 firmware.bundle
Error: PUT "http://localhost:5000/v2/firmware-project-single/manifests/sha256:17fedd3cc0ddd822b37fe98b62b4b5cd212e0dc8bce0c6edef80e1734b02559a": unexpected status code 400: manifest invalid: manifest invalid
make: *** [Makefile:43: attach-bundle] Error 1
```
I'm currently using this version oras
```console
$ oras version
Version:        0.15.1
Go version:     go1.19
Git commit:     36dc01d02c54bf410d56d5e16b597eedbe666ec6
Git tree state: clean
```
There is a later version available, and I'll try updating in case that may help:
```console
$ oras version
Version:        0.16.0
Go version:     go1.19.2
Git commit:     d606fed4be252fd6162f63548e024451c31f3864
Git tree state: clean
```
And lets try the attach command again:
```console
$ make attach-bundle 
oras attach localhost:5000/firmware-project-single:latest \
       --artifact-type=application/json firmware.bundle
Exists    61f31d217518 firmware.bundle
Attached to localhost:5000/firmware-project-single@sha256:cf2b20c1fcff5f5734c1df31634caa40420ad76b7b619e723509053f37289c68
Digest: sha256:aefd8d54134812098c8662a6ee971d3d70f6ed9708e3efb2ef7fb268b2530a4d
```
Yay, that worked this time.

So what do we get if we pull this image (expecting only the single file):
```console
$ make pull-single 
oras pull -o pulled-images localhost:5000/firmware-project-single:latest
Downloading b8d96f286798 firmware.bin
Downloaded  b8d96f286798 firmware.bin
Pulled localhost:5000/firmware-project-single:latest
Digest: sha256:cf2b20c1fcff5f5734c1df31634caa40420ad76b7b619e723509053f37289c68

Pulled images to pulled-images/
firmware.bin
```
If we look closer at the output of `oras attach` command above it says it
attached the attachment to
```
Attached to localhost:5000/firmware-project-single@sha256:cf2b20c1fcff5f5734c1df31634caa40420ad76b7b619e723509053f37289c68
```
I wonder if we can pull that file using the `Digest`:
```console
$ make pull-attachment 
oras pull -o pulled-images localhost:5000/firmware-project-single@sha256:aefd8d54134812098c8662a6ee971d3d70f6ed9708e3efb2ef7fb268b2530a4d
Downloading 61f31d217518 firmware.bundle
Downloading b8d96f286798 firmware.bin
Downloaded  61f31d217518 firmware.bundle
Downloaded  b8d96f286798 firmware.bin
Pulled localhost:5000/firmware-project-single@sha256:aefd8d54134812098c8662a6ee971d3d70f6ed9708e3efb2ef7fb268b2530a4d
Digest: sha256:aefd8d54134812098c8662a6ee971d3d70f6ed9708e3efb2ef7fb268b2530a4d

Pulled images to pulled-images/
firmware.bin  firmware.bundle

Verify the binary using the bundle
env COSIGN_EXPERIMENTAL=1 cosign verify-blob \
	--bundle=pulled-images/firmware.bundle \
	pulled-images/firmware.bin
tlog entry verified offline
Verified OK
```

### Adding the bundle as a reference
First we need to log to `ghcr.io`:
```console
$ echo $PAT | podman login ghcr.io --username <github_username> --password-stdin
```
Where `PAT` is a personal access token.

And we need to start an registry that has support for ORAS artifacts:
```console
$ make start-oras-artifact-support-registry
```
After that we can push an image which just contains firmware.bin using:
```console
$ make push-single
```

And then we can push the firmware.bundle using
```console
$ make push-reference 
oras push -v localhost:5000/firmware-project-single:bundle \
--artifact-type 'signature/example' \
--subject localhost:5000/firmware-project-single:latest \
./firmware.bundle:application/json
Preparing firmware.bundle
Uploading 61f31d217518 firmware.bundle
WARN[0000] reference for unknown type: application/json  digest="sha256:61f31d217518ab7a6a6a659079cef347d76df241f6fd862c53e2a28334d91464" mediatype=application/json size=3906
Pushed localhost:5000/firmware-project-single:bundle
Digest: sha256:21c44ab9bc8d50b2ac94bd02c371200f9174197c133ac625c79c56dcc3fab4e3
```

And we can use `oras discover` to find/show/discover the artifact reference:
```console
$ make discover 
oras discover -o tree localhost:5000/firmware-project-single:latest
localhost:5000/firmware-project-single:latest
└── signature/example
    └── sha256:21c44ab9bc8d50b2ac94bd02c371200f9174197c133ac625c79c56dcc3fab4e3
```

We can pull a reference by using the above digest:
```console
$ make pull-reference 

Digest of reference: sha256:21c44ab9bc8d50b2ac94bd02c371200f9174197c133ac625c79c56dcc3fab4e3

oras pull -a -o ./pulled-ref localhost:5000/firmware-project-single@`oras discover -o json --artifact-type 'signature/example' localhost:5000/firmware-project-single:latest | jq -r ".references[0].digest"`

Downloaded 61f31d217518 firmware.bundle
Pulled localhost:5000/firmware-project-single@sha256:21c44ab9bc8d50b2ac94bd02c371200f9174197c133ac625c79c56dcc3fab4e3
Digest: sha256:21c44ab9bc8d50b2ac94bd02c371200f9174197c133ac625c79c56dcc3fab4e3

Ref downloaded to pulled-ref/
firmware.bundle
```

### Alternatives
So, as I see it there are at least three ways for making the signing material
(like the sigstore/cosing bundle.json above) accociated with an image:

* [Adding the bundle to the container image](#adding-the-bundle-to-the-container-image)
* [Attaching the bundle to the image](#attaching-the-bundle-to-the-image)
* [Adding the bundle as a reference](#adding-the-bundle-as-a-reference)


_work in progress_

### Tasks
* Create a tekton task that can sign the firmware binary using Cosign
* Create a tekton task that will attach the bundle to the container image that
holds the firmware binary, or add the bundle as a reference.
* Update whatever code that is responsible for pulling the image to also pull
the bundle attachment/reference.
* Update whatever code that is responsible for sending the data to the firmware
to be updated to include the bundle. 
* Update the drogue-device FirmwareManager to be able to recieve the bundle and
verify that the firmware binary to be updated passes verification.

### Questions
* Can sigstore-rs be used on a embedded device?


