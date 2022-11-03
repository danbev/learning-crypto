## Signing and verifying firmware
The purpose of this document is to describe how firmware can be signed and
verified using Drogue IoT.

### Background
Currently firmware can be stored in a OCI registry but is not currently signed
in Drogue IoT. So we first need to sign the firmware before/after it has been
pushed to an OCI registry.

We also need to take into consideration that verifying a normal artifacts like
a container usually reach out to transparency log (Rekor) to perform the
verification process. This is probably not going to work for many types of
constrained IoT devices.

Using "stapled inclusion proofs" we verify an object is present in the
transparency log without having to contact the transparency log iself. But the
signer needs to collect the information from the log and present it along with
the artifact and signature.

### Firmware Signing
So we have to create a Sigstore/Cosign `bundle` to be added to stored with/in
the artifact in the OCI repository.

Lets, pretent that we want to sign a binary firmware file, but in this example
we will just be using a text file for testing purposes:
```console
$ echo "firmware..." > firmware.txt
$ shasum -a 256 firmware.txt 
a23356201583455aa0d7c7340c3fbad04587285ddd89a3e5110c071998733233  firmware.txt
```

Next, we can sign the file and tell cosign to generate a `bundle` for it:
```console
$ COSIGN_EXPERIMENTAL=1 cosign sign-blob --bundle=firmware.bundle --output-certificate=firmware.crt --output-signature=firmare.sig firmware.txt
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

When we create the container image that our firmware is part of we will need to
add a layer to this image. This new layer will contain the bundle. By doing this
I think the bundle will be made available to the bootloader application on the
device which can then use that information to verify the firmware itself.

