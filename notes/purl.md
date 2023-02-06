## Package Url (purl)
This is a [Specification](https://github.com/package-url/purl-spec) that
specifies a package-url in a software language, package-manager, neutral way.


## Format
```
scheme:type/namespace/name@version?qualifiers#subpath
```

* scheme     should be `pkg`. Required.
* type       the type/protocol of the package. Examples: maven, npm, rust, gem etc.
Required
* namespace  some namespace specific to this purl, could be a github user/org.
Optional.
* name       the name of the package. Required.
* version    the version. Required.
* qualifiers OS, arch, distro. Optional.
* subpath    subpath relative to the pacage root. Optional.

There is a [rust](https://github.com/package-url/packageurl.rs) implementation.
