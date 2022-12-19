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
So the verifies something that is a SignatureLayer, lets take a closer look at
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
The `Docker-manifest-digest` is a hash for the manifest file of the container
image that we want to sign.

This json is then canonicalized, and we use our private key sign this payload
(the SimpleSigning "json" that is) and that signature is then base64 encoded
so that it can be stored. So wher is it stored?  It is stored in a new
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
mentioned previously, and also the `cosign_signature_image` is the image that
holds the signature created by cosign. For more details about how this works see
[Container Image Signing](https://github.com/danbev/learning-crypto/blob/main/notes/sigstore.md#continer-image-signing).

So back to SignatureLayer, after the simple_signing field we have an oci_digest
then an optional cerfificate_signature, an optional bundle, and optional
signature, and finally raw_data.

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
So the verification constraint will be passed a SignatureLayer which which it
can use to verify. 

### Offline blob verification
If we take a closer look at the bundle payload. First, when we use cosign's
sign-blob which will ouput the signature of the blob:
```console
Bundle wrote in the file artifact.bundle
MEUCIEwujBWM+kBZkNPlVZ4tclosmQUNCSNrhBrGOnf8lZv+AiEAv/VRaBGk1tN6jMvl7M9XbxwyDi86tD+Nc+tvrI4GaOU=
```
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
So to sign a blob offline do we really have to go through the process of
creating the signature layers?

If we take a look at how cosign [verify-blob](https://github.com/sigstore/cosign/blob/main/cmd/cosign/cli/verify/verify_blob.go) works.
It first extracts the base64 signature from the bundle:

```go
func (c *VerifyBlobCmd) Exec(ctx context.Context, blobRef string) error {
  ...
  sig, err := base64signature(c.SigRef, c.BundlePath)
```
The `base64signature` function has a switch/case statement but in this case we
are interested in the bundlePath:
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
This is the equivalent of SignedArtifactBundle in sigstore-rs.
And notice the field `base64Signature` is what is being returned here. So, `sig`
is that field when we return to `VerifyBlobCmd`:
```go
  blobBytes, err := payloadBytes(blobRef)
```
Here the bytes of the blob are read, and this is a function I believe because
input can come from a file or stdin.
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
Notice that this is again marshalling a LocalSignedPayload: TODO: could this
perhaps be done only once in the Go library code?


```go
  ...
  if isIntotoDSSE(blobBytes) {
    // co.SigVerifier = dsse.WrapVerifier(co.SigVerifier)
    signature, err := static.NewAttestation(blobBytes, opts...)
    if err != nil {
      return err
    }
    if _, err = cosign.VerifyBlobAttestation(ctx, signature, co); err != nil {
      return err
    }
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
So what does `NewSignature` do then. I see it takes the `blobBytest`, and then
`sig` which is the base64signature field as we saw earlier:
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
So this `oci.Signature` then be passed to `VerifyBlobSignature` and the options
contains the bundle so that information is also available to
`VerifyBlobSignature`.

It then creates a new Signer using the blob (bytes), the signature (which is in
the bundle), and options which contains the LocalSignedPayload. 
Next cosign.VerifyBlobSignature is called.

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
