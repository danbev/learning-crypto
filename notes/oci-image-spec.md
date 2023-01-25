## Open Container Initiative (OCI) Image Specification
The OCI image [specification] deals with how to package and store a container
image. 

My main motivation for learning about this format is that it is useful to
understand when using [ORAS](./oras.md), and also projects like in-toto and
sigstore use these concepts internally.

### Content addresses
Think about a file path on a file system. We can use /home/danbev/report.txt
to refer to this file. If the file is updated the path does not change but the
contents does. This is pretty obvious, but when we think about images, which are
immutable, having paths like this does not really make sense. Instead we
identify content using a digest (hash) of the content. When the content changes
so does the digest. So we will see these hashes/digests instead of file
names/paths. Content is specified using
[Content Descriptors](#content-descriptors).

### Content Descriptors
These are unique pointers to a particular content of an image.
These include the `type` of the content, `digest` which identifies the content
, and it has a `size`.

* The type is specified using the `mediaType` property.
* The digest is specified using the digest property.
* The size is specified using the size property.

Example of a decriptor:
```
    {
      "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
      "digest": "sha256:bf75762436b060837307bb6b9a016fe728b10f33040c95516c621475280efc32",
      "size": 8004381
    }
```

#### Digest
The digest is part of the descriptor are specified using the following format:
```
algorithm ':' encoded hash
```
Where algorithm can be `sha256`, or `sha512`. There can be others but these are
the ones that are registered as I've understood it.

So the digest tells us what algorithm was used to generate the hash of the
content that this digest is pointing to. So we can use the same algorithm and
pass in the content/data and then check that the hashes are the same.


### Artifact Manifest
The image specification consists of a manifest, an optional image index, a set
of filesystem layers, and a configuration. We will focus mainly on the
manifest part in this section.

#### Image Manifest
This is the file that describes the components that make up the container
image.

The [manifest](https://github.com/opencontainers/image-spec/blob/main/manifest.md)
has the following properties:
* schemaVersion which is always 2
* mediaType
* config is a descriptor pointing to a oci container configuration file.
* layers is an array of descriptors
* subject is a descriptor of another manifest and used to specify a relationship
to that manifest. This is used in the [referrers api](https://github.com/opencontainers/distribution-spec/blob/main/spec.md#listing-referrers)
* annotations can contain any additional metadata in key-value pairs

For example:
```console
{
  "schemaVersion": 2,
  "mediaType": "application/vnd.oci.image.manifest.v1+json",
  "config": {
    "mediaType": "application/vnd.oci.image.config.v1+json",
    "size": 7023,
    "digest": "sha256:b5b2b2c507a0944348e0303114d8d93aaaa081732b86451d9bce1f432a537bc7"
  },
  "layers": [
    {
      "mediaType": "application/vnd.oci.image.layer.v1.tar+gzip",
      "size": 32654,
      "digest": "sha256:9834876dcfb05cb167a5c24953eba58c4ac89b1adf57f28f2f9d09af107ee8f0"
    },
    {
      "mediaType": "application/vnd.oci.image.layer.v1.tar+gzip",
      "size": 16724,
      "digest": "sha256:3c3a4604a545cdc127456d94e421cd355bca5b528f4a9c1905b15da2eb4a4c6b"
    },
    {
      "mediaType": "application/vnd.oci.image.layer.v1.tar+gzip",
      "size": 73109,
      "digest": "sha256:ec4b8955958665577945c89419d1af06b5f7636b4ac3da7f12184802ad867736"
    }
  ],
  "subject": {
    "mediaType": "application/vnd.oci.image.manifest.v1+json",
    "size": 7682,
    "digest": "sha256:5b0bcabd1ed22e9fb1310cf6c2dec7cdef19f0ad69efa1f392e94a4333501270"
  },
  "annotations": {
    "com.example.key1": "value1",
    "com.example.key2": "value2"
  }
}
```

So at the top level of the object we have a mediaType and a version of this
manifest, followed by a config object, and the a layers array.

OCI uses `application/vnd.oci.image.config.v1+json` for the config, and
`application/vnd.oci.image.layer.v1.tar+gzip` for the layers.
The config type is reserved.

The contents of the config and layers is up to the creator and can be pretty
much anything. You can use the mediatype to specify what the data is.

The config is used by a container runtime, as are the layers which are used to
create a bundle.

### Artifacts manifest
Simliar to image manifest described in the previous section but instead of
describing the components that are destined to be run in by a container runtime
the components that an artifact manifest describe are not meant to be run by
a container image.

The spec tries to make the registry format more generic so that any types of
files can be stored and distributed.

The [manifest](https://github.com/opencontainers/image-spec/blob/main/artifact.md)
has the following properties:
* mediaType
* artifactType contains the mediaType of the referenced artifact.
* blobs is an array of descriptors (similar to layers)
* subject is a descriptor of another manifest and used to specify a relationship
to that manifest. This is used in the [referrers api](https://github.com/opencontainers/distribution-spec/blob/main/spec.md#listing-referrers)
* annotations can contain any additional metadata in key-value pairs

Notice that we don't have a config property as these types are not destined to
be run by a container runtime.

Example:
```
{
  "mediaType": "application/vnd.oci.artifact.manifest.v1+json",
  "artifactType": "application/vnd.example.sbom.v1",
  "blobs": [
    {
      "mediaType": "application/gzip",
      "size": 123,
      "digest": "sha256:87923725d74f4bfb94c9e86d64170f7521aad8221a5de834851470ca142da630"
    }
  ],
  "subject": {
    "mediaType": "application/vnd.oci.image.manifest.v1+json",
    "size": 1234,
    "digest": "sha256:cc06a2839488b8bd2a2b99dcdc03d5cfd818eed72ad08ef3cc197aac64c0d0a0"
  },
  "annotations": {
    "org.opencontainers.artifact.created": "2022-01-01T14:42:55Z",
    "org.example.sbom.format": "json"
  }
}
```

[specification]: https://github.com/opencontainers/image-spec/blob/main/spec.md
