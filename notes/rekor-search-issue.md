## Rekor search issue
This page describes an issue I ran into when using sigstore.rs.

What I'm trying to do is to search Rekor using a hash, similar to what can be
done with `rekor-cli`:
```console
$ rekor-cli search --sha 39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8
Found matching entries (listed by UUID):
24296fb24b8ad77a27dbac48342e32d86ca2e056106ff7e94cd4a6ed2a12de00b7ffc57fc2f2c2e4
24296fb24b8ad77ae0d13d8e6787e796456e52513fcb8a0bf77fd6f338a1575296bc8a9653e50007
```
The hash is specified  using `--sha` and is the hash of an intoto attestation.
```
{                                                                               
  "payloadType": "application/vnd.in-toto+json",                                
  "payload": "eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3Nsc2EuZGV2L3Byb3ZlbmFuY2UvdjAuMiIsInN1YmplY3QiOm51bGwsInByZWRpY2F0ZSI6eyJidWlsZGVyIjp7ImlkIjoiaHR0cHM6Ly90ZWt0b24uZGV2L2NoYWlucy92MiJ9LCJidWlsZFR5cGUiOiJ0ZWt0b24uZGV2L3YxYmV0YTEvVGFza1J1biIsImludm9jYXRpb24iOnsiY29uZmlnU291cmNlIjp7fSwicGFyYW1ldGVycyI6e319LCJidWlsZENvbmZpZyI6eyJzdGVwcyI6W3siZW50cnlQb2ludCI6IiMhL3Vzci9iaW4vZW52IHNoXG5lY2hvICdnY3IuaW8vZm9vL2JhcicgfCB0ZWUgL3Rla3Rvbi9yZXN1bHRzL1RFU1RfVVJMXG5lY2hvIFwiZGFuYmV2LXRla3Rvbi1jaGFpbnMtZXhhbXBsZVwiIHwgc2hhMjU2c3VtIHwgdHIgLWQgJy0nIHwgdGVlIC90ZWt0b24vcmVzdWx0cy9URVNUX0RJR0VTVCIsImFyZ3VtZW50cyI6bnVsbCwiZW52aXJvbm1lbnQiOnsiY29udGFpbmVyIjoiY3JlYXRlLWltYWdlIiwiaW1hZ2UiOiJkb2NrZXIuaW8vbGlicmFyeS9idXN5Ym94QHNoYTI1NjpiNWQ2ZmUwNzEyNjM2Y2ViNzQzMDE4OWRlMjg4MTllMTk1ZTg5NjYzNzJlZGZjMmQ5NDA5ZDc5NDAyYTBkYzE2In0sImFubm90YXRpb25zIjpudWxsfV19LCJtZXRhZGF0YSI6eyJidWlsZFN0YXJ0ZWRPbiI6IjIwMjMtMDMtMjJUMDk6NTc6MTVaIiwiYnVpbGRGaW5pc2hlZE9uIjoiMjAyMy0wMy0yMlQwOTo1NzoxOVoiLCJjb21wbGV0ZW5lc3MiOnsicGFyYW1ldGVycyI6ZmFsc2UsImVudmlyb25tZW50IjpmYWxzZSwibWF0ZXJpYWxzIjpmYWxzZX0sInJlcHJvZHVjaWJsZSI6ZmFsc2V9fX0=",
  "signatures": [                                                               
    {                                                                           
      "keyid": "SHA256:caEJWYJSxy1SVF2KObm5Rr3Yt6xIb4T2w56FHtCg8WI",            
      "sig": "MEQCIASjypkm8V/uVJQTn/ttOIYr0Ck50CLfSagQkS11eyR/AiA3eHPYVYrGmcFhly6X9f15bDilUBeXuPj8g2x4SFCADQ=="
    }                                                                           
  ]                                                                             
```
And the decoded payload looks like this:
```console
$ echo "eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3Nsc2EuZGV2L3Byb3ZlbmFuY2UvdjAuMiIsInN1YmplY3QiOm51bGwsInByZWRpY2F0ZSI6eyJidWlsZGVyIjp7ImlkIjoiaHR0cHM6Ly90ZWt0b24uZGV2L2NoYWlucy92MiJ9LCJidWlsZFR5cGUiOiJ0ZWt0b24uZGV2L3YxYmV0YTEvVGFza1J1biIsImludm9jYXRpb24iOnsiY29uZmlnU291cmNlIjp7fSwicGFyYW1ldGVycyI6e319LCJidWlsZENvbmZpZyI6eyJzdGVwcyI6W3siZW50cnlQb2ludCI6IiMhL3Vzci9iaW4vZW52IHNoXG5lY2hvICdnY3IuaW8vZm9vL2JhcicgfCB0ZWUgL3Rla3Rvbi9yZXN1bHRzL1RFU1RfVVJMXG5lY2hvIFwiZGFuYmV2LXRla3Rvbi1jaGFpbnMtZXhhbXBsZVwiIHwgc2hhMjU2c3VtIHwgdHIgLWQgJy0nIHwgdGVlIC90ZWt0b24vcmVzdWx0cy9URVNUX0RJR0VTVCIsImFyZ3VtZW50cyI6bnVsbCwiZW52aXJvbm1lbnQiOnsiY29udGFpbmVyIjoiY3JlYXRlLWltYWdlIiwiaW1hZ2UiOiJkb2NrZXIuaW8vbGlicmFyeS9idXN5Ym94QHNoYTI1NjpiNWQ2ZmUwNzEyNjM2Y2ViNzQzMDE4OWRlMjg4MTllMTk1ZTg5NjYzNzJlZGZjMmQ5NDA5ZDc5NDAyYTBkYzE2In0sImFubm90YXRpb25zIjpudWxsfV19LCJtZXRhZGF0YSI6eyJidWlsZFN0YXJ0ZWRPbiI6IjIwMjMtMDMtMjJUMDk6NTc6MTVaIiwiYnVpbGRGaW5pc2hlZE9uIjoiMjAyMy0wMy0yMlQwOTo1NzoxOVoiLCJjb21wbGV0ZW5lc3MiOnsicGFyYW1ldGVycyI6ZmFsc2UsImVudmlyb25tZW50IjpmYWxzZSwibWF0ZXJpYWxzIjpmYWxzZX0sInJlcHJvZHVjaWJsZSI6ZmFsc2V9fX0=" | base64 -d | jq
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "predicateType": "https://slsa.dev/provenance/v0.2",
  "subject": null,
  "predicate": {
    "builder": {
      "id": "https://tekton.dev/chains/v2"
    },
    "buildType": "tekton.dev/v1beta1/TaskRun",
    "invocation": {
      "configSource": {},
      "parameters": {}
    },
    "buildConfig": {
      "steps": [
        {
          "entryPoint": "#!/usr/bin/env sh\necho 'gcr.io/foo/bar' | tee /tekton/results/TEST_URL\necho \"danbev-tekton-chains-example\" | sha256sum | tr -d '-' | tee /tekton/results/TEST_DIGEST",
          "arguments": null,
          "environment": {
            "container": "create-image",
            "image": "docker.io/library/busybox@sha256:b5d6fe0712636ceb7430189de28819e195e8966372edfc2d9409d79402a0dc16"
          },
          "annotations": null
        }
      ]
    },
    "metadata": {
      "buildStartedOn": "2023-03-22T09:57:15Z",
      "buildFinishedOn": "2023-03-22T09:57:19Z",
      "completeness": {
        "parameters": false,
        "environment": false,
        "materials": false
      },
      "reproducible": false
    }
  }
}
```
And the hash value is just the `sha256sum` of the payload:
```console
$ echo "eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3Nsc2EuZGV2L3Byb3ZlbmFuY2UvdjAuMiIsInN1YmplY3QiOm51bGwsInByZWRpY2F0ZSI6eyJidWlsZGVyIjp7ImlkIjoiaHR0cHM6Ly90ZWt0b24uZGV2L2NoYWlucy92MiJ9LCJidWlsZFR5cGUiOiJ0ZWt0b24uZGV2L3YxYmV0YTEvVGFza1J1biIsImludm9jYXRpb24iOnsiY29uZmlnU291cmNlIjp7fSwicGFyYW1ldGVycyI6e319LCJidWlsZENvbmZpZyI6eyJzdGVwcyI6W3siZW50cnlQb2ludCI6IiMhL3Vzci9iaW4vZW52IHNoXG5lY2hvICdnY3IuaW8vZm9vL2JhcicgfCB0ZWUgL3Rla3Rvbi9yZXN1bHRzL1RFU1RfVVJMXG5lY2hvIFwiZGFuYmV2LXRla3Rvbi1jaGFpbnMtZXhhbXBsZVwiIHwgc2hhMjU2c3VtIHwgdHIgLWQgJy0nIHwgdGVlIC90ZWt0b24vcmVzdWx0cy9URVNUX0RJR0VTVCIsImFyZ3VtZW50cyI6bnVsbCwiZW52aXJvbm1lbnQiOnsiY29udGFpbmVyIjoiY3JlYXRlLWltYWdlIiwiaW1hZ2UiOiJkb2NrZXIuaW8vbGlicmFyeS9idXN5Ym94QHNoYTI1NjpiNWQ2ZmUwNzEyNjM2Y2ViNzQzMDE4OWRlMjg4MTllMTk1ZTg5NjYzNzJlZGZjMmQ5NDA5ZDc5NDAyYTBkYzE2In0sImFubm90YXRpb25zIjpudWxsfV19LCJtZXRhZGF0YSI6eyJidWlsZFN0YXJ0ZWRPbiI6IjIwMjMtMDMtMjJUMDk6NTc6MTVaIiwiYnVpbGRGaW5pc2hlZE9uIjoiMjAyMy0wMy0yMlQwOTo1NzoxOVoiLCJjb21wbGV0ZW5lc3MiOnsicGFyYW1ldGVycyI6ZmFsc2UsImVudmlyb25tZW50IjpmYWxzZSwibWF0ZXJpYWxzIjpmYWxzZX0sInJlcHJvZHVjaWJsZSI6ZmFsc2V9fX0=" | base64 -d | sha256sum
3e3da42ea26ecdb7b826fa15d9ee5b73ad58187fc702ae2098937b946bc2e01c
```
Now, with the `rekor-cli` tool we can now look up one of the uuid's returned.
```console
Found matching entries (listed by UUID):
24296fb24b8ad77a27dbac48342e32d86ca2e056106ff7e94cd4a6ed2a12de00b7ffc57fc2f2c2e4
24296fb24b8ad77ae0d13d8e6787e796456e52513fcb8a0bf77fd6f338a1575296bc8a9653e50007
```
```console
$ rekor-cli get --uuid 24296fb24b8ad77ae0d13d8e6787e796456e52513fcb8a0bf77fd6f338a1575296bc8a9653e50007 --format json | jq
{
  "Attestation": "{\"_type\":\"https://in-toto.io/Statement/v0.1\",\"predicateType\":\"https://slsa.dev/provenance/v0.2\",\"subject\":null,\"predicate\":{\"builder\":{\"id\":\"https://tekton.dev/chains/v2\"},\"buildType\":\"tekton.dev/v1beta1/TaskRun\",\"invocation\":{\"configSource\":{},\"parameters\":{}},\"buildConfig\":{\"steps\":[{\"entryPoint\":\"#!/usr/bin/env sh\\necho 'gcr.io/foo/bar' | tee /tekton/results/TEST_URL\\necho \\\"danbev-tekton-chains-example\\\" | sha256sum | tr -d '-' | tee /tekton/results/TEST_DIGEST\",\"arguments\":null,\"environment\":{\"container\":\"create-image\",\"image\":\"docker.io/library/busybox@sha256:b5d6fe0712636ceb7430189de28819e195e8966372edfc2d9409d79402a0dc16\"},\"annotations\":null}]},\"metadata\":{\"buildStartedOn\":\"2023-03-22T10:05:59Z\",\"buildFinishedOn\":\"2023-03-22T10:06:03Z\",\"completeness\":{\"parameters\":false,\"environment\":false,\"materials\":false},\"reproducible\":false}}}",
  "AttestationType": "",
  "Body": {
    "IntotoObj": {
      "content": {
        "hash": {
          "algorithm": "sha256",
          "value": "403e299edc4fe36ce3ca92baaf8f101303bd4474592e91db31041d3ade6119eb"
        },
        "payloadHash": {
          "algorithm": "sha256",
          "value": "39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8"
        }
      },
      "publicKey": "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFcWlMdUFyUmNaQ1kxczY1MHJnS1VEcGo3ZitiOAo5SE11M0svUERhVWNSOWtjeXlYWThxNlUrVEZUa2M5dTg0d0pUc1plMjF3QlBkL1NUUEV6bzBKcnpRPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="
    }
  },
  "LogIndex": 16027962,
  "IntegratedTime": 1679479563,
  "UUID": "24296fb24b8ad77ae0d13d8e6787e796456e52513fcb8a0bf77fd6f338a1575296bc8a9653e50007",
  "LogID": "c0d23d6ad406973f9559f3ba2d1ca01f84147d8ffc5b8445c224f98b9591801d"
}
```
And in the use case I have I was interesed in the public key:
```console
$ rekor-cli get --uuid 24296fb24b8ad77ae0d13d8e6787e796456e52513fcb8a0bf77fd6f338a1575296bc8a9653e50007 --format json | jq -r '.Body.IntotoObj.publicKey' | base64 -d
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEqiLuArRcZCY1s650rgKUDpj7f+b8
9HMu3K/PDaUcR9kcyyXY8q6U+TFTkc9u84wJTsZe21wBPd/STPEzo0JrzQ==
-----END PUBLIC KEY-----
```

