## The Update Framework (TUF)

This document is an attempt to get some hands-on experience with TUF. The idea
is that having examples will help understand TUF as I've found just reading has
not made it "stick" with me.

As the name "The Update Framework" implies this is a framework for update
systems, and doing so in a secure way. What is getting updated is could be
anything, it could be software packages, source files, certificates, public
keys, etc. And by following this "framework", updates can be performed in a
secure way.

Producers want the thing they produce available to consumers, and they want to
make sure that consumers are getting updates for these things. I'm using
"things" just to make it clear that this does not have to be software packages
which might be what first comes to mind.

Lets take a look what this might look like without TUF:
```
  +-----------+     +---------------------+                      +----------+
  | Producer  |     | Distribution server |                      | Consumer |
  |-----------+     |---------------------+                      |----------|
  | thing_v1  | --> | thing_v1            | -------------------> | thing_v1 |
  +-----------+     +---------------------+                      +----------+
```
So we have a producer that has something that it makes available to consumser's
via a distribution server. The consumer uses this thing by downloading it from
the distribution server in some manner. If the distribution server just allows
the consumer to poll and download the thing/artifact, then it will be up to the
consumer code to decide when it should poll to check for updates and update if
needed. 


For example, lets say that a man in the middle (MITM) attack is put in place
and the consumer is no longer talking to the distribution server but instead
to server controlled by an attacker which provides the consumer with a malicious
artifacts:
```
  +-----------+     +---------------------+     +----------+     +----------+
  | Producer  |     | Distribution server |     | MITM     |     | Consumer |
  |-----------+     |---------------------+     |          |     |----------|
  | thing_v1  | --> | thing_v1            |     | evil_v1  | --> | thing_v1 |
  +-----------+     +---------------------+     +----------+     +----------+
```

An attacker may also target the distribution server itself and modify the
artifacts that the consumers download:
```
  +-----------+     +---------------------+                      +----------+
  | Producer  |     | Distribution server |                      | Consumer |
  |-----------+     |---------------------+                      |----------|
  | thing_v1  | --> | thing_v1 (evil_v1)  | ------------------>  | thing_v1 |
  +-----------+     +---------------------+                      +----------+
```
This type of attack is refered to as `Arbitary software installation`.

Another attack against the distribution server is where the attacker prevents
the consumer from getting updates, and instead provides the consumer with an
older version which might contain a known vulnerabilty.

This would be the current state where the correct/latest version is being used:
```
  +-----------+     +---------------------+                      +----------+
  | Producer  |     | Distribution server |                      | Consumer |
  |-----------+     |---------------------+                      |----------|
  | thing_v3  | --> | thing_v3            | ------------------>  | thing_v3 |
  +-----------+     +---------------------+                      +----------+
```
The attacker then changes the version on the distribution server to an older
version, causing the consumer to downgrade, or rollback to that version:
```
  +-----------+     +---------------------+                      +----------+
  | Producer  |     | Distribution server |                      | Consumer |
  |-----------+     |---------------------+                      |----------|
  | thing_v3  | --> | thing_v2 (evil_v2)  | ------------------>  | thing_v2 |
  +-----------+     +---------------------+                      +----------+
```
This type of attack is refered to as `Rollback attack`.

