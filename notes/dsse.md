## Dead Simple Signing Envelope
Is a standard for signing any type of data.

[github repo](https://github.com/secure-systems-lab/dsse)

This specifies a format for storing data signatures which uses a construct
called an Envelope:
```json
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [{
    "keyid": "<KEYID>",
    "sig": "<Base64(SIGNATURE)>"
  }]
}
```
keyid is optional and is something the must be agreed upon out-of-band between
the signer and the verifier. So the signer would calculate the keyid and set
it.

Notice that we have a single payload field and a single payloadType, but
`signatures` is an array. The documentations says the following about this:
```
"An envelope MAY have more than one signature, which is equivalent to separate
envelopes with individual signatures."
```
So the following:
```
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [
    { "keyid": "keyid_1", "sig": "<Base64(SIGNATURE)>" },
    { "keyid": "keyid_2", "sig": "<Base64(SIGNATURE)>" }
  ]
}
```
Would be the same as:
```
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [{
    "keyid": "keyid_1",
    "sig": "<Base64(SIGNATURE)>"
  }]
}
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [{
    "keyid": "keyid_2",
    "sig": "<Base64(SIGNATURE)>"
  }]
}
```

A signature is defined as:
```
 signature = Sign(PreAuthenicatedEncoding(UTF8(payloadType), SERIALIZED_BODY); 
```

### PreAuthenicatedEncoding
This is what is actually signed and is defined in [protocol]. The following is
what we used in Seedwing Policy Engine:
```rust
/// Pre-Authenticated Encoding (PAE) for DSSEv1                                 
fn pae<'a>(payload_type: &'a str, payload: &str) -> String {                    
    let pae = format!(                                                          
        "DSSEv1 {} {} {} {}",                                                   
        payload_type.len(),                                                     
        payload_type,                                                           
        payload.len(),                                                          
        payload,                                                                
    );                                                                          
    pae                                                                         
}

```

Notice that this format almost exactly like the format
suggested for in-toto attestations which is because it uses DSSE.

### keyid
This field is part the elements/objects in the signature array, for example:
```json
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [{
    "keyid": "<KEYID>",
    "sig": "<Base64(SIGNATURE)>"
  }]
}
```
`keyid` is optional and is something the must be agreed upon out-of-band between
the signer and the verifier. As the name implies it is for identifying which
public key to use to verify the envelope. This is what the spec says:
```
"Optional, unauthenticated hint indicating what key and algorithm was used to
sign the message. As with Sign(), details are agreed upon out-of-band by the
signer and verifier. It MUST NOT be used for security decisions; it may only be
used to narrow the selection of possible keys to try."
```

I'm not sure if this is similar/same as the `kid` field which is in a jwt
header:
```json
{
  "alg": "RS256",
  "kid": "c17ff6fe9126450eaadb9a0d8fc7917dab208a75"
}
```
But in this case the client/user find the openid-configuration, and use the
value of `jwks_uri` to find the key:
```console
$ cd jwt

$ curl -s  https://oauth2.sigstore.dev/auth/.well-known/openid-configuration | jq '.jwks_uri'
"https://oauth2.sigstore.dev/auth/keys"

$ curl -s https://oauth2.sigstore.dev/auth/keys | jq
{
  "keys": [
    {
      "use": "sig",
      "kty": "RSA",
      "kid": "2b526ae69ffd2db0652e20f0a38b6f5222192aab",
      "alg": "RS256",
      "n": "qzItsMn0DaRakhVncA6GF_tuU9Dt2pk9lQITaLFRWQ3xrNF6WfW4PhrqUxb2tQPIajDSh8aiHOkXWCiUlHDNblYSPBpZfNFDe2-TgMF8nu75d7_Py1WYmpUQh0aNIfo4397qwWev8R7VDmdI_Zms3bRrSaZxjH2qXRHhEYjD637h4Lg9HYCe1aIwB_e_69_UP4uIx0A1yULmM8FlhSRqlPXM9iGBSunHl7287MTBL4eQ8VpPR96fTlOsxIb9dTECk919TZQP7cTwKhliPq4yajCqWFp2FXUNufu6zs7LGy9eL0OQZJb8yd5M9vtAy5PrGnOn4xrnUgjKQBenPe1lTw",
      "e": "AQAB"
    },
    {
      "use": "sig",
      "kty": "RSA",
      "kid": "c17ff6fe9126450eaadb9a0d8fc7917dab208a75",
      "alg": "RS256",
      "n": "qz8PaQTJxH17pEzjBA8Gi-YQ4s-aBg6rv8GU79XcB1eFHVWBwfxp0hn4Bp7bnlIIO_kaP2rQQJrjNWUTfAb0GU_y9XTFXR24fD8-Zs69nHhwId13Le_cqAdNI1Lqk8p9LwTyR78vXU2qLTkEJOat4pnuE6JdX2nQwbADc9JtQM8BcmZbdQ5of-Qk6j5UAF-s39B4mLSLFuVkWW2vhuJsu76oiT8ad9sJp25CDV8EcBcEfIK2ToZG-1UgDV3y60fv-Vak0P6rh_21I6zKcUOxRhSl-cKHMpqsxafLnvr1oWzvNdlSfqtulWVtyCdMb5IeHKxVwycrWVKsAxl_ynPH-w",
      "e": "AQAB"
    }
  ]
}

$ curl -s https://oauth2.sigstore.dev/auth/keys | jq -c '.keys[] | select (.kid | contains("c17ff6fe9126450eaadb9a0d8fc7917dab208a75"))' | node index.mjs 
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqz8PaQTJxH17pEzjBA8G
i+YQ4s+aBg6rv8GU79XcB1eFHVWBwfxp0hn4Bp7bnlIIO/kaP2rQQJrjNWUTfAb0
GU/y9XTFXR24fD8+Zs69nHhwId13Le/cqAdNI1Lqk8p9LwTyR78vXU2qLTkEJOat
4pnuE6JdX2nQwbADc9JtQM8BcmZbdQ5of+Qk6j5UAF+s39B4mLSLFuVkWW2vhuJs
u76oiT8ad9sJp25CDV8EcBcEfIK2ToZG+1UgDV3y60fv+Vak0P6rh/21I6zKcUOx
RhSl+cKHMpqsxafLnvr1oWzvNdlSfqtulWVtyCdMb5IeHKxVwycrWVKsAxl/ynPH
+wIDAQAB
-----END PUBLIC KEY-----
```

### Multiple signatures
Notice that we have a single payload field and a single payloadType, but
`signatures` is an array. The documentations says the following about this:
```
"An envelope MAY have more than one signature, which is equivalent to separate
envelopes with individual signatures."
```
So the following:
```
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [
    { "keyid": "keyid_1", "sig": "<Base64(SIGNATURE)>" },
    { "keyid": "keyid_2", "sig": "<Base64(SIGNATURE)>" }
  ]
}
```
Would be the same as:
```
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [{
    "keyid": "keyid_1",
    "sig": "<Base64(SIGNATURE)>"
  }]
}
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [{
    "keyid": "keyid_2",
    "sig": "<Base64(SIGNATURE)>"
  }]
}
```
This might be the case you require that multiple people sign off an envelope.


A signature is defined as:
```
 signature = Sign(PreAuthenicatedEncoding(UTF8(payloadType), SERIALIZED_BODY); 
```

### PreAuthenicatedEncoding
This is what is actually signed and is defined in [protocol]. The following is
what we used in Seedwing Policy Engine:
```rust
/// Pre-Authenticated Encoding (PAE) for DSSEv1                                 
fn pae<'a>(payload_type: &'a str, payload: &str) -> String {                    
    let pae = format!(                                                          
        "DSSEv1 {} {} {} {}",                                                   
        payload_type.len(),                                                     
        payload_type,                                                           
        payload.len(),                                                          
        payload,                                                                
    );                                                                          
    pae                                                                         
}

```

Notice that this format almost exactly like the format
suggested for in-toto attestations which is because it uses DSSE.

### Certificate field
I've seen a `cert` field in addition to the `keyid` and `sig` fields in some
envelopes which is not included in the DSSE specification. For example
```
{
  "payloadType": "application/vnd.in-toto+json",
  "payload": "...",
  "signatures": [
    {
      "keyid": "",
      "sig": "MEUCIQCqzZFn+1aoTTKyoBUavC/GL9gE6OlyXE82ESBVplp9lAIgBMZn8wFaVIkh90nIyKjqYWnOijvZhIuVOk28M2Qgn0U=",
      "cert": "-----BEGIN CERTIFICATE-----\nMIIDwDCCA0agAwIBAgIULJZj6eAZtsWdIHFrKg+M+LVdNA0wCgYIKoZIzj0EAwMw\nNzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl\ncm1lZGlhdGUwHhcNMjMwMzE0MTAyNTA1WhcNMjMwMzE0MTAzNTA1WjAAMFkwEwYH\nKoZIzj0CAQYIKoZIzj0DAQcDQgAEmIAvXVLTh66E3WdWRFZsVSHOUCk0mL+k4KIv\naN39hGzHgpz3jZvbZw6xShrbuVXUn01APrM/QhtaVa1bmeBKWKOCAmUwggJhMA4G\nA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUdnHr\n9JtQeBQGVxmSBdXqA2xCUyUwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y\nZD8wfQYDVR0RAQH/BHMwcYZvaHR0cHM6Ly9naXRodWIuY29tL3Nsc2EtZnJhbWV3\nb3JrL3Nsc2EtZ2l0aHViLWdlbmVyYXRvci8uZ2l0aHViL3dvcmtmbG93cy9idWls\nZGVyX2dvX3Nsc2EzLnltbEByZWZzL3RhZ3MvdjEuNS4wMDkGCisGAQQBg78wAQEE\nK2h0dHBzOi8vdG9rZW4uYWN0aW9ucy5naXRodWJ1c2VyY29udGVudC5jb20wEgYK\nKwYBBAGDvzABAgQEcHVzaDA2BgorBgEEAYO/MAEDBChiNjAxYzMwYjMxYzRlODMx\nYjFhODQxOGZkMTkzZjA0YzI3NWQyMTBjMBMGCisGAQQBg78wAQQEBUdvIENJMDEG\nCisGAQQBg78wAQUEI3NlZWR3aW5nLWlvL3NlZWR3aW5nLWdvbGFuZy1leGFtcGxl\nMB8GCisGAQQBg78wAQYEEXJlZnMvdGFncy92MC4xLjE1MIGKBgorBgEEAdZ5AgQC\nBHwEegB4AHYA3T0wasbHETJjGR4cmWc3AqJKXrjePK3/h4pygC8p7o4AAAGG36by\nJgAABAMARzBFAiEA9rbuMD3hxqdm4BSY16cgpiE0+ZmfHNOEn8knRjzpwZECIDgh\n6kX4wM9d5IPilvFzn2x++ISKXiOKvfrKn1kKThTwMAoGCCqGSM49BAMDA2gAMGUC\nMEO/jxmuiPiPdfVDDcXAEZ0HTRUp9Wpcsf8vXdu1j84Uwoug53ivlumXoFq7heK1\ntgIxAPCol997A8+NqKUierl9DaEwhApnGZUS5rv1/SqjplJIHhELqT36h64yw9uC\nkP8eDg==\n-----END CERTIFICATE-----\n"
    }
  ]
}
```
If we think about the keyid and its purpose which was to identify a publickey
the certificate is providing the publickey directly (recall that the certificate
binds an entity to a publickey).

So the provides the certificate which can be used to validate this attestation.
And recall from above the signature is over the [PAE](#PreAuthenicatedEncoding),
so basically the payloadType and the payload. And we could also have multiple
signatures which could be where you want multiple parties to sign. So we might
want out build system to sign this but in addition perhaps one of the core
maintainers also needs to sign it, in which case we would have two entries in
the signatures array.


[protocol]: https://github.com/secure-systems-lab/dsse/blob/master/protocol.md

### Certificate field
I've seen a `cert` field in addition to the `keyid` and `sig` fields in some
envelopes which is not included in the DSSE specification. For example
```
{
  "payloadType": "application/vnd.in-toto+json",
  "payload": "...",
  "signatures": [
    {
      "keyid": "",
      "sig": "MEUCIQCqzZFn+1aoTTKyoBUavC/GL9gE6OlyXE82ESBVplp9lAIgBMZn8wFaVIkh90nIyKjqYWnOijvZhIuVOk28M2Qgn0U=",
      "cert": "-----BEGIN CERTIFICATE-----\nMIIDwDCCA0agAwIBAgIULJZj6eAZtsWdIHFrKg+M+LVdNA0wCgYIKoZIzj0EAwMw\nNzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl\ncm1lZGlhdGUwHhcNMjMwMzE0MTAyNTA1WhcNMjMwMzE0MTAzNTA1WjAAMFkwEwYH\nKoZIzj0CAQYIKoZIzj0DAQcDQgAEmIAvXVLTh66E3WdWRFZsVSHOUCk0mL+k4KIv\naN39hGzHgpz3jZvbZw6xShrbuVXUn01APrM/QhtaVa1bmeBKWKOCAmUwggJhMA4G\nA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUdnHr\n9JtQeBQGVxmSBdXqA2xCUyUwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y\nZD8wfQYDVR0RAQH/BHMwcYZvaHR0cHM6Ly9naXRodWIuY29tL3Nsc2EtZnJhbWV3\nb3JrL3Nsc2EtZ2l0aHViLWdlbmVyYXRvci8uZ2l0aHViL3dvcmtmbG93cy9idWls\nZGVyX2dvX3Nsc2EzLnltbEByZWZzL3RhZ3MvdjEuNS4wMDkGCisGAQQBg78wAQEE\nK2h0dHBzOi8vdG9rZW4uYWN0aW9ucy5naXRodWJ1c2VyY29udGVudC5jb20wEgYK\nKwYBBAGDvzABAgQEcHVzaDA2BgorBgEEAYO/MAEDBChiNjAxYzMwYjMxYzRlODMx\nYjFhODQxOGZkMTkzZjA0YzI3NWQyMTBjMBMGCisGAQQBg78wAQQEBUdvIENJMDEG\nCisGAQQBg78wAQUEI3NlZWR3aW5nLWlvL3NlZWR3aW5nLWdvbGFuZy1leGFtcGxl\nMB8GCisGAQQBg78wAQYEEXJlZnMvdGFncy92MC4xLjE1MIGKBgorBgEEAdZ5AgQC\nBHwEegB4AHYA3T0wasbHETJjGR4cmWc3AqJKXrjePK3/h4pygC8p7o4AAAGG36by\nJgAABAMARzBFAiEA9rbuMD3hxqdm4BSY16cgpiE0+ZmfHNOEn8knRjzpwZECIDgh\n6kX4wM9d5IPilvFzn2x++ISKXiOKvfrKn1kKThTwMAoGCCqGSM49BAMDA2gAMGUC\nMEO/jxmuiPiPdfVDDcXAEZ0HTRUp9Wpcsf8vXdu1j84Uwoug53ivlumXoFq7heK1\ntgIxAPCol997A8+NqKUierl9DaEwhApnGZUS5rv1/SqjplJIHhELqT36h64yw9uC\nkP8eDg==\n-----END CERTIFICATE-----\n"
    }
  ]
}
```
So the provides the certificae which can be used to validate this attestation.
And recall from above the signature is over the [PAE](#PreAuthenicatedEncoding),
so basically the payloadType and the payload. And we could also have multiple
signatures which could be where you want multiple parties to sign. So we might
want out build system to sign this but in addition perhaps one of the core
maintainers also needs to sign it, in which case we would have two entries in
the signatures array.


[protocol]: https://github.com/secure-systems-lab/dsse/blob/master/protocol.md
