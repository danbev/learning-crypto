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
$ podman run -d -p 5000:5000 --restart always --name registry registry:2
```
Next we sign and create a cosign bundle:
```console
$ cd firmare-project
$ COSIGN_EXPERIMENTAL=1 cosign sign-blob --bundle=firmware.bundle --output-certificate=firmware.crt --output-signature=firmware.sig firmware.bin
Using payload from: firmware.bin
Generating ephemeral keys...
Retrieving signed certificate...

        Note that there may be personally identifiable information associated with this signed artifact.
        This may include the email address associated with the account with which you authenticate.
        This information will be used for signing this artifact and will be stored in public transparency logs and cannot be removed later.
        By typing 'y', you attest that you grant (or have permission to grant) and agree to have this information stored permanently in transparency logs.

Are you sure you want to continue? (y/[N]): y
Your browser will now be opened to:
https://oauth2.sigstore.dev/auth/auth?access_type=online&client_id=sigstore&code_challenge=ao4hhCGg2h3x65uTWJPr_SzCdj0-0-0ETx3fqy3-95Q&code_challenge_method=S256&nonce=2H50snpcyLJWl0H24FoNF4QSrGp&redirect_uri=http%3A%2F%2Flocalhost%3A34013%2Fauth%2Fcallback&response_type=code&scope=openid+email&state=2H50spy70kRa5scsX9E5navr2Ak
Successfully verified SCT...
using ephemeral certificate:
-----BEGIN CERTIFICATE-----
MIICqDCCAi+gAwIBAgIUcdPJurmkcpKbYJvw4llJfmeRkQkwCgYIKoZIzj0EAwMw
NzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl
cm1lZGlhdGUwHhcNMjIxMTA0MTEwNDExWhcNMjIxMTA0MTExNDExWjAAMFkwEwYH
KoZIzj0CAQYIKoZIzj0DAQcDQgAEVhufiqvnK1pifvBA+v1vQrhEz4mmDfqD/5BY
nCRrC4f9DN9N1hHKc8RbmoLp+mhxh1lpf4JAn+F22fM5o79q+qOCAU4wggFKMA4G
A1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUihNf
HdNegQqUXxGdVP1pgH22G4AwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y
ZD8wJwYDVR0RAQH/BB0wG4EZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTAsBgor
BgEEAYO/MAEBBB5odHRwczovL2dpdGh1Yi5jb20vbG9naW4vb2F1dGgwgYsGCisG
AQQB1nkCBAIEfQR7AHkAdwDdPTBqxscRMmMZHhyZZzcCokpeuN48rf+HinKALynu
jgAAAYRCUATNAAAEAwBIMEYCIQDLjHRPecK/Q9UP1IsP/Dy5Nyv65TJZSZo1hh2s
FmLXdwIhAJKluazS9EoSEkM8Quv51WEE2FdAx9j8PWtP+wkqJMDCMAoGCCqGSM49
BAMDA2cAMGQCMDkY9u32X3pzMmJaCWoPsJbIrulRe9SoAIQlNmyeR1/XeeQkhcG0
L2rEp7gnIoG+yQIwd+VSyer0EZtORiw1OcAojnWUaS2+CWQomnCIDAlkHCuI2Ntc
DYOL9ycu6/pVYntj
-----END CERTIFICATE-----

