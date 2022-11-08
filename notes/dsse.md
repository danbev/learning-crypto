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
keyid is optional. Notice that this format almost exactly like the format
suggested for in-toto attestations which is because it uses DSSE.
