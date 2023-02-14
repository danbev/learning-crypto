## Vulnerabilty Exploitablity Exchange (VEX)
Is a document that accompanies an SBOM and allows a software producer to
specify if their product is exploitable by know software vulnerabilities. This
can be important as just be cause there an exploit has been disclosed it might
not mean that the producer's software is actually exploitable.
Often the exploits might only be performed under certain circumstances
like it might only be in a small part of a thirdparty software. It the producer
does not use that part, then the product they produce is not exploitable.

Apperently large software companies created VEX to avoid having their customer
call in about vulnerability which would incure a large cost. But this also helpful
the end users. For example an exploit might be made public and a manager knows
that a team uses that library and asks the obvious question, are we vulnerable
to this exploit? 

So a VEX document would be created by how? The thirdparty software developers,
or are then written by the consumers of that thirdparty?
Well, most software today is made up on a number of dependencies, and an
open source dependencies might have it's own dependencies. So in that case
the open source project would create a VEX decribing if/how or not it is
vulnerable to an exploit. This can then be consumed by downstream projects to
quickly assess if they are vulnerable or not. It short of bubbles up the
dependency chain.

### Producing VEX documents
The [scenario] in the spec I found has very helpful to understand VEX documents
are produced.

### Formats
There are currently two standardized formats:
* Common Security Advisory Framwwork (CSAF) 
* CycloneDX

[scenario]: https://github.com/openvex/spec/blob/main/OPENVEX-SPEC.md#a-sample-scenario

### OpenVEX

Lets take a look at this [openvex example]:
```json
{
  "@context": "https://openvex.dev/ns",
  "@id": "https://openvex.dev/docs/example/vex-9fb3463de1b57",
  "author": "Wolfi J Inkinson",
  "role": "Document Creator",
  "timestamp": "2023-01-08T18:02:03.647787998-06:00",
  "version": "1",
  "statements": [
    {
      "vulnerability": "CVE-2023-12345",
      "products": [
        "pkg:apk/wolfi/git@2.39.0-r1?arch=armv7",
        "pkg:apk/wolfi/git@2.39.0-r1?arch=x86_64"
      ],
      "status": "fixed"
    }
  ]
}
```
The first to elements of this JSON Object are JSON for Linking Data [JSON-LD],
which are `@context`, and `@id`.

[OpenVEX example]: (https://github.com/openvex/spec/blob/main/OPENVEX-SPEC.md#document-1
[JSON-LD]: https://json-ld.org/
