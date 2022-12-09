## Open Container Initiative
A number of organizations came together in 2015 to standardize container
runtimes and image formats.

There is a runtime specification which addresses how images are run. Then there
is an image specification that deals with how to package and store a container
image. And there is also a distribution specification that deals with how to
transfer a container image.

## Content addresses
Think about a file path on a file system. We can use /home/danbev/report.txt
to refer to this file. If the file is updated the path does not change but the
contents does. This is pretty obvious, but when we think about images, which are
immutable, having paths like this does not really make sense. Instead we
identify content using a digest (hash the content) of the content. When the
content changes so does the digest. So we will see these hashes instead of
file names/paths.

### OCI Registries
Are mostly used to store images but they are able to store other things, for
example OPA policies. These are called OCI Artifacts.

### OCI Image Format Specification
The format consists of a manifest, an optional image index, a set of filesystem
layers, and a configuration.

So lets say we have rust program what we want to package into an image and
allow it to be executable. The program itself would be compiled and all the
required libs (if dynamically linked) will need to be in the layers.
There would be a config which tells how the program its to be run, which might
include command line options or whatever.
The image manifest specifies the cpu arch for the layers and the config.

#### Image index
This is a set of image manifests. Recall from above that these manifests specify
cpu archs and it might be that multiple archs are supported.

#### Image layout
This is a file system layout for the contents of an image.

#### OCI Content Descriptors
These are unique pointers to a particular content of an image.
These include the type of the content, digest which identifies the content
, and it has a size.
The type is specified using the `mediaType` property.
The digest is specified using the digest property.
The size is specified using the size property.

#### Digest
The digest is specified in an as:
```
algorithm ':' encoded hash
```
Where algorithm can be `sha256`, or `sha512`.
So the digest tells us what algorithm was used to generate the hash of the
content that this digest is pointing to. So we can use the same algorithm and
pass in the content/data and then check that the hashes are the same.

Example of a decriptor:
```
    {
      "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
      "digest": "sha256:bf75762436b060837307bb6b9a016fe728b10f33040c95516c621475280efc32",
      "size": 8004381
    }
```

#### Image Manifest
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

Note that the mediaType's in this case are docker media types.
So at the top level of the object we have a mediaType and a version of this
manifest, followed by a config object, and the a layers array.

OCI used application/vnd.oci.image.config.v1+json for the config, and
application/vnd.oci.image.layer.v1.tar+gzip for the layers.
The config type is reserved.

The contents of the config and layers is up to the creator and can be pretty
much anything. You can use the mediatype to specify what the data is.

### OCI Artifacts
Tries to make the registry format more generic so that any types of files can
be stored and distributed.
 
