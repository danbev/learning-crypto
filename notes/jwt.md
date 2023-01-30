## JSON Web Token (JWT)
Is a standard to share information, called a claim,  between two entities and
the json object is signed so that tampering can be detected.

Can be either a [jws](./jsw.md) or a [jwe](./jwe.md).

### Structure
The JWT contains 3 sections, a header, a payload, and a signature. The 
header and the payload are base64 encoded. 
```
base64(header).base64(payload).signature
```


### Idenity Token
Contains infomation of an authenitcated user.

### Example 
Header:
```json
{
  "alg": "RS256",
  "kid": "14026c9c44b1a0514c87ad8033e35bfdbf20f01e"
}
```
Note that the algorithm is the algorithm that should be used to verify the
payload (using the signature which is a hash/digest over the payload. In this
case RSA 256-bit is being used.

Payload:
```json
{
  "iss": "https://oauth2.sigstore.dev/auth",
  "sub": "CgY0MzIzNTESJmh0dHBzOiUyRiUyRmdpdGh1Yi5jb20lMkZsb2dpbiUyRm9hdXRo",
  "aud": "sigstore",
  "exp": 1668518946,
  "iat": 1668518886,
  "nonce": "2HaMkKSCuNJlYxG0KUNQpPoHvAT",
  "at_hash": "TW8MOeSJrk-4dmtwGN47rQ",
  "c_hash": "kB83-IR05h25H1jv4l74sA",
  "email": "daniel.bevenius@gmail.com",
  "email_verified": true,
  "federated_claims": {
    "connector_id": "https://github.com/login/oauth",
    "user_id": "432351"
  }
}
```

The signature is a hashed combo of the header and the payload:
```console
xYg1z8AwzIA8AJ0V4pjhQzYkNL90_Yc41edd57364C1-8DVIYvhW9t8JAGrIwzAY55rpGOANEnR1sdmyZACo9fcFbS9UUWXPvr3AL3Ck061RpryDO1PdsEds6UIV7A2XfTsq0YtBWDd8maFw5Q6Pfn27k1a3XZIBgOqxTleDb84WdPxVgTu_TGqJ2BB5R-R0eKxaeKBsHOX9ngv_BvMx9wES7KCspB_cPnFWf6kw4glsLxtjDSrBq6Rp6LfN54RLIaP2xp1kbkmcayMq3N-3vnoN32F92_3z5A4VOQOIrvudCAbz04De5SqyCvXYZZTYdd1hPFNVE46pEC4Sl-2Mww
```
So how do we verify this, we need to have a public key to verify the signature
that was generated. We can get this from the `issuer` field using  the following
command:
```console
$ openssl s_client -connect oauth2.sigstore.dev:443 -showcerts </dev/null | openssl x509 -outform pem > sigstore.pem
```
And the we can extract the public key from `sigstore.pem` using:
```console
$ openssl x509 -pubkey -noout -in sigstore.pem  > pubkey.pem
```
```console
$ cat pubkey.pem 
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyUjcXSd38Jd3mlSvMDYP
ImXNOgYPQSvSAcaSqsKLYLc0czBmQzvSjkOaW3eeYWFXno7xQLxhh6kiulCR/6kr
r0Y/KfiaXxZQvv4pM47U8KGeWiBRdWTkH1ZP89z7OVMxebTa9+Ryqp8ObZRaRER2
a/yopAIWRRLvBQ8UT7Cyp6SK6YpiOEkaW2vEZQuKqerAV/i+QMuVpWlXY4S3ou48
y60Tu/QTU7lchJF8i9W1C1vadM0dMMWUtI6tod5LkOT69MDT5+QLnDGQgdYdBy+j
aLXLbg2nrM9MfdtQZW8tZhfxbCEs8i0zdx2dO6u62i7k6YDAOcBAeKeSiprEbugx
gQIDAQAB
-----END PUBLIC KEY----
```

Alright, so I now discovered that most auth servers expose a discovery endpoint
which we can use:
```console
$ curl https://oauth2.sigstore.dev/auth/.well-known/openid-configuration
{
  "issuer": "https://oauth2.sigstore.dev/auth",
  "authorization_endpoint": "https://oauth2.sigstore.dev/auth/auth",
  "token_endpoint": "https://oauth2.sigstore.dev/auth/token",
  "jwks_uri": "https://oauth2.sigstore.dev/auth/keys",
  "userinfo_endpoint": "https://oauth2.sigstore.dev/auth/userinfo",
  "device_authorization_endpoint": "https://oauth2.sigstore.dev/auth/device/code",
  "grant_types_supported": [
    "authorization_code",
    "refresh_token",
    "urn:ietf:params:oauth:grant-type:device_code"
  ],
  "response_types_supported": [
    "code"
  ],
  "subject_types_supported": [
    "public"
  ],
  "id_token_signing_alg_values_supported": [
    "RS256"
  ],
  "code_challenge_methods_supported": [
    "S256",
    "plain"
  ],
  "scopes_supported": [
    "openid",
    "email",
    "groups",
    "profile",
    "offline_access"
  ],
  "token_endpoint_auth_methods_supported": [
    "client_secret_basic",
    "client_secret_post"
  ],
  "claims_supported": [
    "iss",
    "sub",
    "aud",
    "iat",
    "exp",
    "email",
    "email_verified",
    "locale",
    "name",
    "preferred_username",
    "at_hash"
  ]
}
```
Notice the `jwtks_uri`, and if we access that we find: 
```console
$ curl -s https://oauth2.sigstore.dev/auth/keys | jq
{
  "keys": [
    {
      "use": "sig",
      "kty": "RSA",
      "kid": "1456029e0714f659284b99cc6b27bdd61a455d7f",
      "alg": "RS256",
      "n": "9fZODOmWJINdAetTxzZkLnk3Vr9_jb6--NX284FQtxIxb-tbHR8oEW94Wl-moe76pGfk0SFJac2uq6_C-805kkaqgIQqgES_CCD0-8BXikdY1-5Q-dj6yQU98GUhrRDxx-WsdGnyrbsSrUre32_WQkaEQEP2IcEo63b3Y4F4WZ8arBFua_rYz0uwR3oX7HuOTIhGi5R6oy_FSsx2NYxlqnJxSWc7vv9GiQ3WabRtAN2OiETSIM-SNpRCjf7WngfBq2gcSvPJ8mp-epARPQUe0FCevZT_dq4YU29okOn9mRwo_s7eiy-9JIuTWbTYmUyobERXFaEMWSzpyzC_f6ts_w",
      "e": "AQAB"
    },
    {
      "use": "sig",
      "kty": "RSA",
      "kid": "14026c9c44b1a0514c87ad8033e35bfdbf20f01e",
      "alg": "RS256",
      "n": "2irsovCma5xSo4VLrjw_nI3zj2ardi6Rq5nD8PaD-rOvnS9WFgSP0ULHnfOTdAiS3EyvjQghjQ5-P43KkhRZKntpDR5l_tNjtz8MCCW3MYsc1HYerJGSXrOXR3X8TGccQ6Qe4K2_NYNRJYVr5V8lPAZ6TMe1pIDyKv5wIcWDC4jSzD8va7XL3MFRZmcs8aDqmbmcNw2_FYn6_0sfwIwQ6SrUjjD7H-pdoj1HONg6hNcQdn0RocYtxxlQKOyBrVMWsRAupGZxPVRKCS94RcGsGaONrQYQO_gB-LKPFEP35-jZm92OHvyXE4UxPGYXbBAbjVSrMjdKLaWEDNAnIaBRQw",
      "e": "AQAB"
    }
  ]
}
```
And now we can understand where the `kid` is coming from:
```json
  "kid": "14026c9c44b1a0514c87ad8033e35bfdbf20f01e"
```
So lets try verifying a signature. Now I'm guessing that the signature was
created like this:
```
signature = hash(private_key, header.json + payload.json)
```
So to verify we would need to do:
```console
$ jq -c '' < jwt-header.json > jwt-header-compact.json
```
And the 
```console
$ openssl dgst -sha256 -verify pubkey.pem -signature jwt-signature.txt jwt-header-compact.json
```