There are other [attacks](https://theupdateframework.io/security/) but these
I found helped me better understand the metadata that is provided by TUF.

A simplified overview of this can be seen below and I'm going into more
details later in the document.

What I'd like to convey with this is that the producer will update the TUF
repository by creating `metadata` about the artifact(s) that are going to be
made available. This metadata is signed by one or more keys.
The motivation for signing is that we want to prevent the situation above where
an attacker is able to replace an artifact with an older version, or a modified
version.

Having the metadata signed for each version means that it would not be
possible for an attacker to do this as the TUF client framework will verify
signatures.

TUF overview:
```
  +-----------+         +---------------------+               +------------------+
  | Producer  |         | TUF Repository      | <-----------  | TUF Consumer     |
  |-----------+ update  |---------------------+               |------------------|
  | thing_v1  | ------> | Metadata (signed)   | ------------> | Metadata (signed)|
  +-----------+         +---------------------+               +------------------+
                                                ------------> | thing_v1         |
                                                              +------------------+
```
Notice that in addition to the metadata that exists on the TUF repository there
is also metadata on the TUF consumer/client side. This metadata is downloaded
and frequently resigned, and it has an short expiration date. This is how the
TUF framework enforces that updates actually take place. Because the TUF client
framework checks the expiration and the signature of the metadata file, it
can detect if the expiration date has passed. If there has not been any updates
and the expiration date has passed, perhaps a certain number of times, it can
take action to notify the client side software about this situation. This is how
TUF can enforce that updates actually get applied to the consumer. 

This metadata is also signed as we don't want an attacker to be able to serve
the client with a metadata file they crafted themselves, which might have
enabled the attacker to trick the consumer into thinking that there are no
updates and force the consumer to be stuck on an older version.

Hopefully this has provided an overview and some idea about the metadata and
the signing in TUF. Later we will see a concrete example of the metadata files
to get "feel" for what they look like.

So, we have mentioned metadata files and signing. The following is an attempt
to visualize where the metadata files exist:
```
  +-----------+         +---------------------+               +---------------+
  | Producer  |         | TUF Repository      | <-----------  | TUF Consumer  |
  |-----------+ update  |---------------------+               |---------------|
  | Keys      | ------> | Metadata            | ------------> | Metadata      |
  +-----------+         | 1.root.json         |               | snapshot.json |  
  | Metadata  |         | 1.targets.json      |               |---------------|
  | root.json |         | 1.snapshot.json     |               | thing_v1      |
  +-----------+         | timestamp.json      |               +---------------+
  | thing_v1  |         +---------------------+                                      
  +-----------+         | thing_v1            |
                        +---------------------+                                      
```
The names of the metadata files are named after four top level roles in TUF, 
the `Root`, `Target`, `Timestamp`, and `Snapshot` roles.

#### Root Metadata
Instead of having a single root key, there will often be multiple root keys
which are stored in different offline locations. And these keys are used to sign
keys that can be used for other purposes.

So we have a number of keys that are `offline`, meaning that they are not used
for signing things, like software artifacts or things like that. Instead these
keys are often stored in separate locations and then used together to sign other
keys, which are then used for `online` tasks, like signing metadata describing
artifacts.

The non-root keys can then be re-signed/revoked/rotated if/when needed.

Each role has metadata associated with it, and the specification defines a
canonical json format for these. So there would be a root.json, a targets.json,
a, timestamps.json, and a snapshot.json.

So what do these file look like?  

Lets try this out by using a tool called
[tuftool](https://github.com/awslabs/tough/tree/develop/tuftool):
```console
$ cargo install --force tuftool
```
An example directory can be found in [tuf](../tuf) which will contain all the
files that are generated below.

#### Root metadata

Next we initiate a new `root.json` file using the following command:
```console
$ tuftool root init root/root.json
```
This command will generate a file named [root.json](../tuf/root/root.json):
```console
$ cat root/root.json 
{
  "signed": {
    "_type": "root",
    "spec_version": "1.0.0",
    "consistent_snapshot": true,
    "version": 1,
    "expires": "2023-01-17T07:48:23Z",
    "keys": {},
    "roles": {
      "timestamp": {
        "keyids": [],
        "threshold": 1507
      },
      "root": {
        "keyids": [],
        "threshold": 1507
      },
      "snapshot": {
        "keyids": [],
        "threshold": 1507
      },
      "targets": {
        "keyids": [],
        "threshold": 1507
      }
    }
  },
  "signatures": []
}
```
These are just the default values, and we can see that there are no root keys,
that field is just an empty object, and notice that the roles are mainly empty
apart from the `threshold` values which is 1507. The threshold value specifies
the minimum number of keys required to sign that roles metadata. 1507 is a large
number of keys and we can change this to just requiring one key:
```console
$ tuftool root set-threshold root/root.json snapshot 1
$ tuftool root set-threshold root/root.json root 1
$ tuftool root set-threshold root/root.json timestamp 1
$ tuftool root set-threshold root/root.json targets 1
```
And we can see that `root/root.json` has been updated:
```
$ cat root/root.json 
{
  "signed": {
    "_type": "root",
    "spec_version": "1.0.0",
    "consistent_snapshot": true,
    "version": 1,
    "expires": "2023-02-27T14:05:04Z",
    "keys": {},
    "roles": {
      "root": {
        "keyids": [],
        "threshold": 1
      },
      "targets": {
        "keyids": [],
        "threshold": 1
      },
      "snapshot": {
        "keyids": [],
        "threshold": 1
      },
      "timestamp": {
        "keyids": [],
        "threshold": 1
      }
    }
  },
  "signatures": []
}
```
We can also set the expire time for the root using the following command:
```console
$ tuftool root expire root/root.json 'in 6 weeks'
```
Now, to sign anthing we will need a private key to create the signatures and
a matching public key to be used to verify signatures.

We can generate a root key and one can be generated using
`tuftool root gen-rsa-key`:
```console
$ tuftool root gen-rsa-key root/root.json ./keys/root.pem --role root
6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f
```
This will generate [keys/root.pem](../tuf/keys/root.pem) which is a private
key in pkcs8 format. The hex value printed on above is the `key_id` which will
be used later to reference this key in metadata files.

Now, if we again inspect `root.json` we find that a key has been added:
```console
$ cat root/root.json
{
  "signed": {
    "_type": "root",
    "spec_version": "1.0.0",
    "consistent_snapshot": true,
    "version": 1,
    "expires": "2023-02-28T08:30:48Z",
    "keys": {
      "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f": {
        "keytype": "rsa",
        "keyval": {
          "public": "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA5ZiWzak3CBJkRrCfw5GO\nSUtYjIK2jLozyaZ44FePW/KYEhM8LyHcNz9lwx45tZ8gId4AsxGBj9fhsOgjpN7l\nMPXpaKsV/5f37HzQLCrbldz3ei9LkMWG5La4Cwil0qPDpTxfzI7IWDKk6l4/epgi\nOrAJDaQ/mKhH5OZ485JYuDIE7a0jplU/GvsNeCdZVMEQ8dko/CA4Di8lPkDRRdSw\naC/8g3K6mF+87ADdGOmZ+LFodLEPvqIVljece2JlX2z44Io3N7Y5FH63Az4H3MFL\nDPZJH5lFs7Lb/fHx25rWSE2/GHcUUTs4oScPp2X0hAnblOsmCFSCjf8Kb0R7dLUb\nnQIDAQAB\n-----END PUBLIC KEY-----"
        },
        "scheme": "rsassa-pss-sha256"
      }
    },
    "roles": {
      "timestamp": {
        "keyids": [],
        "threshold": 1
      },
      "targets": {
        "keyids": [],
        "threshold": 1
      },
      "root": {
        "keyids": [
          "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f"
        ],
        "threshold": 1
      },
      "snapshot": {
        "keyids": [],
        "threshold": 1
      }
    }
  },
  "signatures": []
}
```
Notice that the `keys` object has a field which is named after the `key_id` and
that the `root` role has this `key_id` in its `keyids` array. And notice also
that `keys` only includes the public key.

The Target role is a role that signs metadata files that describe the
project artifacts, like software packages, source code, or whatever artifacts
that are to be consumed by consumers.

So lets add a target key, and we will use the same private key as before:
```console
$ tuftool root add-key root/root.json ./keys/root.pem --role targets
6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f
```
This will update the `targets` role in `root.json`:
```console
$ jq '.signed.roles.targets' < root/root.json
{
  "keyids": [
    "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f"
  ],
  "threshold": 1
}
```

The Snapshot roles signs a metadata file which contains information about the 
latest version of the targets metadata. This is used to identify which versions
of a target are in a repository at a certain time. This is used to know if there
is an update available (remember its call The Update Framework).

So, lets add a key to the snapshot role:
```console
$ tuftool root add-key root/root.json ./keys/root.pem --role snapshot
```
And we can see that following change to `root.json`:
```console
$ jq '.signed.roles.snapshot' < root/root.json
{
  "keyids": [
    "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f"
  ],
  "threshold": 1
}
```
Finally, we have the timestamp role sed tell if there is an update. 
```console
$ tuftool root add-key root/root.json ./keys/root.pem --role timestamp
```
And we can see that following change to `root.json`:
```console
$ jq '.signed.roles.timestamp' < root/root.json
{
  "keyids": [
    "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f"
  ],
  "threshold": 1
}
```
So we have configured which keys to be used for each of the roles but we have
not signed this metadata file. We need to sign it to prevent tampering of it
as it will be sent to the TUF repository, usually on server I think but for
the example everything will be on the local file system.

We can now sign root.json using the following command:
```console
$ tuftool root sign ./root/root.json -k ./keys/root.pem
```
And we can check `root.json` that the signatures field has been updated with
a keyid and a signature.
```console
 jq '.signatures' < root/root.json
[
  {
    "keyid": "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f",
    "sig": "8e7c7ca88e242ff360af30ba83e0ccfd9de2a9dee774166abe508ad2757620d6439bc7a5163c55867e069812a21ba31b7097d9ded3590f03f8bdc7106755a9ae840efbfe9b6c7d69e047230c59f3bd682e83f0b5b9c271d6db60943f7fa57d565790de58687560b50951a363725471c3a8f64c3980385eb214876bb1fe87d4aefc5cdd557bd022ddd794a52368f8502c1944185c75827ca97bba8fd5cdd5bb41b7ad76f0105072fbee980d3dbbf9889ec223ea1399228560fd747bc03a378d3ba93990560b000d02a59aab04844ec70662f8baaee33f8591f5bbe3126fb057f9b3055d498005220d1715c92166506995e89f2e8e62d1032452d51ba6579eb0e2"
  }
]
```

With the `root.json` and the private key we can generate a tuf repository
using the following command:
```console
$ tuftool create \
  --root root/root.json \
  --key keys/root.pem \
  --add-targets artifacts \
  --targets-expires 'in 3 weeks' \
  --targets-version 1 \
  --snapshot-expires 'in 3 weeks' \
  --snapshot-version 1 \
  --timestamp-expires 'in 1 week' \
  --timestamp-version 1 \
  --outdir repo
```
That command will create a directory named [repo](../tuf/repo) which contains
two directories, `metadata` and `targets`.

Let start by `targets` directory:
```console
$ ls -l repo/targets/01ab0faaf41a4543df1fa218b8e9f283d07536339cf11d2afae9d116a257700c.artifact_1.txt 
lrwxrwxrwx. 1 danielbevenius danielbevenius 79 Jan 17 12:50 repo/targets/01ab0faaf41a4543df1fa218b8e9f283d07536339cf11d2afae9d116a257700c.artifact_1.txt -> artifacts/artifact_1.txt
```
Notice that the name of this link is the sha256sum of the contents of
artifact_1.txt file:
```console
$ sha256sum  artifacts/artifact_1.txt 
01ab0faaf41a4543df1fa218b8e9f283d07536339cf11d2afae9d116a257700c  artifacts/artifact_1.txt
```

Now, lets take a look at the `metadata` directory.
```console
$ ls repo/metadata/
1.root.json  1.snapshot.json  1.targets.json  timestamp.json
```
The number prefix is the version, to 1.targets.json is for version 1 for
example.

Lets start by looking at `targets.json`:
```json
{
  "signed": {
    "_type": "targets",
    "spec_version": "1.0.0",
    "version": 1,
    "expires": "2023-02-07T11:50:00.608598188Z",
    "targets": {
      "artifact_1.txt": {
        "length": 24,
        "hashes": {
          "sha256": "01ab0faaf41a4543df1fa218b8e9f283d07536339cf11d2afae9d116a257700c"
        }
      }
    },
    "delegations": {
      "keys": {},
      "roles": []
    }
  },
  "signatures": [
    {
      "keyid": "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f",
      "sig": "48b9810f275e16acb2d093c5487da4107f2312e0c9e084f6974aa836661a0e87d341be37fe84a4afd6ba82e8e54301e01a7431a0e69d3bef95ce3e34d90badff3c4a19ed7a6cea2a4ec69c6dc7392fde1f20b1246f3113ace85a223bfc54203a9254e82c8cd9686b8b973bfc41cdda657ff707a41c3db125b61dfc41c8937896f7fcc0ea17429a934b9c0fee912ca4df4a3b1dac6811968aa34bbf2d3327bbeab9cad1dadc1f8134c0add4267bf8ff285c066d24ea39b24d9bca197bf9762025133205612d41b167ee1232adf8c122320d77b70b936817ddf2cc93732228f772078b663f3fc896ec8873873414ba44fd3e28772589f69af06ee3e1297e0b2b37"
    }
  ]
}
```
We have information about the targets, in this case on a single file named
artifact_1.txt, and this is the file that a client wants to consume. This
metadata file is signed and the signature is in the `sig` field of the
`signatures` array.
Again, this needs to be signed to prevent an attacker from modifying the targets
and modifying the `length`, and `sha256` fields which would otherwise allow them
to replace the target artifact with a potentially malicious version.

Next, lets take a look at the `snapshots.json` metadata file:
```console
$ cat repo/metadata/1.snapshot.json 
{
  "signed": {
    "_type": "snapshot",
    "spec_version": "1.0.0",
    "version": 1,
    "expires": "2023-02-07T11:50:00.608597347Z",
    "meta": {
      "targets.json": {
        "length": 1048,
        "hashes": {
          "sha256": "896781ff1260ed4ad5b05a004b034279219dc92b64068a2cc376604e8a6821c9"
        },
        "version": 1
      }
    }
  },
  "signatures": [
    {
      "keyid": "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f",
      "sig": "5a8d9597329183c52547591d1abc8e36c1535d81c0e51ed51d95d2ddf1ec2076f2412ba8e631f039c7bf9e5a14cdd44eb7a5c7dae5dcc84e6aa2ebd51049ee791cf3c3dc486af26731fc06ba39e449ef85b102247c4254cb48784e4a95b54943df9e668470a6def79c7c3d532a68e93d18f1d59f1636455dddec0b5960afeb5a9ac38c38c6891e6f819f22aed7996a7f9964d655d634a940e1234f2015caa8f4f710570443bc0bc3ec04117c3dc97c8d564f42489cc499593f6232b7f5062646644aecafaf50dc9a4005a000f6720b0b9c455e5b92d7a1bcfb96f14a6a9da162e9b091497b0eb24283a837ba1d15ff67f12d104b1c5e1d83c36ae49400bb326e"
    }
  ]
}
```
So looking at the above `meta` field, we can see that there is a `targets.json`
"meta path". If we search for this file we won't be able to find it. The actual
file is prefixed with the version from the `version` field. So the file in
question is `repo/metadata/1.targets.json`, and we can check the size of this
file using:
```console
$ stat -c "%s" repo/metadata/1.targets.json 
1048
```
And that matches the `length` field above.

And we also can check the hash using:
```console
$ sha256sum repo/metadata/1.targets.json 
896781ff1260ed4ad5b05a004b034279219dc92b64068a2cc376604e8a6821c9  repo/metadata/1.targets.json
```
The snapshot metadata file tells us which which versions of metadata files
existed for a specific version. Even though we only have one version currently
in our example real world repositories would have multiple versions of
metadata files. The purpose of the snapshot.json file is to prevent an attacker
from including metadata files from another version in some version:
```
       TUF Repository              TUF Client
     +----------------+           +----------------+
     | timestamp.json | --------> | timestamp.json |
     | 1.targets.json | version 3 | - version 3    |
     | 2.targets.json | <-------  |                |
     | 3.targets.json | version 2 |                |
     |                | --------> |                |
     +----------------+           +----------------+kk
```
The above is trying to show that the client has received a timestamp.json,
which we will discuss shortly, with a version number of 3. The client proceeds
to retrieve this version update from the TUF server. Now if an attacker was able
to include other metadata files, which are available in the TUF repository only
they are not part of the request version, it would be possible for them those
targets to be sent to the client. Having the `snapshot.json` prevents this as
it specifies which metadata files are included in a specific version and no
other metadata files that may exist in the TUF repository are included.

So that leaves us with `timestamp.json`. This is a file that is downloaded by
the client and usually has a short expiration date. As mentioned before this is
the part that allows the system to enforce that updates are actually reaching
consumers, and if they are not they allow the consumer to take action.

The metadata looks like this:
```console
$ cat repo/metadata/timestamp.json 
{
  "signed": {
    "_type": "timestamp",
    "spec_version": "1.0.0",
    "version": 1,
    "expires": "2023-01-24T11:50:00.608598985Z",
    "meta": {
      "snapshot.json": {
        "length": 1004,
        "hashes": {
          "sha256": "b92c443c21b6bc15d4f3991491e8bcb201f66a26ab289fb8cc9af7f851530872"
        },
        "version": 1
      }
    }
  },
  "signatures": [
    {
      "keyid": "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f",
      "sig": "3708f055fe58b2c70e92cbc46bd9cc0f3149900bf25b3e924ff666eb8b45187df8d1f064b249cd790c170b5e97b322866d298d527ff950d2fbcbf508097868afca34ebaa159890799155c6bb615bab9a8bcfc34a39574584716d9a89b531fce97d876884fad2db69dc8f3569870dee280e87c9d506b5b08698e7c23e7dbbf4a3209fbc91ec764b54bf87367145cbb7bc9d7edaf47f709355284315fac9167312833d990e9e064852bb4fa905ec4edb5fe051480e70d505694528c5e9b47fefc3b78f6e54623f93344511326bdeec392a5eac31e7299bf9f602036d9f9524810eb03c4720370250f3e9f503e8d5bee94a6e6539ca9f988a27272d47612fd03436"
    }
  ]
}
```
Notice that `snapshot.json` is refering to the file `1.timestamp.json` in the
TUF repository. 

Finally, we have repo/metadata/1.root.json which is identical to root/root.json
which we saw previously.

What we have been doing is setting up a repository which would be on a server
somewhere.

From a consumer of the artifact (software) they would use TUF to download the
artifacts, and would specify the metadata and targets to download:
```console
$ tuftool download \
   --root root/root.json \
   --targets-url "file://$PWD/repo/targets" \
   --metadata-url "file://$PWD/repo/metadata" \
   downloaded-repo
```
And we can inspect the downloaded file:
```console
$ cat downloaded-repo/artifact_1.txt 
version 1 of artifact_1
```

Alright, now lets say we need create a new release of our target,
artifact_1.txt. To do this we would use the tuftool update command. First we
need to make a change to our artifact:
```console
$ echo "version 2 of artifact_1" > artifacts/artifact_1.txt
```
Now we can run the update command:
```console
$ tuftool update \
   --root root/root.json \
   --key keys/root.pem \
   --add-targets artifacts \
   --targets-expires 'in 3 weeks' \
   --targets-version 2 \
   --snapshot-expires 'in 3 weeks' \
   --snapshot-version 2 \
   --timestamp-expires 'in 1 week' \
   --timestamp-version 2 \
   --outdir repo \
   --metadata-url file:///$PWD/repo/metadata
```
After that command has been run there will have been a changes to
`repo/metadata/timestamp.json`:
```console
$ cat repo/metadata/timestamp.json 
{
  "signed": {
    "_type": "timestamp",
    "spec_version": "1.0.0",
    "version": 2,
    "expires": "2023-01-24T12:49:36.580297365Z",
    "meta": {
      "snapshot.json": {
        "length": 1004,
        "hashes": {
          "sha256": "05e44a53c09c018f812a7ffb59ecd147d4265c6ca2b10ba4e56a46397fb5d7ff"
        },
        "version": 2
      }
    }
  },
  "signatures": [
    {
      "keyid": "6e99ec437323f2c7334c8b16fd7a7a197829ba89ff50d07aa4b50fc9634dad9f",
      "sig": "73995d97b2df1ebe2044152d054febad55100f0699ec199ebd1e237c57242c9e536a5a53b9198eda8f965c1434c9b75829b7a34f6294176e9cee24b140a0737fb3c1c3b7fe13466c4ea2622b18177e78a9440cfd2e66fd46f2041bcb139722e175491a355f83f9c8f7f8d44366daddadb79a123d9d4d8886eb46f92a8e16a2900beb3daeba72d6fa815b82d7ab49af4246e2148dbd41854eef309aabb7ca6598ca52195357b4d0537bdd61a228cce2c8ee24d66a0465b528a2552a9fac65497619480c62b2beecfea67f05994eb39bf021800eeb0fede3f4dcf1e8c1211cf429a0e8db2fcd5c2680d4873911eeb1e4067b9d7b50aa60e8668cfbb01ba343d901"
    }
  ]
}
```
And we have two new metadata files, `repo/metadata/2.snapshot.json`, and
`repoo/metadata/2.targets.json`.

So lets simulate our downloading from the tuf server again:
```console
$ tuftool download \
   --root root/root.json \
   --targets-url "file://$PWD/repo/targets" \
   --metadata-url "file://$PWD/repo/metadata" \
   downloaded-repo2
```
And lets take a look at the artifact:
```console
$ cat downloaded-repo2/artifact_1.txt 
version 2 of artifact_1
```

Keys that are more likely to be compromised are signed with offline keys and
are used with specific purposes. For example, keys to publish software artifacts
would be an example of a key that is more likely to be compromised as probably
there are a number of individuals/systems that are allow to publish the
software.

### Heartbeats
This is about software updates and how to mitigate freeze or rollback
attacks. What we are trying to prevent is that software does not get updated
even when there are new updates to be installed from the provider. The provider
of the software wants to make sure that bug fixes get applied and that users are
not running old software (the freeze, or stale part of the attack I think). Lets
say an attacker is able to prevent updates from the provider, this software on
the consumers device should handle this in some way. 
For this TUF includes exploding timestamps, which works like this. The server
will commit to sending updates on a certain schedule. These updates will come
whether or not there actually are any software updates to be installed. If there
is no update, that is the timestamp expires, when there should have been, then
something needs to be done. What that action is is up to the implementer. 

Now, these exploding timestamps can also be used for key rotation. So we can
create keys which have a certain/short lifetime and which should be
rotated/resigned after that point.

#### Metadata signing
The roles mentioned later in this document talk about signing and this refers
to signing of metadata information. The format in the spec is a canonical JSON
format. So each role also has a metadata json file associated with it.


### Specification
[specification](https://theupdateframework.github.io/specification/latest/).

### Key Protection
Is about key management and preventing keys from leaking. The idea it has is
that keys will be leaked/lost and that is a situation that need to be handled.

Instead of having a single root key, there would be multiple root keys which
are stored in different offline locations. And these keys are used to sign keys
that can be used for other purposes.

So we have a number of keys that are `offline`, meaning that they are not used
for signing things, like software artifacts or things like that. Instead these
keys are often stored in separate locations and then used together to sign other
keys, which are then used for `online` tasks, like signing metadata describing
artifacts.
The non-root keys can then be re-signed/revoked/rotated if/when needed.

TUF specifies four top level roles which are the `Root`, `Target`, `Timestamp`,
and `Snapshot` roles.
The root role specifies which keys are trusted for the other roles.
