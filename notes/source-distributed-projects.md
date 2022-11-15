## Source distributed projects
This document contains notes about an attempt to secure source distributed
projects, in particular smaller project that are often used as third-party
depencencies by larger project.

The motivation for this is that adding signing/attestations/sbom for our 
project is great, but a lot or project have a number of third-party dependencies
and their security level is unknown may be vulnerable to exploits which also
means that our projects security level is unknown in reality.

I might be naive here, but could we start helping open source projects, like the
ones that are most used by our customers and help them setup in-toto or
something else.

This document is a description/journal of the experience of trying this out.

## Using cosign keys for signing with in-toto
The goal here was to use cosign's ephemeral keys, and then use them with in-toto.
An additional goal was that this process not require any human interaction and
that it should be possible to run it as a github action. 

The first task was to setup a github action that uses githubs OIDC access token 
and pass that to Fulcio, Sigstore's Certificate Authority (CA).
TODO: add link to the github action.

We wrote to program in Rust that uses
[sigstore-rs](https://github.com/sigstore/sigstore-rs) to request a
signing-certificate from Fulcio and save the short-lived keys and certificate
to disk. The keys types from Fulcio are `ecdsa` which in-toto did not have
support for in their command line tools. We created and issue for this addition,
[#519](https://github.com/in-toto/in-toto/issues/519), and the in-toto command
line tools now have support for these types.

Next we wanted to use these keys for creating the in-toto layout and also for
creating the steps. Now, as mentioned earlier the keys from cosign are of type
`ecdsa` and in-toto now also has support for `ecdsa` but in-toto requires that
the format of the `ecdsa` keys be in `securesystemslib` json format whereas the
keys from cosign are in pem format.

So we added a Python script to perform this conversion, python because in-toto
and securesystemslib is written in python and we could use methods provided.
But we ran into an
[issue](https://github.com/secure-systems-lab/securesystemslib/pull/457) with
how the method create the `keyid` field. We were able to work around this in the
script. As others might have the same need to doing this conversion we have
created an [issue](https://github.com/in-toto/in-toto/issues/522) in in-toto
suggesting something be created for doing this (and perhaps hide some of the
internal details regarding the json format).

With those changes we can now have a github action that creates an in-toto
layout which is signed, and also create the steps (currently only a git clone
and running of tests) and verify the layout. But this verification and signing
is not using the additional Sigstore components like Rekor, the transparency
log. The next step is to look into how this can be made possible.

_work in progress_
