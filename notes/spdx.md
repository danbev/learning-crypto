## Software Product Data Exchange (SPDX)
[Specification](https://spdx.dev/specifications/)

Is an ISO standard and covers components, licences, and copyrights associated
with software projects.
Is under the Linux Foundation.

A spdx doc can be associated with a set of software packages, file, or portion
of files (snippets) and contain information about the software.

Overview:
A spdx document is componsed of one or more of the following sections:
```
    Document Creation Info
    Package information
    File information
    Signing information
    Other licensing Information
    Relationship
    Annotations
```
Not all of the above sections are required in a spdx document though.

### Example
[opensbom-generator] can be used to generate an example:
```
$ cd spdx/example-project
$ ../spdx-sbom-generator
INFO[2023-04-06T09:08:31+02:00] Starting to generate SPDX ...                
INFO[2023-04-06T09:08:31+02:00] Running generator for Module Manager: `cargo` with output `bom-cargo.spdx` 
INFO[2023-04-06T09:08:31+02:00] Current Language Version cargo 1.69.0-nightly (39c13e67a 2023-02-12) 
INFO[2023-04-06T09:08:31+02:00] Global Setting File                          
INFO[2023-04-06T09:08:32+02:00] Command completed successful for below package managers 
INFO[2023-04-06T09:08:32+02:00] Plugin cargo generated output at bom-cargo.spdx 
```

With the generated [bom-cargo.spdx] we can then use `cosign attest` to add
an attestation to a container images. For example:

First we need to build an image and push it:
```console
$ make build-image
$ make push
$ make genkeys
$ make sign
```

Then we can create an attestation using the bom-carge-spdx:
```console
$ make attest
$ make attest
cosign attest -y --no-upload --key cosign.key --type spdx --predicate bom-cargo.spdx "ttl.sh/danbev-spdx-example-container:2h"
Enter password for private key: 
Using payload from: bom-cargo.spdx
{"payloadType":"application/vnd.in-toto+json","payload":"eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3NwZHguZGV2L0RvY3VtZW50Iiwic3ViamVjdCI6W3sibmFtZSI6InR0bC5zaC9kYW5iZXYtc3BkeC1leGFtcGxlLWNvbnRhaW5lciIsImRpZ2VzdCI6eyJzaGEyNTYiOiJlNWNhMGU1MDVmMWNjZWQyMGIxNjcxMTAxNWU5MTQ1YTk4Yjk0MDBiOWIyZGIyOGVjMjc0YjM4NjZkZTUyYWFlIn19XSwicHJlZGljYXRlIjoiU1BEWFZlcnNpb246IFNQRFgtMi4yXG5EYXRhTGljZW5zZTogQ0MwLTEuMFxuU1BEWElEOiBTUERYUmVmLURPQ1VNRU5UXG5Eb2N1bWVudE5hbWU6IGV4YW1wbGUtcHJvamVjdC0wLjEuMFxuRG9jdW1lbnROYW1lc3BhY2U6IGh0dHA6Ly9zcGR4Lm9yZy9zcGR4cGFja2FnZXMvZXhhbXBsZS1wcm9qZWN0LTAuMS4wLThlMzFhMmUxLTdlZjItNDk4Yi1iMjBmLTRlNzk4Y2Q5N2IyN1xuQ3JlYXRvcjogVG9vbDogc3BkeC1zYm9tLWdlbmVyYXRvci12MC4wLjE1XG5DcmVhdGVkOiAyMDIzLTA0LTA2VDA3OjA4OjMyWlxuXG5cbiMjIyMjIFBhY2thZ2UgcmVwcmVzZW50aW5nIHRoZSBleGFtcGxlLXByb2plY3RcblxuUGFja2FnZU5hbWU6IGV4YW1wbGUtcHJvamVjdFxuU1BEWElEOiBTUERYUmVmLVBhY2thZ2UtZXhhbXBsZS1wcm9qZWN0XG5QYWNrYWdlVmVyc2lvbjogMC4xLjBcblBhY2thZ2VTdXBwbGllcjogT3JnYW5pemF0aW9uOiBleGFtcGxlLXByb2plY3RcblBhY2thZ2VEb3dubG9hZExvY2F0aW9uOiBOT0FTU0VSVElPTlxuRmlsZXNBbmFseXplZDogZmFsc2VcblBhY2thZ2VDaGVja3N1bTogU0hBMTogN2JiYzQ4MDY2MzM5MTBmOTY5OWI4YWIzZWMxOTY2OGI3ZDg5ZTMwOVxuUGFja2FnZUhvbWVQYWdlOiBOT0FTU0VSVElPTlxuUGFja2FnZUxpY2Vuc2VDb25jbHVkZWQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZURlY2xhcmVkOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvcHlyaWdodFRleHQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZUNvbW1lbnRzOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvbW1lbnQ6IE5PQVNTRVJUSU9OXG5cblJlbGF0aW9uc2hpcDogU1BEWFJlZi1ET0NVTUVOVCBERVNDUklCRVMgU1BEWFJlZi1QYWNrYWdlLWV4YW1wbGUtcHJvamVjdCJ9","signatures":[{"keyid":"","sig":"MEUCIBZfFaJDRCxAWbOZEs50xRs+RBqmhNoGytiho+F3RMnMAiEA2UHzVBl9FKoW5B/kR2h2rXguKFjk1KBlllCLWjWEGI4="}]}
```

And we can inspect the above output using:
```console
$ echo '{"payloadType":"application/vnd.in-toto+json","payload":"eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3NwZHguZGV2L0RvY3VtZW50Iiwic3ViamVjdCI6W3sibmFtZSI6InR0bC5zaC9kYW5iZXYtc3BkeC1leGFtcGxlLWNvbnRhaW5lciIsImRpZ2VzdCI6eyJzaGEyNTYiOiJlNWNhMGU1MDVmMWNjZWQyMGIxNjcxMTAxNWU5MTQ1YTk4Yjk0MDBiOWIyZGIyOGVjMjc0YjM4NjZkZTUyYWFlIn19XSwicHJlZGljYXRlIjoiU1BEWFZlcnNpb246IFNQRFgtMi4yXG5EYXRhTGljZW5zZTogQ0MwLTEuMFxuU1BEWElEOiBTUERYUmVmLURPQ1VNRU5UXG5Eb2N1bWVudE5hbWU6IGV4YW1wbGUtcHJvamVjdC0wLjEuMFxuRG9jdW1lbnROYW1lc3BhY2U6IGh0dHA6Ly9zcGR4Lm9yZy9zcGR4cGFja2FnZXMvZXhhbXBsZS1wcm9qZWN0LTAuMS4wLThlMzFhMmUxLTdlZjItNDk4Yi1iMjBmLTRlNzk4Y2Q5N2IyN1xuQ3JlYXRvcjogVG9vbDogc3BkeC1zYm9tLWdlbmVyYXRvci12MC4wLjE1XG5DcmVhdGVkOiAyMDIzLTA0LTA2VDA3OjA4OjMyWlxuXG5cbiMjIyMjIFBhY2thZ2UgcmVwcmVzZW50aW5nIHRoZSBleGFtcGxlLXByb2plY3RcblxuUGFja2FnZU5hbWU6IGV4YW1wbGUtcHJvamVjdFxuU1BEWElEOiBTUERYUmVmLVBhY2thZ2UtZXhhbXBsZS1wcm9qZWN0XG5QYWNrYWdlVmVyc2lvbjogMC4xLjBcblBhY2thZ2VTdXBwbGllcjogT3JnYW5pemF0aW9uOiBleGFtcGxlLXByb2plY3RcblBhY2thZ2VEb3dubG9hZExvY2F0aW9uOiBOT0FTU0VSVElPTlxuRmlsZXNBbmFseXplZDogZmFsc2VcblBhY2thZ2VDaGVja3N1bTogU0hBMTogN2JiYzQ4MDY2MzM5MTBmOTY5OWI4YWIzZWMxOTY2OGI3ZDg5ZTMwOVxuUGFja2FnZUhvbWVQYWdlOiBOT0FTU0VSVElPTlxuUGFja2FnZUxpY2Vuc2VDb25jbHVkZWQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZURlY2xhcmVkOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvcHlyaWdodFRleHQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZUNvbW1lbnRzOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvbW1lbnQ6IE5PQVNTRVJUSU9OXG5cblJlbGF0aW9uc2hpcDogU1BEWFJlZi1ET0NVTUVOVCBERVNDUklCRVMgU1BEWFJlZi1QYWNrYWdlLWV4YW1wbGUtcHJvamVjdCJ9","signatures":[{"keyid":"","sig":"MEUCIBZfFaJDRCxAWbOZEs50xRs+RBqmhNoGytiho+F3RMnMAiEA2UHzVBl9FKoW5B/kR2h2rXguKFjk1KBlllCLWjWEGI4="}]}' | jq
{
  "payloadType": "application/vnd.in-toto+json",
  "payload": "eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3NwZHguZGV2L0RvY3VtZW50Iiwic3ViamVjdCI6W3sibmFtZSI6InR0bC5zaC9kYW5iZXYtc3BkeC1leGFtcGxlLWNvbnRhaW5lciIsImRpZ2VzdCI6eyJzaGEyNTYiOiJlNWNhMGU1MDVmMWNjZWQyMGIxNjcxMTAxNWU5MTQ1YTk4Yjk0MDBiOWIyZGIyOGVjMjc0YjM4NjZkZTUyYWFlIn19XSwicHJlZGljYXRlIjoiU1BEWFZlcnNpb246IFNQRFgtMi4yXG5EYXRhTGljZW5zZTogQ0MwLTEuMFxuU1BEWElEOiBTUERYUmVmLURPQ1VNRU5UXG5Eb2N1bWVudE5hbWU6IGV4YW1wbGUtcHJvamVjdC0wLjEuMFxuRG9jdW1lbnROYW1lc3BhY2U6IGh0dHA6Ly9zcGR4Lm9yZy9zcGR4cGFja2FnZXMvZXhhbXBsZS1wcm9qZWN0LTAuMS4wLThlMzFhMmUxLTdlZjItNDk4Yi1iMjBmLTRlNzk4Y2Q5N2IyN1xuQ3JlYXRvcjogVG9vbDogc3BkeC1zYm9tLWdlbmVyYXRvci12MC4wLjE1XG5DcmVhdGVkOiAyMDIzLTA0LTA2VDA3OjA4OjMyWlxuXG5cbiMjIyMjIFBhY2thZ2UgcmVwcmVzZW50aW5nIHRoZSBleGFtcGxlLXByb2plY3RcblxuUGFja2FnZU5hbWU6IGV4YW1wbGUtcHJvamVjdFxuU1BEWElEOiBTUERYUmVmLVBhY2thZ2UtZXhhbXBsZS1wcm9qZWN0XG5QYWNrYWdlVmVyc2lvbjogMC4xLjBcblBhY2thZ2VTdXBwbGllcjogT3JnYW5pemF0aW9uOiBleGFtcGxlLXByb2plY3RcblBhY2thZ2VEb3dubG9hZExvY2F0aW9uOiBOT0FTU0VSVElPTlxuRmlsZXNBbmFseXplZDogZmFsc2VcblBhY2thZ2VDaGVja3N1bTogU0hBMTogN2JiYzQ4MDY2MzM5MTBmOTY5OWI4YWIzZWMxOTY2OGI3ZDg5ZTMwOVxuUGFja2FnZUhvbWVQYWdlOiBOT0FTU0VSVElPTlxuUGFja2FnZUxpY2Vuc2VDb25jbHVkZWQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZURlY2xhcmVkOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvcHlyaWdodFRleHQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZUNvbW1lbnRzOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvbW1lbnQ6IE5PQVNTRVJUSU9OXG5cblJlbGF0aW9uc2hpcDogU1BEWFJlZi1ET0NVTUVOVCBERVNDUklCRVMgU1BEWFJlZi1QYWNrYWdlLWV4YW1wbGUtcHJvamVjdCJ9",
  "signatures": [
    {
      "keyid": "",
      "sig": "MEUCIBZfFaJDRCxAWbOZEs50xRs+RBqmhNoGytiho+F3RMnMAiEA2UHzVBl9FKoW5B/kR2h2rXguKFjk1KBlllCLWjWEGI4="
    }
  ]
}
```
And this might look familiar as this is an in-toto attestation envelope.
```console
$ echo '{"payloadType":"application/vnd.in-toto+json","payload":"eyJfdHlwZSI6Imh0dHBzOi8vaW4tdG90by5pby9TdGF0ZW1lbnQvdjAuMSIsInByZWRpY2F0ZVR5cGUiOiJodHRwczovL3NwZHguZGV2L0RvY3VtZW50Iiwic3ViamVjdCI6W3sibmFtZSI6InR0bC5zaC9kYW5iZXYtc3BkeC1leGFtcGxlLWNvbnRhaW5lciIsImRpZ2VzdCI6eyJzaGEyNTYiOiJlNWNhMGU1MDVmMWNjZWQyMGIxNjcxMTAxNWU5MTQ1YTk4Yjk0MDBiOWIyZGIyOGVjMjc0YjM4NjZkZTUyYWFlIn19XSwicHJlZGljYXRlIjoiU1BEWFZlcnNpb246IFNQRFgtMi4yXG5EYXRhTGljZW5zZTogQ0MwLTEuMFxuU1BEWElEOiBTUERYUmVmLURPQ1VNRU5UXG5Eb2N1bWVudE5hbWU6IGV4YW1wbGUtcHJvamVjdC0wLjEuMFxuRG9jdW1lbnROYW1lc3BhY2U6IGh0dHA6Ly9zcGR4Lm9yZy9zcGR4cGFja2FnZXMvZXhhbXBsZS1wcm9qZWN0LTAuMS4wLThlMzFhMmUxLTdlZjItNDk4Yi1iMjBmLTRlNzk4Y2Q5N2IyN1xuQ3JlYXRvcjogVG9vbDogc3BkeC1zYm9tLWdlbmVyYXRvci12MC4wLjE1XG5DcmVhdGVkOiAyMDIzLTA0LTA2VDA3OjA4OjMyWlxuXG5cbiMjIyMjIFBhY2thZ2UgcmVwcmVzZW50aW5nIHRoZSBleGFtcGxlLXByb2plY3RcblxuUGFja2FnZU5hbWU6IGV4YW1wbGUtcHJvamVjdFxuU1BEWElEOiBTUERYUmVmLVBhY2thZ2UtZXhhbXBsZS1wcm9qZWN0XG5QYWNrYWdlVmVyc2lvbjogMC4xLjBcblBhY2thZ2VTdXBwbGllcjogT3JnYW5pemF0aW9uOiBleGFtcGxlLXByb2plY3RcblBhY2thZ2VEb3dubG9hZExvY2F0aW9uOiBOT0FTU0VSVElPTlxuRmlsZXNBbmFseXplZDogZmFsc2VcblBhY2thZ2VDaGVja3N1bTogU0hBMTogN2JiYzQ4MDY2MzM5MTBmOTY5OWI4YWIzZWMxOTY2OGI3ZDg5ZTMwOVxuUGFja2FnZUhvbWVQYWdlOiBOT0FTU0VSVElPTlxuUGFja2FnZUxpY2Vuc2VDb25jbHVkZWQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZURlY2xhcmVkOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvcHlyaWdodFRleHQ6IE5PQVNTRVJUSU9OXG5QYWNrYWdlTGljZW5zZUNvbW1lbnRzOiBOT0FTU0VSVElPTlxuUGFja2FnZUNvbW1lbnQ6IE5PQVNTRVJUSU9OXG5cblJlbGF0aW9uc2hpcDogU1BEWFJlZi1ET0NVTUVOVCBERVNDUklCRVMgU1BEWFJlZi1QYWNrYWdlLWV4YW1wbGUtcHJvamVjdCJ9","signatures":[{"keyid":"","sig":"MEUCIBZfFaJDRCxAWbOZEs50xRs+RBqmhNoGytiho+F3RMnMAiEA2UHzVBl9FKoW5B/kR2h2rXguKFjk1KBlllCLWjWEGI4="}]}' | jq -r '.payload' | base64 -d |jq
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "predicateType": "https://spdx.dev/Document",
  "subject": [
    {
      "name": "ttl.sh/danbev-spdx-example-container",
      "digest": {
        "sha256": "e5ca0e505f1cced20b16711015e9145a98b9400b9b2db28ec274b3866de52aae"
      }
    }
  ],
  "predicate": "SPDXVersion: SPDX-2.2\nDataLicense: CC0-1.0\nSPDXID: SPDXRef-DOCUMENT\nDocumentName: example-project-0.1.0\nDocumentNamespace: http://spdx.org/spdxpackages/example-project-0.1.0-8e31a2e1-7ef2-498b-b20f-4e798cd97b27\nCreator: Tool: spdx-sbom-generator-v0.0.15\nCreated: 2023-04-06T07:08:32Z\n\n\n##### Package representing the example-project\n\nPackageName: example-project\nSPDXID: SPDXRef-Package-example-project\nPackageVersion: 0.1.0\nPackageSupplier: Organization: example-project\nPackageDownloadLocation: NOASSERTION\nFilesAnalyzed: false\nPackageChecksum: SHA1: 7bbc4806633910f9699b8ab3ec19668b7d89e309\nPackageHomePage: NOASSERTION\nPackageLicenseConcluded: NOASSERTION\nPackageLicenseDeclared: NOASSERTION\nPackageCopyrightText: NOASSERTION\nPackageLicenseComments: NOASSERTION\nPackageComment: NOASSERTION\n\nRelationship: SPDXRef-DOCUMENT DESCRIBES SPDXRef-Package-example-project"
}
```
Notice that the format is not json which I'm used to seeing. JSON format is
also possible and can be generated using `make spdx-json` and the
`make attest-json`.

One thing I noticed when using the json format is that there are additional
`\n\t` in the json predicate field which I was not expecting:
```console
$ cat spdx-2.2-statement.json | jq -r '.payload' | base64 -d  | jq
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "predicateType": "https://spdx.dev/Document",
  "subject": [
    {
      "name": "ttl.sh/danbev-spdx-example-container",
      "digest": {
        "sha256": "e5ca0e505f1cced20b16711015e9145a98b9400b9b2db28ec274b3866de52aae"
      }
    }
  ],
  "predicate": "{\n\t\"spdxVersion\": \"SPDX-2.2\",\n\t\"dataLicense\": \"CC0-1.0\",\n\t\"SPDXID\": \"SPDXRef-DOCUMENT\",\n\t\"name\": \"example-project-0.1.0\",\n\t\"documentNamespace\": \"http://spdx.org/spdxpackages/example-project-0.1.0-08438f50-4d7d-47d9-aad6-70667e16fa79\",\n\t\"creationInfo\": {\n\t\t\"created\": \"2023-04-06T07:48:47Z\",\n\t\t\"creators\": [\n\t\t\t\"Tool: spdx-sbom-generator-v0.0.15\"\n\t\t]\n\t},\n\t\"packages\": [\n\t\t{\n\t\t\t\"name\": \"example-project\",\n\t\t\t\"SPDXID\": \"SPDXRef-Package-example-project\",\n\t\t\t\"versionInfo\": \"0.1.0\",\n\t\t\t\"supplier\": \"Organization: example-project\",\n\t\t\t\"downloadLocation\": \"NOASSERTION\",\n\t\t\t\"filesAnalyzed\": false,\n\t\t\t\"checksums\": [\n\t\t\t\t{\n\t\t\t\t\t\"algorithm\": \"SHA1\",\n\t\t\t\t\t\"checksumValue\": \"7bbc4806633910f9699b8ab3ec19668b7d89e309\"\n\t\t\t\t}\n\t\t\t],\n\t\t\t\"homepage\": \"NOASSERTION\",\n\t\t\t\"licenseConcluded\": \"NOASSERTION\",\n\t\t\t\"licenseDeclared\": \"NOASSERTION\",\n\t\t\t\"copyrightText\": \"NOASSERTION\",\n\t\t\t\"licenseComments\": \"NOASSERTION\",\n\t\t\t\"comment\": \"NOASSERTION\"\n\t\t}\n\t],\n\t\"relationships\": [\n\t\t{\n\t\t\t\"spdxElementId\": \"SPDXRef-DOCUMENT\",\n\t\t\t\"relatedSpdxElement\": \"SPDXRef-Package-example-project\",\n\t\t\t\"relationshipType\": \"DESCRIBES\"\n\t\t}\n\t]\n}"
}
```
It looks like the json was created using "pretty printing" or whatever it would
be called:
```console
$ cat spdx-2.2-statement.json | jq -r '.payload' | base64 -d  | jq -r '.predicate'
{
	"spdxVersion": "SPDX-2.2",
	"dataLicense": "CC0-1.0",
	"SPDXID": "SPDXRef-DOCUMENT",
	"name": "example-project-0.1.0",
	"documentNamespace": "http://spdx.org/spdxpackages/example-project-0.1.0-08438f50-4d7d-47d9-aad6-70667e16fa79",
	"creationInfo": {
		"created": "2023-04-06T07:48:47Z",
		"creators": [
			"Tool: spdx-sbom-generator-v0.0.15"
		]
	},
	"packages": [
		{
			"name": "example-project",
			"SPDXID": "SPDXRef-Package-example-project",
			"versionInfo": "0.1.0",
			"supplier": "Organization: example-project",
			"downloadLocation": "NOASSERTION",
			"filesAnalyzed": false,
			"checksums": [
				{
					"algorithm": "SHA1",
					"checksumValue": "7bbc4806633910f9699b8ab3ec19668b7d89e309"
				}
			],
			"homepage": "NOASSERTION",
			"licenseConcluded": "NOASSERTION",
			"licenseDeclared": "NOASSERTION",
			"copyrightText": "NOASSERTION",
			"licenseComments": "NOASSERTION",
			"comment": "NOASSERTION"
		}
	],
	"relationships": [
		{
			"spdxElementId": "SPDXRef-DOCUMENT",
			"relatedSpdxElement": "SPDXRef-Package-example-project",
			"relationshipType": "DESCRIBES"
		}
	]
}
```

[opensbom-generator]: https://github.com/opensbom-generator/spdx-sbom-generator/releases
[bom-cargo.spdx]: ../spdx/example-project/bom-cargo.spdx
