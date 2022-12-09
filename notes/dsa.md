## Digital Signature Algorithm (DSA)
Was developed in 1991 by NIST and concerns the following:

* Key Generation
* Key Distribution
* Signing
* Signature verification

### Overview
```
    Sender

  +-------------+      +------------+        +------------+
  | Message     |-----→| Message    |        | Message    |-----+
  +-------------+      |------------| ------>|------------|     |
       ↓               | signature  |        | signature  |     |
  +---------------+    +------------+        +------------+     |
  | Hash function |          ↑                     |            ↓
  +---------------+          |                     |       +---------------+
       |                     |                     |       | Hash function |
  +---------------+          |                     |       +---------------+
  |  digest       |          |                     |            |
  +---------------+          |                     |    +-------+
       ↓                     |                     ↓    ↓
  +---------------+    +-----------+         +--------------+     
  | sign function |--->| signature |         | verification |---→ true | false
  +---------------+    +-----------+         +--------------+
       ↑                                           ↑
  +---------------+                          +--------------------+
  | private key   |                          | senders public key |
  +---------------+                          +--------------------+
```

Notice that we have have a hash function, and we also need to have a private
and a public key, as well as a signature function that takes the hash/digest
and computes a signature. This signature is sent with message to the receiver.

The receiver takes the signature and passes it to a verification function with
the senders public key. Now, the verification function will try to decrypt the
signature and if successful it will output true, otherwise false.

