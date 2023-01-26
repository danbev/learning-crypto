## in-toto attestation
When we sign an artifact, like a blob, the signature proves that we were in
possesion of the private key. When we verify, we use the signature, the public
key, and the blob, and we are verifying that this was in fact the case.
But it does not say anything else about the artifact, we don't know what
was actually signed.

By providing, and signing a document specifying `statements` about the artifact
we can say things about the artifact as well. Statements could be anything which
we will address later in this document. A signed Statement is called an
Attestation.

An attestation is authenticated metadata about software artifacts and follows
the [Software-chain Levels for Software Artifacts attestation model](https://slsa.dev/attestation-model).

An attestation is a json object and the outer-most layer is the Envelope:
```json
{
  "payloadType": "application/vnd.in-toto+json",
  "payload": "<Base64(Statement)>",
  "signatures": [{"sig": "<Base64(Signature)>"}]
}
```
Notice that the payload is a base64 encoded `Statement`. Apparently this format
follows the [dsse](./dsse.md) format.

The payloadType could be JSON, CBOR, or ProtoBuf.

The structure of the `Statement`  looks something like this before it is
base64 encoded:
```json
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "subject": [ {
      "name": "<NAME>",
      "digest": {"<ALGORITHM>": "<HEX_VALUE>"}
    },
  ],
  "predicateType": "<URI>",
  "predicate": {}
}
```
The subjects bind this attestation to a set of software artifacts, notice that
this is an array of objects.

Each software artifact is given a name and a digest. The digest contains the
name of the hashing algorithm used and the digest (the outcome of the hash
function).
The name could be a file name but it can also be left unspecified using `_`.

This leads us to the `predicate` fields, which like shown above has one field
for the type of the predicate, and an object as the content of the predicate.

The predicate can contain pretty much any metadata related to the Statement
object's subjects. The `predicateType provides a way to knowing how to interpret
the predicate field.

Examples of predicate types are
[SLSA Provenance](https://slsa.dev/provenance/v0.1#example),
[Link](https://github.com/in-toto/attestation/blob/main/spec/predicates/link.md)
from in-toto 0.9, [SPDX](https://github.com/in-toto/attestation/blob/main/spec/predicates/spdx.md)
, [Software Supply Chain Attribute Integrity (SCAI)](https://github.com/in-toto/attestation/blob/main/spec/predicates/scai.md) document type.

NPM also uses this for it publish attestation:
```json
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "subject": [{
    "name": "pkg:npm/@scope/package-foo@1.4.3",
    "digest": { "sha512": "41o0P/CEffYGDqvo2pHQXRBOfFOxvYY3WkwkQTy..." }
  }],
  "predicateType": "https://github.com/npm/attestation/tree/main/specs/publish/v0.1",
  "predicate": {
    "name": "@scope/package-foo",
    "version": "1.4.3",
    "registry": "https://registry.npmjs.org",
  }
}
```
The `digest` in this case is the sha512sum of the published tar file.

So, we mentioned that the predicate type is used by the consumer of the
predicate so it knows how to interprete the contents of the predicate. But who
is the consumer?  
Most often this would be a Policy Engine. The Policy engine would be passed
the contents of the Statement related to the predicate, and rules written in
the Policy Engine's language would process the predicate as input. The outcome
would be a true/false result (remember that a predicate is a statment/function
that returns true or false).


Just keep in mind that the predicate is part of the Statement, which is base64
encoded, and then included in the `payload` field of the Envelope.

So we can imaging if we used a `predicateType` of `Link` then we would have a
json object in here like the contents of an in-toto link file.

Now, if we have created an Envelope as described above and we can provide or
make it available. From a consumers point of view they would take the Envelope,
the software artifact itself, a collection of name/public_key pairs to verify
the attesters of the Envelope.

### Processing an attestation
1) First the attestation is decoded as a JSON encoded Envelope.  

2) Next the statements signatures are collected and later used to verify all the
subjects.

If any of the above steps fail then validation fails. If the above steps pass,
then the ouput of the above will be fed into a policy engine:
* predicateType
* predicate
* artifactNames (gathered by the first step)
* attesterNames (gathered by the first step)

So this was the part that was not clear to me regarding the predicate, but this
is provided as input to a policy rule engine, like OPA, which can then
approve/deny depending on the rules written.

### in-toto-enhancements (ITE)
https://github.com/in-toto/ITE

[attestation spec]: https://github.com/in-toto/attestation/blob/main/spec/README.md
[ITE-5]: https://github.com/in-toto/ITE/tree/master/ITE/5

### Specification
The specification can be found [here](https://github.com/in-toto/attestation/blob/main/spec/README.md).