So I tried using sigstore.rs which has support for interacting with Rekor and
the is a `search-index` which can pass the hash value simlar to what we did
above with the rekor-cli tool:
```console
$ cargo run --example search_index -- --hash="39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8" 
     Running `target/debug/examples/search_index --hash=39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8`
[
    "24296fb24b8ad77a27dbac48342e32d86ca2e056106ff7e94cd4a6ed2a12de00b7ffc57fc2f2c2e4",
    "24296fb24b8ad77ae0d13d8e6787e796456e52513fcb8a0bf77fd6f338a1575296bc8a9653e50007",
]
```
And we can see that returns the same two uuid's as we say earlier. Next we want
to use those uuid's and call `get_log_entry_by_uuid`:
```rust
for uuid in uuid_vec {
    let result = entries_api::get_log_entry_by_uuid(&configuration, &uuid).await;
    println!("{:#?}", result);
}
```
But this fails with the following error:
```console
thread 'main' panicked at 'Failed to decode Body: SerdeJsonError(Error("missing field `signature`", line: 1, column: 530))', src/rekor/models/log_entry.rs:34:22
```

```console
{
"uuid": String("24296fb24b8ad77a27dbac48342e32d86ca2e056106ff7e94cd4a6ed2a12de00b7ffc57fc2f2c2e4"),
"logID": String("c0d23d6ad406973f9559f3ba2d1ca01f84147d8ffc5b8445c224f98b9591801d"),
"body": String("eyJhcGlWZXJzaW9uIjoiMC4wLjEiLCJraW5kIjoiaW50b3RvIiwic3BlYyI6eyJjb250ZW50Ijp7Imhhc2giOnsiYWxnb3JpdGhtIjoic2hhMjU2IiwidmFsdWUiOiIxM2NlMDhmN2JhMjU2ZjM5NDcwMGI2ZWU2OGFmY2Y4NWRjMDZjNDM4NTg3NTUxMzE5ZDNlMTUwYTI0N2RlMGExIn0sInBheWxvYWRIYXNoIjp7ImFsZ29yaXRobSI6InNoYTI1NiIsInZhbHVlIjoiMzllODY0MTNhN2YxM2ExZTIwZDFiYjkxNWRmNGZhYjhhNTg2NzdlNzhhNmIzMzViYjJlNjE0YmU4YmVmMWRjOCJ9fSwicHVibGljS2V5IjoiTFMwdExTMUNSVWRKVGlCUVZVSk1TVU1nUzBWWkxTMHRMUzBLVFVacmQwVjNXVWhMYjFwSmVtb3dRMEZSV1VsTGIxcEplbW93UkVGUlkwUlJaMEZGY1dsTWRVRnlVbU5hUTFreGN6WTFNSEpuUzFWRWNHbzNaaXRpT0FvNVNFMTFNMHN2VUVSaFZXTlNPV3RqZVhsWVdUaHhObFVyVkVaVWEyTTVkVGcwZDBwVWMxcGxNakYzUWxCa0wxTlVVRVY2YnpCS2NucFJQVDBLTFMwdExTMUZUa1FnVUZWQ1RFbERJRXRGV1MwdExTMHRDZz09In19"),
"logIndex": Number(16028007),
"verification": Object {"inclusionProof": Object {"checkpoint": String("rekor.sigstore.dev - 2605736670972794746\n11956977\nbsvNvfX9esnxdaxvAUZ95z7/TqW8K/QCTTZ9488hhSE=\nTimestamp: 1679575021729507429\n\n— rekor.sigstore.dev wNI9ajBEAiBT4UXhcgXKxJAJrqoE4L5AHGpF9cMI/CaRt39/ulvMjQIgCyKEtY8LGT3cZyptA73dtP9pyMuLN8T3XhPfG8P/VpA=\n"),
"hashes": Array [String("3f1221ed0d7c405db61b0ad3b7f661c69bfe78edf04f9cc9308157280cc078f3"),
String("43dc68a51dc9810f1be1f1ab06f51010271dfa09d322c1eee5c5255c7f03a958"),
String("9c3bb52fefd243164ee238e7c40c7721bf12dcbfc67a43b51ae9c1d6f8df27cf"),
String("5cf566aeb4e101c8f6ced114fa450bdee0042fdfd470dc86ffc14c5197823252"),
String("c173ec1516068a47769b36a124a3b8ff454d1239b280ceb0b0833e734f108feb"),
String("33c63510a658c197ce6a9a60e4544f4306de8aaddc4adcdeb395e41a662c2490"),
String("a53d4a13bffe11b29009bbbd073cb0b124019acbdbab1a5bca00b91718618571"),
String("cdefdaf6418611cf4fdf2b44363362aef18cc513d0490beca513568d4b5f86d7"),
String("493c15bd69ec404a3d8c5f443f28463a6c7609d335205a46c96f5eeea04a397d"),
String("f2017227ad1f41da87c707647aa2b2af308f4f32c19c564c66378ebdee2ceed1"),
String("08d3fd2ff04ea48f1db4f37b77c77ba68c9338da5364f6d82bab0566277c39a2"),
String("212391d9c5218d3af59e1d8266c2e888c07f91e4caa2c4210e80c058f207b9f0"),
String("ff6ef17eb8375a86cf192e229b7866a986cd3ec331e43efb8992a4dd4b31820c"),
String("1265f0ce11e097ee0f9f07a69ab6420bff4968142633f873265b769a2111d411"),
String("c447d9ff002a9569a5e562d1f880f35d4fdebe67e65661dec5bd5697ba2c2e91"),
String("36349b0cecd1c87f216cde73effb87f1ce372b33e88231d70876602a08806c4b"),
String("3e42964add9d10de9b112bf9996dd5f3d7373279c095f6d4636401ce4a73a493"),
String("ef3ebd52c145b439f9d7f5258e73fdfdc5625e5e80da4502b66ce366499462f7"),
String("dd29ff68a0cd214aa4a5a2847962c98f75f11c1973727cb108090cc0411134d1"),
String("a73e5b5766f077934d4d2c307d5a02a63d1d31b6efb23f6847de1ac33345d56d"),
String("0c20f6f7b6738b7eb7957b48b6fd750c6f997586a935f4235e7810876cc1674a"),
String("9e040066dfe5f02004658386ac66cf0bb6ffe857ed71cb337c7f5545ecf4558b")],
"logIndex": Number(11864576),
"rootHash": String("6ecbcdbdf5fd7ac9f175ac6f01467de73eff4ea5bc2bf4024d367de3cf218521"),
"treeSize": Number(11956977)},
"signedEntryTimestamp": String("MEUCIQDIsUQrDVAmRyyDbojn1PyM6mXKBizdbxaN0f8DUg14WgIgSGCisGD+VwNRc6VF3GrW1nGtJfGrGi2Lt8UM4S35sNA=")},
 "attestation": Object {"data": String("eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3Nsc2EuZGV2L3Byb3ZlbmFuY2UvdjAuMiIsInN1YmplY3QiOm51bGwsInByZWRpY2F0ZSI6eyJidWlsZGVyIjp7ImlkIjoiaHR0cHM6Ly90ZWt0b24uZGV2L2NoYWlucy92MiJ9LCJidWlsZFR5cGUiOiJ0ZWt0b24uZGV2L3YxYmV0YTEvVGFza1J1biIsImludm9jYXRpb24iOnsiY29uZmlnU291cmNlIjp7fSwicGFyYW1ldGVycyI6e319LCJidWlsZENvbmZpZyI6eyJzdGVwcyI6W3siZW50cnlQb2ludCI6IiMhL3Vzci9iaW4vZW52IHNoXG5lY2hvICdnY3IuaW8vZm9vL2JhcicgfCB0ZWUgL3Rla3Rvbi9yZXN1bHRzL1RFU1RfVVJMXG5lY2hvIFwiZGFuYmV2LXRla3Rvbi1jaGFpbnMtZXhhbXBsZVwiIHwgc2hhMjU2c3VtIHwgdHIgLWQgJy0nIHwgdGVlIC90ZWt0b24vcmVzdWx0cy9URVNUX0RJR0VTVCIsImFyZ3VtZW50cyI6bnVsbCwiZW52aXJvbm1lbnQiOnsiY29udGFpbmVyIjoiY3JlYXRlLWltYWdlIiwiaW1hZ2UiOiJkb2NrZXIuaW8vbGlicmFyeS9idXN5Ym94QHNoYTI1NjpiNWQ2ZmUwNzEyNjM2Y2ViNzQzMDE4OWRlMjg4MTllMTk1ZTg5NjYzNzJlZGZjMmQ5NDA5ZDc5NDAyYTBkYzE2In0sImFubm90YXRpb25zIjpudWxsfV19LCJtZXRhZGF0YSI6eyJidWlsZFN0YXJ0ZWRPbiI6IjIwMjMtMDMtMjJUMTA6MDU6NTlaIiwiYnVpbGRGaW5pc2hlZE9uIjoiMjAyMy0wMy0yMlQxMDowNjowM1oiLCJjb21wbGV0ZW5lc3MiOnsicGFyYW1ldGVycyI6ZmFsc2UsImVudmlyb25tZW50IjpmYWxzZSwibWF0ZXJpYWxzIjpmYWxzZX0sInJlcHJvZHVjaWJsZSI6ZmFsc2V9fX0=")},
"integratedTime": Number(1679479591)
}
```
A GET request to /api/v1/log/entries/{entryUUID} will return a LogEntry which
is defined in the [openapi specification] and we can see that the body element
is defined to of type object:
```
        body:
          type: object
          additionalProperties: true
```