tlog entry created with index: 6485149
Bundle wrote in the file firmware.bundle
Signature wrote in the file firmware.sig
Certificate wrote in the file firmware.crt
```
And lets make sure that we can verify this the binary:
```console
$ cosign verify-blob --bundle=firmware.bundle firmware.bin
tlog entry verified offline
Verified OK
```

Now we build an example firmware image:
```console
$ cd drogue-ajour/firmware-project
$ podman build -f Dockerfile .
```

Then we can push this to the registry we started above:
```console
$ oras push -v localhost:5000/firmware-bundle:latest firmware-project:application/octet-stream firwmare.bundle:application/json
Preparing firmware-project
Uploading cc514a6172b4 firmware-project
Uploading 44136fa355b3 application/vnd.unknown.config.v1+json
Uploaded  cc514a6172b4 firmware-project
Uploaded  44136fa355b3 application/vnd.unknown.config.v1+json
Uploading 4d8efc9e414c application/vnd.oci.image.manifest.v1+json
Uploaded  4d8efc9e414c application/vnd.oci.image.manifest.v1+json
Pushed localhost:5000/firmware-bundle:latest
Digest: sha256:4d8efc9e414c892a856215f4f360210c2f5fe29560c9edb3f9997be6885ee0a6
```

We can fetch the manifest using:
```console
$  oras manifest fetch localhost:5000/firmware-bundle:latest | jq
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
      "digest": "sha256:d0960116cb5e16fe2603c424690d09adbcecd8d69a74f015399dfdf57130efb3",
      "size": 94808,
      "annotations": {
        "io.deis.oras.content.digest": "sha256:cd1ca4e7e885be45b28939aeec9970ef70a46d97eb5465a417ad6722e72b81f9",
        "io.deis.oras.content.unpack": "true",
        "org.opencontainers.image.title": "firmware-project"
      }
    },
    {
      "mediaType": "application/json",
      "digest": "sha256:3b091317f2a6ee9d31d3367e8a9f787d834ca7c4eb212a58523c43e9f65d9183",
      "size": 3886,
      "annotations": {
        "org.opencontainers.image.title": "firmware.bundle"
      }
    }
  ]
}
```
This would make the cosign bundle needed for verification a layer in the image.

I've tried to attach the the bundle using oras but running into an issue when
trying to do this. 
First we push a new image, this time without the firmware.bundle:
```console
$ oras manifest fetch localhost:5000/firmware-bundle3:latest
{"schemaVersion":2,"mediaType":"application/vnd.oci.image.manifest.v1+json","config":{"mediaType":"application/vnd.oras.config.v1+json","digest":"sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855","size":0},"layers":[{"mediaType":"application/octet-stream","digest":"sha256:d0960116cb5e16fe2603c424690d09adbcecd8d69a74f015399dfdf57130efb3","size":94808,"annotations":{"io.deis.oras.content.digest":"sha256:cd1ca4e7e885be45b28939aeec9970ef70a46d97eb5465a417ad6722e72b81f9","io.deis.oras.content.unpack":"true","org.opencontainers.image.title":"firmware-project"}}]}$ oras manifest fetch localhost:5000/firmware-bundle3:latest | jq
{
  "schemaVersion": 2,
  "mediaType": "application/vnd.oci.image.manifest.v1+json",
  "config": {
    "mediaType": "application/vnd.oras.config.v1+json",
    "digest": "sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
    "size": 0
  },
  "layers": [
    {
      "mediaType": "application/octet-stream",
      "digest": "sha256:d0960116cb5e16fe2603c424690d09adbcecd8d69a74f015399dfdf57130efb3",
      "size": 94808,
      "annotations": {
        "io.deis.oras.content.digest": "sha256:cd1ca4e7e885be45b28939aeec9970ef70a46d97eb5465a417ad6722e72b81f9",
        "io.deis.oras.content.unpack": "true",
        "org.opencontainers.image.title": "firmware-project"
      }
    }
  ]
}
```
And the we try to attach the bundle:
```
$ oras attach -v localhost:5000/firmware-bundle3:latest firmware.bundle:application/json --artifact-type cosign/bundle
Preparing firmware.bundle
Exists    3b091317f2a6 firmware.bundle
Uploading 131526905df7 application/vnd.cncf.oras.artifact.manifest.v1+json
Error: PUT "http://localhost:5000/v2/firmware-bundle3/manifests/sha256:131526905df7741cc7af933d5dc92c807caffccd284cddd8dbb01629b4c4da82": unexpected status code 400: manifest invalid: manifest invalid
```
_work in progress_
