## Supply-Chain Levels for Software Artifacts (SLSA)
My take on this is that it is about being able to have a common language/Levels
to describe how secure some software is, including its dependencies.

Software producers can follow these guidelines and consumers can make decisions
based on this. In the long term I think there vision is to have tools that help
achieve these levels for software producers and also tools for verifying by
consoumers. TODO: are there any such tool currently available?

### Level 1
The build process is automated and documented and includes how the artifact was
built, what the source was, and what dependencies it has. This is called
provenance. This is not signed so there is no way to detect tampering, but this
can still be valuable as the source can be identified and having the
dependencies documented can help in vulnerability management/scanning.

For example, only the build commands might be documented but they
would not be signed like they would for example when using in-toto.
This could probably be done with in-toto, just without signing the steps and
layout (or perhaps even without the layout).

### Level 2
Tamper reistance of the build service. In this something that proves that the
build was performed in a secure way which I think would match what in-toto can
provide by specifying a functionary that performs specific steps of the build
process and also signs that work. This level also mentions that the build
service should be hosted.

### Level 3
This level extends on Level 2 with additional requirements that the source
control and build platforms must meet. 

### Level 4
Requires 2-person review of all changes, a hermetic (self-contained) which means
that the build is fully specified up front, so compiler versions are specified,
thirdparty dependeny version are as well, any other build tools etc.

### Dependencies
One thing to not is that the levels are not transitive, so if my project is at
Level 3 then that only means the sources I've produces are at that level, not
that the dependencies I use are at that level. So I'm could still be vulnerable
to software supply chain attacks if one of those dependencies are exploited.

### github-generators
https://github.com/slsa-framework/slsa-github-generator



