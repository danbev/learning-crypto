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



[protocol]: https://github.com/secure-systems-lab/dsse/blob/master/protocol.md
