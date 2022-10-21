## Software Bill Of Materials (SBOM)
This is simlar to a BOM from industrial manufacturing (I've read) where there
the is a BOM, which is a document of all the components that go into a product.
It also contains who supplied the components, perhaps a chain of suppliers for
some parts (which is simlar to a thirdparty dependency in software), how the
manufacturing processes was performed (software build steps), perhaps
information about the factory environment if that is important like temperature,
how the end product was packaged, etc.

Simlar information can be collected, documented, and signed for a software
project. The components that go into the product are source code, thirdparty
deps, the factory is perhaps a build server where we document the environment
that performed the build.

To be able to generate this information and to consume it we need standards so
that these can be consumed by anyone. This page contains notes about the most
common SBOM standards that are out there today.

### CycloneDX specification
[Specification](https://github.com/CycloneDX/specification)

Originated from the OWASP community.

### Software Product Data Exchange (SPDX)
[Specification](https://spdx.dev/specifications/)

Is an ISO standard and covers components, licences, and copyrights associated
with software projects.
Is under the Linux Foundation.