Now if we take the body above and base64 decode it we get:
```console
$ echo "eyJhcGlWZXJzaW9uIjoiMC4wLjEiLCJraW5kIjoiaW50b3RvIiwic3BlYyI6eyJjb250ZW50Ijp7Imhhc2giOnsiYWxnb3JpdGhtIjoic2hhMjU2IiwidmFsdWUiOiIxM2NlMDhmN2JhMjU2ZjM5NDcwMGI2ZWU2OGFmY2Y4NWRjMDZjNDM4NTg3NTUxMzE5ZDNlMTUwYTI0N2RlMGExIn0sInBheWxvYWRIYXNoIjp7ImFsZ29yaXRobSI6InNoYTI1NiIsInZhbHVlIjoiMzllODY0MTNhN2YxM2ExZTIwZDFiYjkxNWRmNGZhYjhhNTg2NzdlNzhhNmIzMzViYjJlNjE0YmU4YmVmMWRjOCJ9fSwicHVibGljS2V5IjoiTFMwdExTMUNSVWRKVGlCUVZVSk1TVU1nUzBWWkxTMHRMUzBLVFVacmQwVjNXVWhMYjFwSmVtb3dRMEZSV1VsTGIxcEplbW93UkVGUlkwUlJaMEZGY1dsTWRVRnlVbU5hUTFreGN6WTFNSEpuUzFWRWNHbzNaaXRpT0FvNVNFMTFNMHN2VUVSaFZXTlNPV3RqZVhsWVdUaHhObFVyVkVaVWEyTTVkVGcwZDBwVWMxcGxNakYzUWxCa0wxTlVVRVY2YnpCS2NucFJQVDBLTFMwdExTMUZUa1FnVUZWQ1RFbERJRXRGV1MwdExTMHRDZz09In19" | base64 -d

{"apiVersion":"0.0.1","kind":"intoto","spec":{"content":{"hash":{"algorithm":"sha256","value":"13ce08f7ba256f394700b6ee68afcf85dc06c438587551319d3e150a247de0a1"},"payloadHash":{"algorithm":"sha256","value":"39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8"}},"publicKey":"LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFcWlMdUFyUmNaQ1kxczY1MHJnS1VEcGo3ZitiOAo5SE11M0svUERhVWNSOWtjeXlYWThxNlUrVEZUa2M5dTg0d0pUc1plMjF3QlBkL1NUUEV6bzBKcnpRPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="}}$ echo "eyJhcGlWZXJzaW9uIjoiMC4wLjEiLCJraW5kIjoiaW50b3RvIiwic3BlYyI6eyJjb250ZW50Ijp7Imhhc2giOnsiYWxnb3JpdGhtIjoic2hhMjU2IiwidmFsdWUiOiIxM2NlMDhmN2JhMjU2ZjM5NDcwMGI2ZWU2OGFmY2Y4NWRjMDZjNDM4NTg3NTUxMzE5ZDNlMTUwYTI0N2RlMGExIn0sInBheWxvYWRIYXNoIjp7ImFsZ29yaXRobSI6InNoYTI1NiIsInZhbHVlIjoiMzllODY0MTNhN2YxM2ExZTIwZDFiYjkxNWRmNGZhYjhhNTg2NzdlNzhhNmIzMzViYjJlNjE0YmU4YmVmMWRjOCJ9fSwicHVibGljS2V5IjoiTFMwdExTMUNSVWRKVGlCUVZVSk1TVU1nUzBWWkxTMHRMUzBLVFVacmQwVjNXVWhMYjFwSmVtb3dRMEZSV1VsTGIxcEplbW93UkVGUlkwUlJaMEZGY1dsTWRVRnlVbU5hUTFreGN6WTFNSEpuUzFWRWNHbzNaaXRpT0FvNVNFMTFNMHN2VUVSaFZXTlNPV3RqZVhsWVdUaHhObFVyVkVaVWEyTTVkVGcwZDBwVWMxcGxNakYzUWxCa0wxTlVVRVY2YnpCS2NucFJQVDBLTFMwdExTMUZUa1FnVUZWQ1RFbERJRXRGV1MwdExTMHRDZz09In19" | base64 -d | jq
{
  "apiVersion": "0.0.1",
  "kind": "intoto",
  "spec": {
    "content": {
      "hash": {
        "algorithm": "sha256",
        "value": "13ce08f7ba256f394700b6ee68afcf85dc06c438587551319d3e150a247de0a1"
      },
      "payloadHash": {
        "algorithm": "sha256",
        "value": "39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8"
      }
    },
    "publicKey": "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFcWlMdUFyUmNaQ1kxczY1MHJnS1VEcGo3ZitiOAo5SE11M0svUERhVWNSOWtjeXlYWThxNlUrVEZUa2M5dTg0d0pUc1plMjF3QlBkL1NUUEV6bzBKcnpRPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="
  }
}
```
So that is what the body actually contains which matches [V001Entry].
Notice that the body has a `kind` which which is part of [RekorType]:
```go
// RekorType is the base struct that is embedded in all type implementations
type RekorType struct {
	Kind       string                 // this is the unique string that identifies the type
	VersionMap VersionEntryFactoryMap // this maps the supported versions to implementation
}
```
And the kind `intoto` is declared as:
```go
const (
	KIND = "intoto"
)

type BaseIntotoType struct {
	types.RekorType
}
```
Now, if we look at the sigstore-rs code we can see that it is not inspecting
the `kind` but instead trying to deserialize the body into the type
[sigstore::rekor::models::log_entry::Body]
```rust
#[derive(Default, Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct Body {
    #[serde(rename = "kind")]
    pub kind: String,
    #[serde(rename = "apiVersion")]
    pub api_version: String,
    #[serde(rename = "spec")]
    pub spec: Spec,
}
```
[Spec] is defined like this:
```rust
/// Stores the Signature and Data struct
#[derive(Default, Debug, Clone, Eq, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Spec {
    pub signature: Signature,
    pub data: Data,
}
```
Now, when serde tries to deserialise the json we have above it will not be able
to as the json does not contain a signature field. In our case the `Spec` in
this case should be [Intoto Spec]:
```rust
#[derive(Clone, Debug, PartialEq, Eq, Default, Serialize, Deserialize)]
pub struct Intoto {
    #[serde(rename = "kind")]
    pub kind: String,
    #[serde(rename = "apiVersion")]
    pub api_version: String,
    #[serde(rename = "spec")]
    pub spec: serde_json::Value,
}
```

