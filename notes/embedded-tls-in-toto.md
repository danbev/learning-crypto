## embedded-tls in-toto task
This document contains note around adding [in-toto](https://in-toto.io/) to
[embedded-tls](https://github.com/drogue-iot/embedded-tls).

### Goals
One goal of this task is to gain a better understanding of what is involved in
adding code-signing, and code attestation to a source distributed open source
project. 
A second goal is to understand how verification of the attestations can done for
source distributed projects.

### Creating the software supply chain layout
For open source software dependency projects we making a few assumptions which
might not hold for all such projects but a good portion of them. We are assuming
they are rather small and won't require multple physical funcionariess. And
since these are source distributed software project they don't contains any
packaging (like tar, jar, rpm, etc).

First we will create unsigned in-toto metadata for the project:
```console
$ in-toto-mock --name clone_project -- git clone https://github.com/drogue-iot/embedded-tls.git
Running 'sscs'...
Recording materials '.'...
Running command 'git clone https://github.com/drogue-iot/embedded-tls.git'...
Cloning into 'embedded-tls'...
Recording products '.'...
Creating link metadata...
Storing unsigned link metadata to 'sscs.link'...
```
This will generate a `clone_project.link` metadata file by recording everything
in the cloned repository. But there will be no signatures in this file, not yet.

So that would be one step in the process. After that is done we should run
add an attestation that the tests of the project have been run:
```console
$ in-toto-mock --name run_tests -- cargo t --manifest-path embedded-tls/Cargo.toml
Running 'run_tests'...
Recording materials '.'...
Running command 'cargo t --manifest-path embedded-tls/Cargo.toml'...
    Updating crates.io index
    Updating git repository `https://github.com/lulf/webpki`
   Compiling libc v0.2.135
   ...
(in-toto-mock) TimeoutExpired: Command '['cargo', 't', '--manifest-path', 'embedded-tls/Cargo.toml']' timed out after 10.0 seconds
```
As we can see above this will time out. Building could be done as we are only
really interested in running the tests for this step but I'm getting the feeling
that this might not be the correct way for going about this. What we want is that
a user can verify that the test suite was run. 

_wip_


