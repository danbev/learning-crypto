## Vulnerabilty Exploitablity Exchange (VEX)
Is an document that accompanies an SBOM and allows a software producer to
specify if their product is exploitable by know software vulnerabilities. This
can be important as just be cause there an exploit has been disclosed it might
not mean that the producers software is actually exploiteable using that
exploit. Often the exploits might only be performed under certain circumstances
like using a part of a thirdparty software. It the producer does not use that
part then the product they produce is not exploitable.

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
