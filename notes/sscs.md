## Software Supply-Chain Security (SSCS)
Is about security around the products that users install, or use as thirdparty
dependencies, to allow them to be able to verify that nothing has been tampered
with, or replaced, when installing/using a piece of software.

There are many steps in the process of building software and making the produced
artifact available to users. The build process could be be tampered with, the
source control management system could be compromised, the packaging could
tampered, as well as the distribution server/channel.

The goal is to that an end user or system be able to verify that the software
they want to install or use can be verified that it comes from where they think
it comes from and that it has not been tampered with.

To achieve this we can use a combination of tools/products/project and this
document is my attempt to make sense of some/all of these. 

### Motivation for using tools/projects
Lets say we have a software project that we want to distribute. We want to sign
the artifact that we produce, lets say it's distributed as a tar file. This is
possible to to do manually but it involves some work like managing keys and
tools to perform the signing tasks. But using [sigstore](./sigstore.md)
simplfies this process, similar to how Let's Encrypt made it simpler to get
certificate to be used with web sites. Sigstore also provides tools to verify
signatures and transparency log to store certificates. So that allows us to
sign the end product and publish the code-signing certs to the transparancy
log and verify our artifact. But how can we trust what was built, like if I
build this on my local laptop I could replace a source code file with a backdoor
and still be able to produce a valid signature and end product/artifact. This
is also the case if a build server is used, and we need something more.

This is where another project named [in-toto](./in-toto.md) comes into play. It
contains tools to define steps of a build process, assign someone that is
responsible for that step and also to sign the artifact produced by that step.
Each step is signed by the person responsible for that step, called the
funtionary, and then all the steps are signed by a product owner. This will
produce a document which lists the steps that were followed to produce the
software with signatures for each step. So one step might have been to checking
out a specific git version, and this could be verified that it was indeed that
version that was used, the compiled object would also be signed and be
verifiable, the packaged tar also etc. This gives the end user insight into the
product that it is about to install.

Now, if the end user is using the above artifact in a project they might add
the verification step to their build process, like if they are using our
software to build their own product.

So we now have our built artifact signed and we have attestations, in this case
json files that contain metadata about how it was build. And we can use
in-toto-verify to verfify that all that information is correct.

Now, lets say that another company or another project want include our software
in theirs, like a thirdparty dependency. Ours might be one of many dependencies
that they include in their product and they might have requirements/restrictions
on what they are allow to use. For example, they might require that only certain
licences are used. The license information is hopefully available if the
projects have used in-toto, but there is nothing available to say that only
certain licenses are allowed. This is where [Open Policy Agent (OPA)](./opa.md)
comes into play. OPA gives us the ability to write policy rules that take
in-toto json files as input and verifies that there are licences for all
thirdparty dependencies and that they are of the type/types that are allowed.
Rules can be written to handle other types of restrictions/requirements as well,
which are the policies that the company has.

_wip_

### Definitions in SSCS
Just terms that were not clear, or needed refreshing.

#### Attestation
Like is sometimes required to get a new passport, some one needs
to attest, with their signature, that the photo is actually of you. So in this
context attestation could be signing of a build with the private key by the
person/system responible for that step.

#### Verification
Is the process of establishing that something is true. So in this
context it could be the process of verifying the signature of the build.

#### Provenance
Information about where, when, and how software the artifacts were
produced. In this context it could be information about the build, like the
system (CI/CD perhaps), date and the command used to build. 

