### Verify Bundle notes
This document contains notes taken while adding a verify example to sigstore-rs
which uses a bundle created by cosign.

First we need to generate the bundle, which can be done with the following
command (using Go cosign):
```console
$ echo something > artifact.txt
$ COSIGN_EXPERIMENTAL=1 cosign sign-blob --bundle=artifact.bundle artifact.txt
```
And we can make sure that we are able for verify this blob using cosign (again
Go cosign and not sigstore-rs):
```
$ cosign verify-blob --bundle=artifact.bundle artifact.txt
```

### Existing verify example
Now, after looking at the existing example that verifies, we can see that it
uses a VerificationConstraintVec:
```rust
    // Build verification constraints
    let mut verification_constraints: VerificationConstraintVec = Vec::new();
```
Which is a type defined in `src/cosign/verification_constraint/mod.rs`:
```rust
/// A list of references to objects implementing the [`VerificationConstraint`] trait
pub type VerificationConstraintRefVec<'a> = Vec<&'a Box<dyn VerificationConstraint>>;

pub trait VerificationConstraint: std::fmt::Debug {
    /// Given the `signature_layer` object, return `true` if the verification
    /// check is satisfied.
   ...
   fn verify(&self, signature_layer: &SignatureLayer) -> Result<bool>;
}
```
So this verifies something that is a SignatureLayer, lets take a closer look at
it. This is a struct defined in `src/cosign/signature_layers.rs`
```rust
pub struct SignatureLayer {
    pub simple_signing: SimpleSigning,
    pub oci_digest: String,
    pub certificate_signature: Option<CertificateSignature>,
    pub bundle: Option<Bundle>,
    pub signature: Option<String>,
    pub raw_data: Vec<u8>,
}
```
`SimpleSigning` is a rust implementation of Red Hat's
[Simple Signing Spec](https://www.redhat.com/en/blog/container-image-signing).

So each signature will have a SimpleSigning which in json format looks like
this:
```json
{
    "critical": {
           "identity": {
               "docker-reference": "testing/manifest"
           },
           "image": {
               "Docker-manifest-digest": "sha256:20be...fe55"
           },
           "type": "atomic container signature"
    },
    "optional": {
           "creator": "atomic",
           "timestamp": 1458239713
    }
}
```
The `Docker-manifest-digest` is a hash of the manifest file of the container
image that we want to sign.

This json is then canonicalized, and we use our private key to sign this payload
(the SimpleSigning "json" that is) and that signature is then base64 encoded
so that it can be stored. So where is it stored?  It is stored in a new
container image named the same as the container that we are signing, but with
the manifest digest of the image added to the image name, and a `.sig` suffix.
For example:
```console
$ crane manifest ttl.sh/danbev-simple-container:sha256-96d13e1500053d6f21aee389b74c5826b3192cda9dd226a6026cef0474a351da.sig | jq
{                                                                               
  "schemaVersion": 2,                                                           
  "mediaType": "application/vnd.oci.image.manifest.v1+json",                    
  "config": {                                                                   
    "mediaType": "application/vnd.oci.image.config.v1+json",                    
    "size": 248,                                                                
    "digest": "sha256:fca11d85342bd4bde3708cd2712dec318322852e5d1e220729356e0c6478a5bd"
  },                                                                            
  "layers": [                                                                   
    {                                                                           
      "mediaType": "application/vnd.dev.cosign.simplesigning.v1+json",          
      "size": 246,                                                              
      "digest": "sha256:754122687a83f6ed95dfd06e354238ec7c3805d5910f77fee0469d624d0abe81",
      "annotations": {                                                          
        "dev.cosignproject.cosign/signature": "MEUCIDF7Q/9GP7PxzcWL0C5V0ocu4LHRhBBAWYKVitwMfhyBAiEA2yKFfyva7aSuq5zuAvoDOrsF0PNjtZzwoJVm4Wn2Usg="
      }                                                                         
    }                                                                           
  ]                                                                             
}     
```

The SimpleSigning struct looks like this in sigstore-rs:
```
pub struct SimpleSigning {
    pub critical: Critical,
    pub optional: Option<Optional>,
}

pub struct Critical {
    pub identity: Identity,
    pub image: Image,
    #[serde(rename = "type")]
    pub type_name: String,
}

pub struct Identity {
    pub docker_reference: String,
}

pub struct Image {
    pub docker_manifest_digest: String,
}
```

In the existing verify example the image name will be passed to triangulate:
```console
    let image: &str = cli.image.as_str();
    let (cosign_signature_image, source_image_digest) = client.triangulate(image, auth).await?;
```
This function is what provides the image manifest digest (source_image_digest)
mentioned previously, and also the `cosign_signature_image` which is the image
that holds the signature created by cosign. For more details about how this
works see [Container Image Signing](https://github.com/danbev/learning-crypto/blob/main/notes/sigstore.md#continer-image-signing).
So that will reach out to the OCI registry and get `cosign_signature_image`,
which is the image that contains the signature, and the `source_image_digest` is
the digest of the container (a manifest) to be verified I think.

So back to SignatureLayer, after the simple_signing field we have an oci_digest
then an optional certificate_signature, an optional bundle, and optional
signature, and finally a raw_data field.

This data is then passed to:
```rust
    let trusted_layers = client
        .trusted_signature_layers(auth, &source_image_digest, &cosign_signature_image)
        .await?;
```

```rust
    async fn trusted_signature_layers(
        &mut self,
        auth: &Auth,
        source_image_digest: &str,
        signature_image: &str,
    ) -> Result<Vec<SignatureLayer>> {
        // first the manifest and layers are fetched for the .sig container image
        let (manifest, layers) = self.fetch_manifest_and_layers(auth, signature_image).await?;
        // Checks that the manifest is of Image manifest and not an ImageIndex
        let image_manifest = match manifest {
            oci_distribution::manifest::OciManifest::Image(im) => im,
            oci_distribution::manifest::OciManifest::ImageIndex(_) => {
                return Err(SigstoreError::RegistryPullManifestError {
                    image: signature_image.to_string(),
                    error: "Found a OciImageIndex instead of a OciImageManifest".to_string(),
                });
            }
        };

        let sl = build_signature_layers(
            &image_manifest,              // the .sig manifest
            source_image_digest,          // the digest of the image to be verified
            &layers,                      // the layers of the .sig container image
            self.rekor_pub_key.as_ref(),  
            self.fulcio_cert_pool.as_ref(),
        )?;

        debug!(signature_layers=?sl, ?signature_image, "trusted signature layers");
        Ok(sl)
    }
```
The signature_layers will later be passed to `verify_constraints`:
```rust
                let filter_result = sigstore::cosign::verify_constraints(
                    &trusted_layers,
                    verification_constraints.iter(),
                );
```

A VerificationConstraint can be added to a VerificationConstraintVec like this:
```rust
    let bundle_verifier = BundleVerifier::try_from(&bundle_json, rekor_pub_key)?;
    let mut verification_constraints: VerificationConstraintVec = Vec::new();
    verification_constraints.push(Box::new(bundle_verifier));
```
So the verification constraint will be passed a SignatureLayer which it
can use to verify. 

### Offline blob verification
If we take a closer look at the bundle payload. First, when we use cosign's
sign-blob which will ouput the signature of the blob:
```console
Bundle wrote in the file artifact.bundle
MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU=
```

Lets first take a look at the `artifact.bundle`:
```console
$ cat artifact.bundle | jq
{
  "base64Signature": "MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU=",
  "cert": "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFL1lKVGs0M2RGOUJZWUlKV1BKWDlSYytCSGhQNgpHRVJaNFRqa2tCOWwvdnBIVTZSRTJnU1QxcnpBcEUyN3pCWEVXTWVyZzRGNHdsTXA4WjNxbXdsdDlnPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg==",
  "rekorBundle": {
    "SignedEntryTimestamp": "MEQCIAPZVWW0hqsRsy/oymge/6FSJz5ghL++h7kx3Hx0ERysAiB4ydjcdx888b2M9g2IkoEIY+37l8eUSVTUCYNp5uJoEQ==",
    "Payload": {
      "body": "eyJhcGlWZXJzaW9uIjoiMC4wLjEiLCJraW5kIjoiaGFzaGVkcmVrb3JkIiwic3BlYyI6eyJkYXRhIjp7Imhhc2giOnsiYWxnb3JpdGhtIjoic2hhMjU2IiwidmFsdWUiOiI0YmM0NTNiNTNjYjNkOTE0YjQ1ZjRiMjUwMjk0MjM2YWRiYTJjMGUwOWZmNmYwMzc5Mzk0OWU3ZTM5ZmQ0Y2MxIn19LCJzaWduYXR1cmUiOnsiY29udGVudCI6Ik1FVUNJRXd1akJXTStrQlprTlBsVlo0dGNsb3NtUVVOQ1NOcmhCckdPbmY4bFp2K0FpRUF2L1ZSYUJHazF0TjZqTXZsN005WGJ4d3lEaTg2dEQrTmMrdHZySTRHYU9VPSIsInB1YmxpY0tleSI6eyJjb250ZW50IjoiTFMwdExTMUNSVWRKVGlCUVZVSk1TVU1nUzBWWkxTMHRMUzBLVFVacmQwVjNXVWhMYjFwSmVtb3dRMEZSV1VsTGIxcEplbW93UkVGUlkwUlJaMEZGTDFsS1ZHczBNMlJHT1VKWldVbEtWMUJLV0RsU1l5dENTR2hRTmdwSFJWSmFORlJxYTJ0Q09Xd3ZkbkJJVlRaU1JUSm5VMVF4Y25wQmNFVXlOM3BDV0VWWFRXVnlaelJHTkhkc1RYQTRXak54Ylhkc2REbG5QVDBLTFMwdExTMUZUa1FnVUZWQ1RFbERJRXRGV1MwdExTMHRDZz09In19fX0=",
      "integratedTime": 1671439882,
      "logIndex": 9394536,
      "logID": "c0d23d6ad406973f9559f3ba2d1ca01f84147d8ffc5b8445c224f98b9591801d"
    }
  }
}
```
`base64Signature` is the same as the signature in the rekor.Payload.body. This
is something that I missed initially that the bundle64Signature field and the
signature in the payload content are the same:
```console
$ cat artifact.bundle | jq '.base64Signature'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq '.spec.signature.content'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
```
We can check the cert field using:
```console
$ cat artifact.bundle | jq  -r '.cert' | base64 -d -
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE/YJTk43dF9BYYIJWPJX9Rc+BHhP6
GERZ4TjkkB9l/vpHU6RE2gST1rzApE27zBXEWMerg4F4wlMp8Z3qmwlt9g==
-----END PUBLIC KEY-----
```
We can inspect the public key using:
```
$ cat artifact.bundle | jq  -r '.cert' | base64 -d - | openssl ec -pubin -text
read EC key
Public-Key: (256 bit)
pub:
    04:fd:82:53:93:8d:dd:17:d0:58:60:82:56:3c:95:
    fd:45:cf:81:1e:13:fa:18:44:59:e1:38:e4:90:1f:
    65:fe:fa:47:53:a4:44:da:04:93:d6:bc:c0:a4:4d:
    bb:cc:15:c4:58:c7:ab:83:81:78:c2:53:29:f1:9d:
    ea:9b:09:6d:f6
ASN1 OID: prime256v1
NIST CURVE: P-256
writing EC key
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE/YJTk43dF9BYYIJWPJX9Rc+BHhP6
GERZ4TjkkB9l/vpHU6RE2gST1rzApE27zBXEWMerg4F4wlMp8Z3qmwlt9g==
-----END PUBLIC KEY-----
```
After that we have the `rekorBundle` field which is an object that contains
the `SignedEntryTimeStamp`, the and `Payload`.

And if we inspect the artifact.bundle and look at the body of the payload we
see:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq
{
  "apiVersion": "0.0.1",
  "kind": "hashedrekord",
  "spec": {
    "data": {
      "hash": {
        "algorithm": "sha256",
        "value": "4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1"
      }
    },
    "signature": {
      "content": "MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU=",
      "publicKey": {
        "content": "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFL1lKVGs0M2RGOUJZWUlKV1BKWDlSYytCSGhQNgpHRVJaNFRqa2tCOWwvdnBIVTZSRTJnU1QxcnpBcEUyN3pCWEVXTWVyZzRGNHdsTXA4WjNxbXdsdDlnPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="
      }
    }
  }
}
```
The value of `spec.data.hash.value` is the sha256 hash of artifact.txt:
```console
$ sha256sum artifact.txt 
4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1  artifact.txt
```
And `spec.signature.content` is the signature which is the same as the signature
that cosign sign-blob ouputs (see above):
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq '.spec.signature.content'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
```
This signature was created using the private key, and the public key is in the
field `spec.signature.public.content`:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq -r '.spec.signature.publicKey.content'| base64 -d -
-----BEGIN CERTIFICATE-----
MIICqDCCAi+gAwIBAgIUTPWTfO/1NRaSFdecaAQ/pBDGJp8wCgYIKoZIzj0EAwMw
NzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl
cm1lZGlhdGUwHhcNMjIxMTI1MDczNzEyWhcNMjIxMTI1MDc0NzEyWjAAMFkwEwYH
KoZIzj0CAQYIKoZIzj0DAQcDQgAEJQQ4W/5XP9m4YbWRBQtHGWwn9uUhae38UpcJ
pEM3DOs4zW4MIrMfW4WQD0fwp8PUURDXvQ394poqgGEmSkruLqOCAU4wggFKMA4G
A1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUo3Kn
jJQZ0XfigbD5b0OVNN0xqSowHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y
ZD8wJwYDVR0RAQH/BB0wG4EZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTAsBgor
BgEEAYO/MAEBBB5odHRwczovL2dpdGh1Yi5jb20vbG9naW4vb2F1dGgwgYsGCisG
AQQB1nkCBAIEfQR7AHkAdwDdPTBqxscRMmMZHhyZZzcCokpeuN48rf+HinKALynu
jgAAAYStuBHyAAAEAwBIMEYCIQDM5YSQ/GL6KI5R9Odcn/pSk+qVD6bsL83+Ep9R
2hWTawIhAK1ji1lZ56DsfuLfX7bBC9nbR3ElxalBhv1zQXMU7tlwMAoGCCqGSM49
BAMDA2cAMGQCMBK8tsgHeguh+Yhel3PijHQlyJ1Q5K64p0xqDdo7W4fxfoAS9xrP
s2PKQcdoD9bXwgIwX6zLjybZkNHP5xtBp7vK2FYeZt0OWLRlUllcUDL3T/7JQfwc
GSq6vVBNwJ00w9HR
-----END CERTIFICATE-----
```
And we can inspect the cert using openssl:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq -r '.spec.signature.publicKey.content'| base64 -d - | openssl x509 -text
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number:
            4c:f5:93:7c:ef:f5:35:16:92:15:d7:9c:68:04:3f:a4:10:c6:26:9f
        Signature Algorithm: ecdsa-with-SHA384
        Issuer: O = sigstore.dev, CN = sigstore-intermediate
        Validity
            Not Before: Nov 25 07:37:12 2022 GMT
            Not After : Nov 25 07:47:12 2022 GMT
        Subject: 
        Subject Public Key Info:
            Public Key Algorithm: id-ecPublicKey
                Public-Key: (256 bit)
                pub:
                    04:25:04:38:5b:fe:57:3f:d9:b8:61:b5:91:05:0b:
                    47:19:6c:27:f6:e5:21:69:ed:fc:52:97:09:a4:43:
                    37:0c:eb:38:cd:6e:0c:22:b3:1f:5b:85:90:0f:47:
                    f0:a7:c3:d4:51:10:d7:bd:0d:fd:e2:9a:2a:80:61:
                    26:4a:4a:ee:2e
                ASN1 OID: prime256v1
                NIST CURVE: P-256
        X509v3 extensions:
            X509v3 Key Usage: critical
                Digital Signature
            X509v3 Extended Key Usage: 
                Code Signing
            X509v3 Subject Key Identifier: 
                A3:72:A7:8C:94:19:D1:77:E2:81:B0:F9:6F:43:95:34:DD:31:A9:2A
            X509v3 Authority Key Identifier: 
                keyid:DF:D3:E9:CF:56:24:11:96:F9:A8:D8:E9:28:55:A2:C6:2E:18:64:3F

            X509v3 Subject Alternative Name: critical
                email:daniel.bevenius@gmail.com
            1.3.6.1.4.1.57264.1.1: 
                https://github.com/login/oauth
            CT Precertificate SCTs: 
                Signed Certificate Timestamp:
                    Version   : v1 (0x0)
                    Log ID    : DD:3D:30:6A:C6:C7:11:32:63:19:1E:1C:99:67:37:02:
                                A2:4A:5E:B8:DE:3C:AD:FF:87:8A:72:80:2F:29:EE:8E
                    Timestamp : Nov 25 07:37:12.434 2022 GMT
                    Extensions: none
                    Signature : ecdsa-with-SHA256
                                30:46:02:21:00:CC:E5:84:90:FC:62:FA:28:8E:51:F4:
                                E7:5C:9F:FA:52:93:EA:95:0F:A6:EC:2F:CD:FE:12:9F:
                                51:DA:15:93:6B:02:21:00:AD:63:8B:59:59:E7:A0:EC:
                                7E:E2:DF:5F:B6:C1:0B:D9:DB:47:71:25:C5:A9:41:86:
                                FD:73:41:73:14:EE:D9:70
    Signature Algorithm: ecdsa-with-SHA384
         30:64:02:30:12:bc:b6:c8:07:7a:0b:a1:f9:88:5e:97:73:e2:
         8c:74:25:c8:9d:50:e4:ae:b8:a7:4c:6a:0d:da:3b:5b:87:f1:
         7e:80:12:f7:1a:cf:b3:63:ca:41:c7:68:0f:d6:d7:c2:02:30:
         5f:ac:cb:8f:26:d9:90:d1:cf:e7:1b:41:a7:bb:ca:d8:56:1e:
         66:dd:0e:58:b4:65:52:59:5c:50:32:f7:4f:fe:c9:41:fc:1c:
         19:2a:ba:bd:50:4d:c0:9d:34:c3:d1:d1
-----BEGIN CERTIFICATE-----
MIICqDCCAi+gAwIBAgIUTPWTfO/1NRaSFdecaAQ/pBDGJp8wCgYIKoZIzj0EAwMw
NzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl
cm1lZGlhdGUwHhcNMjIxMTI1MDczNzEyWhcNMjIxMTI1MDc0NzEyWjAAMFkwEwYH
KoZIzj0CAQYIKoZIzj0DAQcDQgAEJQQ4W/5XP9m4YbWRBQtHGWwn9uUhae38UpcJ
pEM3DOs4zW4MIrMfW4WQD0fwp8PUURDXvQ394poqgGEmSkruLqOCAU4wggFKMA4G
A1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUo3Kn
jJQZ0XfigbD5b0OVNN0xqSowHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y
ZD8wJwYDVR0RAQH/BB0wG4EZZGFuaWVsLmJldmVuaXVzQGdtYWlsLmNvbTAsBgor
BgEEAYO/MAEBBB5odHRwczovL2dpdGh1Yi5jb20vbG9naW4vb2F1dGgwgYsGCisG
AQQB1nkCBAIEfQR7AHkAdwDdPTBqxscRMmMZHhyZZzcCokpeuN48rf+HinKALynu
jgAAAYStuBHyAAAEAwBIMEYCIQDM5YSQ/GL6KI5R9Odcn/pSk+qVD6bsL83+Ep9R
2hWTawIhAK1ji1lZ56DsfuLfX7bBC9nbR3ElxalBhv1zQXMU7tlwMAoGCCqGSM49
BAMDA2cAMGQCMBK8tsgHeguh+Yhel3PijHQlyJ1Q5K64p0xqDdo7W4fxfoAS9xrP
s2PKQcdoD9bXwgIwX6zLjybZkNHP5xtBp7vK2FYeZt0OWLRlUllcUDL3T/7JQfwc
GSq6vVBNwJ00w9HR
-----END CERTIFICATE-----
```
The process of verifying needs to verify the signature of the bundle using
the base64Signature and cert fields. And it also need to verify the signature of
the Payload using the certificate in the payload.

So to sign a blob offline do we really have to go through the process of
creating the signature layers?

If we take a look at how cosign [verify-blob](https://github.com/sigstore/cosign/blob/main/cmd/cosign/cli/verify/verify_blob.go) works.
The command looks like this:
```console
$ cosign verify-blob --bundle=artifact.bundle artifact.txt
```
So we are passing in the bundle and the blob (which is just a text file in
this case).

`VerifyBlobCmd` first extracts the `base64Signature` from the bundle:
```go
func (c *VerifyBlobCmd) Exec(ctx context.Context, blobRef string) error {
  ...
  sig, err := base64signature(c.SigRef, c.BundlePath)
```
The `base64signature` function has a switch/case statement and in this case we
are only interested in the bundlePath:
```go
func base64signature(sigRef, bundlePath string) (string, error) {
  ...
  case bundlePath != "":
    b, err := cosign.FetchLocalSignedPayloadFromPath(bundlePath)
    if err != nil {
      return "", err
    }
    targetSig = []byte(b.Base64Signature)
}
`LocalSignedPayload` is what FetchLocalSignedPayloadFromPath which looks like
this:
```go
type LocalSignedPayload struct {
	Base64Signature string              `json:"base64Signature"`
	Cert            string              `json:"cert,omitempty"`
	Bundle          *bundle.RekorBundle `json:"rekorBundle,omitempty"`
}
```
So this is extracting the base64Signature field:
```console
$ cat artifact.bundle | jq '.base64Signature'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
```
And recall that this is the signature of the bundle and not the signature of the
the bundle payload, the actual blob. I initially got this mixed up and is the
reason for stating this several times).

`LocalSignedPayload` is the equivalent of SignedArtifactBundle in sigstore-rs.
And notice the field `base64Signature` is what is being returned here. So, `sig`
is that field when we return to `VerifyBlobCmd`:
```go
  blobBytes, err := payloadBytes(blobRef)
```
Here the bytes of the blob are read, and this is a function I believe because
the input can come from a file or from stdin.

A little further down in `VerifyBlobCmd` we then have:
```go
if c.BundlePath != "" {
  b, err := cosign.FetchLocalSignedPayloadFromPath(c.BundlePath)
  ...
  // We have to condition on this because sign-blob may not output the signing
  // key to the bundle when there is no tlog upload.
  if b.Cert != "" {
    // b.Cert can either be a certificate or public key
    certBytes := []byte(b.Cert)
    if isb64(certBytes) {
      certBytes, _ = base64.StdEncoding.DecodeString(b.Cert)
    }
    cert, err = loadCertFromPEM(certBytes)
    if err != nil {
      // check if cert is actually a public key
      co.SigVerifier, err = sigs.LoadPublicKeyRaw(certBytes, crypto.SHA256)
      if err != nil {
        return fmt.Errorf("loading verifier from bundle: %w", err)
      }
    }
  }
  opts = append(opts, static.WithBundle(b.Bundle))
}
```
This is parsing the `cert` fields which can be a public key or a certificate
with a public key, and can be used for verifying the bundle's signature
(base64Signature).

Notice that this is again marshalling a LocalSignedPayload: TODO: could this
perhaps be done only once in the Go library code?

And later in VerifyBlobCmd we have:
```go
  ...
  if isIntotoDSSE(blobBytes) {
    ...
  } else {
    signature, err := static.NewSignature(blobBytes, sig, opts...)
    if err != nil {
      return err
    }
    if _, err = cosign.VerifyBlobSignature(ctx, signature, co); err != nil {
      return err
    }
  }
  fmt.Fprintln(os.Stderr, "Verified OK")
```
`NewSignature` takes the `blobBytes`, and then `sig` which is the
`base64signature` field from the bundle as we saw earlier:
```go
// NewSignature constructs a new oci.Signature from the provided options.
func NewSignature(payload []byte, b64sig string, opts ...Option) (oci.Signature, error) {
  o, err := makeOptions(opts...)
  ...
  return &staticLayer{
    b:      payload,
    b64sig: b64sig,
    opts:   o,
  }, nil
}
```
So this returns a staticLayer which implements oci.Signature functions.
```go
type staticLayer struct {
	b      []byte
	b64sig string
	opts   *options
}
```
So once again, the payload is the content of `artifact.txt`, and `b64sig` is
the `base64Signature` field from `artifact.bundle`.

This `oci.Signature` is then be passed to `VerifyBlobSignature` and the options
(which contains the bundle so that information is also available):
```go
// VerifyBlobSignature verifies a blob signature.
func VerifyBlobSignature(ctx context.Context, sig oci.Signature, co *CheckOpts)
    (bundleVerified bool, err error) {
  // The hash of the artifact is unused.
  return verifyInternal(ctx, sig, v1.Hash{}, verifyOCISignature, co)
}
```
`verifyOCISignature` is a function pointer which looks like this:
```go
func verifyOCISignature(ctx context.Context, verifier signature.Verifier, sig payloader) error {
  b64sig, err := sig.Base64Signature()
  if err != nil {
    return err
  }
  signature, err := base64.StdEncoding.DecodeString(b64sig)
  if err != nil {
    return err
  }
  payload, err := sig.Payload()
  if err != nil {
    return err
  }
    return verifier.VerifySignature(bytes.NewReader(signature),
                                    bytes.NewReader(payload),
                                    options.WithContext(ctx))
}
```
But that function is not executed yet, only the pointer to the function is
passed to `verifyInternal`:
```go
func verifyInternal(ctx context.Context, sig oci.Signature, h v1.Hash,
	verifyFn signatureVerificationFn, co *CheckOpts) (
	bundleVerified bool, err error) {


  bundleVerified, err = VerifyBundle(sig, co)
}
```
`VerifyBundle` will do the following:
```go
func VerifyBundle(sig oci.Signature, co *CheckOpts) (bool, error) {
  bundle, err := sig.Bundle()

  if err := compareSigs(bundle.Payload.Body.(string), sig); err != nil {
    return false, err
  }

}
```

```go
func compareSigs(bundleBody string, sig oci.Signature) error {
  actualSig, err := sig.Base64Signature()
}
```
So `actualSig` will be the value of `base64Signature`.
```go
bundleSignature, err := bundleSig(bundleBody)
```
This is extracting the signature from the bundle payload body.
Keep in mind that the bundleBody looks like this:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq
{
  "apiVersion": "0.0.1",
  "kind": "hashedrekord",
  "spec": {
    "data": {
      "hash": {
        "algorithm": "sha256",
        "value": "4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1"
      }
    },
    "signature": {
      "content": "MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU=",
      "publicKey": {
        "content": "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFL1lKVGs0M2RGOUJZWUlKV1BKWDlSYytCSGhQNgpHRVJaNFRqa2tCOWwvdnBIVTZSRTJnU1QxcnpBcEUyN3pCWEVXTWVyZzRGNHdsTXA4WjNxbXdsdDlnPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="
      }
    }
  }
}
```
And this will extract the signature:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq '.spec.signature.content'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
```
```go
if bundleSignature != actualSig {
   return &VerificationError{"signature in bundle does not match signature being verified"}
}
```
This is checking that the base64Signature is the same as the signature in the
Payload body. This is something that I missed initially that the
bundle64Sigature field and the signature in the payload content are the same:
```console
$ cat artifact.bundle | jq '.base64Signature'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d - | jq '.spec.signature.content'
"MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU="
```
When the blob is signed and the signature is [attached](https://github.com/sigstore/cosign/blob/3bbdf16c7ec69562edb94a4b3114f441179c8c21/cmd/cosign/cli/sign/sign_blob.go#L124) to the
LocalSignedPayload which is the serialized and written to disk:
```go
// if bundle is specified, just do that and ignore the rest
  if ko.BundlePath != "" {
  signedPayload.Base64Signature = base64.StdEncoding.EncodeToString(sig)
  signedPayload.Cert = base64.StdEncoding.EncodeToString(rekorBytes)

  contents, err := json.Marshal(signedPayload)
  if err != nil {
    return nil, err
  }
  if err := os.WriteFile(ko.BundlePath, contents, 0600); err != nil {
    return nil, fmt.Errorf("create bundle file: %w", err)
  }
  fmt.Printf("Bundle wrote in the file %s\n", ko.BundlePath)
}
```
`VeribyBundle` also checks the public keys:
```go
if err := comparePublicKey(bundle.Payload.Body.(string), sig, co); err != nil {
  return false, err
}
```
This is going to compare the `cert` field with the
`rekorBundle.Payload.body.spec.signature.publicKey.content` field, similar to
the below:
```console
$ cat artifact.bundle | jq -r '.cert' | base64 -d 
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE/YJTk43dF9BYYIJWPJX9Rc+BHhP6
GERZ4TjkkB9l/vpHU6RE2gST1rzApE27zBXEWMerg4F4wlMp8Z3qmwlt9g==
-----END PUBLIC KEY-----
```
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d | jq -r '.spec.signature.publicKey.content' | base64 -d 
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE/YJTk43dF9BYYIJWPJX9Rc+BHhP6
GERZ4TjkkB9l/vpHU6RE2gST1rzApE27zBXEWMerg4F4wlMp8Z3qmwlt9g==
-----END PUBLIC KEY-----
```

The payload if verified as well:
```go
  payload, err := sig.Payload()
  signature, err := sig.Base64Signature()
  alg, bundlehash, err := bundleHash(bundle.Payload.Body.(string), signature)
  h := sha256.Sum256(payload)
  payloadHash := hex.EncodeToString(h[:])
```
Notice that `bundleHash` returns two values (and an error but I'm ignoring that)
, `alg`, and `bundleHash`.
`alg` will be the contents of:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d | jq -r '.spec.data.hash.algorithm'
sha256
```
And `bundleHash` will be:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d | jq -r '.spec.data.hash.value'
4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1
```
`h` is the `artifact.txt` bytes passed through sha256 sum:
```console
$ sha256sum artifact.txt 
4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1  artifact.txt
```
And this is then encoded into a string from hex:
```console
$ cat artifact.bundle | jq -r '.rekorBundle.Payload.body' | base64 -d | jq -r '.spec.data.hash.value' | xxd -p -c 128
346263343533623533636233643931346234356634623235303239343233366164626132633065303966663666303337393339343965376533396664346363310a
```
The `bundleHash` is then compared with the `payloadHash`:
```go
  if alg != "sha256" || bundlehash != payloadHash {
    return false, fmt.Errorf("matching bundle to payload: %w", err)
  }
  return true, nil
```
I'm not exactly sure where the bundleHash is encoded from hex to string, but it
looks like that must be happening. Perhaps this is part of the marshalling into
a Record.

So we know that we need sha256sum of the blob.

_wip_

### Running the example
Then we can run the example using:
```console
$ cd examples/verify-bundle
$ cargo run --example verify-bundle -- \
    --rekor-pub-key ~/.sigstore/root/targets/rekor.pub \
    --bundle artifact.bundle \
    artifact.txt
```
