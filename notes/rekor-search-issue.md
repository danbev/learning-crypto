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
}
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
But this is not the same!! The payload hash is a hash of the Rekor rekord not
of the payload in attestations :( 

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

[Pull request #244](https://github.com/sigstore/sigstore-rs/pull/244) was opened
with a suggestion for fixing this issue.


[RekorType]: https://github.com/sigstore/rekor/blob/5b7385d35968ddba72debc3529889d12ce83ef84/pkg/types/types.go#L35
[openapi specification]: https://github.com/sigstore/rekor/blob/5b7385d35968ddba72debc3529889d12ce83ef84/openapi.yaml#L423
[V001Entry]: https://github.com/sigstore/rekor/blob/5b7385d35968ddba72debc3529889d12ce83ef84/pkg/types/intoto/v0.0.1/entry.go#L59
[sigstore::rekor::models::log_entry::Body]: https://github.com/sigstore/sigstore-rs/blob/8f084e9916e78eb0df75d7753f21c2a3da848f1a/src/rekor/models/log_entry.rs#L43
[Spec]: https://github.com/sigstore/sigstore-rs/blob/8f084e9916e78eb0df75d7753f21c2a3da848f1a/src/rekor/models/hashedrekord.rs#L39
[Intoto Spec]: https://github.com/sigstore/sigstore-rs/blob/8f084e9916e78eb0df75d7753f21c2a3da848f1a/src/rekor/models/intoto.rs#L16
