### Verification Summary Attestation (VSA)
This is part of SLSA and it provides/proves that an artifact has been verified
at a specific SLSA Level and details about the verification. This also includes
SLSA levels of dependencies.

So this can be used by a consumer to make a decision about an artifact and its
deps without having to access all the attestations (including deps).

```json
"_type": "https://in-toto.io/Statement/v1",
"subject": [{
  "name": <NAME>,
  "digest": { <digest-in-request> }
}],

"predicateType": "https://slsa.dev/verification_summary/v1",
"predicate": {
  // Required
  "verifier": {
    "id": "<URI>"
  },
  "timeVerified": <TIMESTAMP>,
  "resourceUri": <artifact-URI-in-request>,
  "policy": ResourceDescriptor
  "inputAttestations": [ResourceDescriptor]
  ],
  "verificationResult": "<PASSED|FAILED>",
  "verifiedLevels": ["<SlsaResult>"],
  "dependencyLevels": {
    "<SlsaResult>": <Int>,
    "<SlsaResult>": <Int>,
    ...
  },
  "slsaVersion": "<MAJOR>.<MINOR>",
}
```
The `verifier` field is entity that performed the verification.

The `policy` field describes the `policy` that the `subject` was verfied against.

The `inputAttestations` field contains an array of the attestations that were
used to perform verification.

The `verifiedLevels` is an array of SlsaResult which can be thought of as an
enum:
```
enum SlsaResult {
  SLSA_BUILD_LEVEL_0,
  SLSA_BUILD_LEVEL_1,
  SLSA_BUILD_LEVEL_2,
  SLSA_BUILD_LEVEL_3,
  FAILED,
}
```

The `dependencyLevels` field contains a object/map where the SlsaResult is they
key and the value is the number of transitive deps that were verified at the
SlsaResult specific variant (enum value above that is).

The `slsaVersion` field specifies the version of the SLSA specification that
the verifier used (remember that this document is produced by the verifier).


[ResourceDescriptor]: https://github.com/in-toto/attestation/blob/main/spec/v1.0/resource_descriptor.md
