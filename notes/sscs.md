## Software Supply-Chain Security (SSCS)
Is about security around the products that users install to allow them to be
able to verify that nothing has been tampered with, or replaced, when installing
a piece of software.

There are many points in the process of building software and making it
available to users. The build process could be be tampered with, the packaging
could tampered with like replacing some parts. Also the repository that host the
packaged product could be compromised.

The goal is to that an end user/system be able to verify that the software they
want to install/use can be verified to come from where they think it comes from
and that it has not been tampered with.

To achieve this we can use a combination of tools/products/project and this
document is my attempt to make sense of some/all of these. 

Lets say we have a software project that we want to distribute. We want to sign
the artifact that we produce, lets say it is a tar tar file. This is possible to
to do manually using which [sigstore](./sigstore.md) simplfies this process for
us. It also provides a tool to verify signatures and transparency log to store
certificates. So that allows us to verify that end product was signed by who
we think it was signed by. Lets say that this is done by part of an automatted
process where a build server first compiles the code, runs tests etc. What if
the build server has been compromised in some way, we would still end up with
a valid signature and the users system might be compromised. This where another
project named [in-toto](./in-toto.md) comes into play. It contains tools to
define steps of a build process, assign someone that is responsible for that
step and also to sign the artifact produces by that step. Each step is signed
by the person responsible called the funtionary, and then all the steps are
signed by a product owner. This will produce a document which lists the steps
that were followed to produce the software with signatures for each step. So
one set might have been to checking out a specific git version, and this could
be verified that it was indeed that version that was used, the compiled object
would also be signed and be verifiable, the packaged tar also etc. This gives
the end user insight into the product that it is about to install.

Now, if the end user is using the above "product" in a project they might add
the verification step to their build process, like if they are using our
product to build their product.

That was not too bad for a single project perhaps but lets think about a larger
company where there are lot of things getting deployed by many different teams. 
In this case might want to enforce certain rules about what is considered
alright to use in production, like licences, only using thirdparty packages/libs
that are supported. Using [OPA](./opa.md) to define rules which can enforce
these requirements/restrictions would allow them to be shared among projects.
Exposing a policy endpoint as a service would allow bulid processes, CI/CD
processes, container environments, to verify these policies.

_wip_

One nice thing about OPA is that they are testable.