The contents of the body will look this when the kind is intoto:
```
{
  "apiVersion": "0.0.1",
  "kind": "intoto",

  "spec": {
    "content": {
      "hash": {
        "algorithm": "sha256",
        "value": "13ce08f7ba256f394700b6ee68afcf85dc06c438587551319d3e150a247de0a1"},
        "payloadHash": {
          "algorithm": "sha256",
          "value": "39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8"}
     },
     "publicKey": "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFcWlMdUFyUmNaQ1kxczY1MHJnS1VEcGo3ZitiOAo5SE11M0svUERhVWNSOWtjeXlYWThxNlUrVEZUa2M5dTg0d0pUc1plMjF3QlBkL1NUUEV6bzBKcnpRPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="
  }
}
```

Perhaps we can change the definition of Body to be something like the following:
```rust
/// Stores the response returned by Rekor after making a new entry
#[derive(Default, Debug, Clone, Eq, PartialEq, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LogEntry {
    pub uuid: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub attestation: Option<Attestation>,
    pub body: Body,
    pub integrated_time: i64,
    pub log_i_d: String,
    pub log_index: i64,
    pub verification: Verification,
}

#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(tag = "kind")]
#[allow(non_camel_case_types)]
pub enum Body {
    alpine(AlpineAllOf),
    helm(HelmAllOf),
    jar(JarAllOf),
    rfc3161(Rfc3161AllOf),
    rpm(RpmAllOf),
    tuf(TufAllOf),
    intoto(IntotoAllOf),
    hashedrekord(HashedrekordAllOf),
    rekord(RekordAllOf),
}
```
This would allow the body of LogEntry to be following for an Intotot kind:
```console
intoto(IntotoAllOf { api_version: "0.0.1", spec: Object {"content": Object {"hash": Object {"algorithm": String("sha256"), "value": String("13ce08f7ba256f394700b6ee68afcf85dc06c438587551319d3e150a247de0a1")}, "payloadHash": Object {"algorithm": String("sha256"), "value": String("39e86413a7f13a1e20d1bb915df4fab8a58677e78a6b335bb2e614be8bef1dc8")}}, "publicKey": String("LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFcWlMdUFyUmNaQ1kxczY1MHJnS1VEcGo3ZitiOAo5SE11M0svUERhVWNSOWtjeXlYWThxNlUrVEZUa2M5dTg0d0pUc1plMjF3QlBkL1NUUEV6bzBKcnpRPT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg==")} })
```
And for a hashedrekord kind it would look like this:
```console
hashedrekord(HashedrekordAllOf { api_version: "0.0.1", spec: Object {"data": Object {"hash": Object {"algorithm": String("sha256"), "value": String("c7ead87fa5c82d2b17feece1c2ee1bda8e94788f4b208de5057b3617a42b7413")}}, "signature": Object {"content": String("MEUCIHWACbBnw+YkJCy2tVQd5i7VH6HgkdVBdP7HRV1IEsDuAiEA19iJNvmkE6We7iZGjHsTkjXV8QhK9iXu0ArUxvJF1N8="), "publicKey": Object {"content": String("LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUZrd0V3WUhLb1pJemowQ0FRWUlLb1pJemowREFRY0RRZ0FFeEhUTWRSQk80ZThCcGZ3cG5KMlozT2JMRlVrVQpaUVp6WGxtKzdyd1lZKzhSMUZpRWhmS0JZclZraGpHL2lCUjZac2s3Z01iYWZPOG9FM01lUEVvWU93PT0KLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg==")}}} })
```


__wip__


[RekorType]: https://github.com/sigstore/rekor/blob/5b7385d35968ddba72debc3529889d12ce83ef84/pkg/types/types.go#L35



[openapi specification]: https://github.com/sigstore/rekor/blob/5b7385d35968ddba72debc3529889d12ce83ef84/openapi.yaml#L423
[V001Entry]: https://github.com/sigstore/rekor/blob/5b7385d35968ddba72debc3529889d12ce83ef84/pkg/types/intoto/v0.0.1/entry.go#L59
[sigstore::rekor::models::log_entry::Body]: https://github.com/sigstore/sigstore-rs/blob/8f084e9916e78eb0df75d7753f21c2a3da848f1a/src/rekor/models/log_entry.rs#L43
[Spec]: https://github.com/sigstore/sigstore-rs/blob/8f084e9916e78eb0df75d7753f21c2a3da848f1a/src/rekor/models/hashedrekord.rs#L39
[Intoto Spec]: https://github.com/sigstore/sigstore-rs/blob/8f084e9916e78eb0df75d7753f21c2a3da848f1a/src/rekor/models/intoto.rs#L16
