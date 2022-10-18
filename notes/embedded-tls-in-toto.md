## embedded-tls in-toto task
This document contains notes about adding [in-toto](https://in-toto.io/) to
[embedded-tls](https://github.com/drogue-iot/embedded-tls).

### Goals
One goal of this task is to gain a better understanding of what is involved in
adding code-signing and code attestation to a source distributed open source
project. 

A second goal is to understand how verification of the attestations can done for
source distributed projects.

### Questions
When do we want to generate the in-toto artifacts?  
I think will probably only be done at the time of a new release. So each time
a release is doing to be released the tool is run to verify that the sources,
and run the tests. My initial thought was that we should also be creating .link
file for things like creating a branch, updating the version, creating a tag
etc. That might be alright for one project but will be harder to generalize.

What I'm currently trying to figure out is that for source distributed projects
there is no package/archive release where we can stick the .link files and the
layout file which are needed for verification. I think it would be nice to
store these in the source repository which makes them available when fetching
them with Cargo or any other tool. So this would be a step called as part of
the release process.

### Creating the software supply chain layout
For open source software dependency projects we are making a few assumptions
which might not hold for all projects but for a good portion of them. We are
assuming they are rather small and won't require multple physical funcionaries.
And since these are source distributed software project they don't contains any
packaging steps (like tar, jar, rpm, etc).

First we will create unsigned in-toto metadata for the project:
```console
$ in-toto-mock --name clone_project -- git clone https://github.com/drogue-iot/embedded-tls.git
Running 'clone_project'...
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
We could build a manual task first:
```console
$ cargo b --manifest-path=embedded-tls/Cargo.toml
```
And then running `in-toto-mock`:
```console
$ in-toto-mock --name run_tests -- cargo t --manifest-path embedded-tls/Cargo.toml
Running 'run_tests'...
Recording materials '.'...
Running command 'cargo t --manifest-path embedded-tls/Cargo.toml'...
...
test result: ok. 1 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 1.46s

Recording products '.'...
Creating link metadata...
Storing unsigned link metadata to 'run_tests.link'...
```

Next we want to update the version number:
```console
$ in-toto-record start --step-name update-version --materials embedded-tls/Cargo.toml
```
TODO: find if there is a Cargo tool that is used for this, for example
cargo release.
This won't work because we need a key or gpg id to use this command.

What I'd really want is to use sigstore to generate a keypair for signing using
OIDC and then using that private key in the commands.

I'm going to create the example using a generated keypair and then revisit this
part with the intention of integrating this with sigstore so that this process
can be "keyless".

### Creating the software supply chain layout (take 2)
I've created a bash script to perform the steps while doing this investigation.
```console
$ sscs.sh
```

_wip_
