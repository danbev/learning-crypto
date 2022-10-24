## Secure Production Identification Framework for Everyone (SPIFFE)
To understand what SPIFFE solves consider a company that has software deployed
on multiple cloud providers and on-premise. These environments will most likley
be different, different authentication, different authorization etc.

An application running on one cloud should be able to address another
application running in a different cloud. It is a scheme for identifiation that
is not bound to a specific platform/technology/provider.

### SPIFFE ID
```
 spiffe://trust-domain/entity
```

### SPIFFE Verified Identity Document (SVID)
Two supported documents at the moment x509 and JWT (jot).

### Workload API
This is what allows a node to get an SVID without having any secret keys
or other credentials locally. So nodes can just pop-up connect to the Workload
API server and it will figure out what Id this particular node should get.

The is a workload agent which exposes the workload api natively. The agent
communicates with the workload server.

### SPIFFE Runtime Environment (SPIRE)
An implementation of SPIFFI.

#### Installation
```console
$ cd spire-example && cd spire-example
$ curl -s -N -L https://github.com/spiffe/spire/releases/download/v1.4.4/spire-1.4.4-linux-x86_64-glibc.tar.gz | tar xz
```

#### Starting the server
```console
$ spire-1.4.4/bin/spire-server run -config spire-1.4.4/conf/server/server.conf
```
Verify that the server is working:
```console
$ spire-1.4.4/bin/spire-server healthcheck
Server is healthy.
```

### Work flow
This section will be looking at SPIRE usage from the point of view of the agent
being on a build server, or part of a development work flow.

Request a one-time-use token from the spire server which is then shared with
the agent:
```console
$ spire-1.4.4/bin/spire-server token generate -spiffeID spiffe://example.org/myagent
Token: db38cbed-5bf9-439d-951a-e2d300b50a9d
```
Copy the above token as it will be used when we start an agent which would be
on the build server.
There are other possibilities instead of using a token and one using a x509
certificate. I'm thinking that we could generate a certificate it would be used
by the agent on the build server instead of having to generate a token like this
(which is only for testing).

The trust domain in this case is `example.org` and if we look closer as the
server console we find:
```console
DEBU[0000] Loading journal                path=data/server/journal.pem subsystem_name=ca_manager
INFO[0000] Journal loaded                 jwt_keys=2 subsystem_name=ca_manager x509_cas=2
INFO[0000] X509 CA activated              expiration="2022-10-24T23:12:38Z" issued_at="2022-10-17T23:12:38Z" slot=B subsystem_name=ca_manager
DEBU[0000] Successfully rotated X.509 CA  subsystem_name=ca_manager trust_domain_id="spiffe://example.org" ttl=114941.774451446
INFO[0000] JWT key activated              expiration="2022-10-25T00:46:38Z" issued_at="2022-10-18T00:46:38Z" slot=B subsystem_name=ca_manager
```
So we can't use just anything here, that trust domain has to be configured
before a token of this kind can be requested.

We need to create an entry in the SPIFFE server for our workload, which in our
case might be a build server:
```console
$ spire-1.4.4/bin/spire-server entry create -parentID spiffe://example.org/myagent -spiffeID spiffe://example.org/myservice -selector unix:uid:$(id -u)
Entry ID         : d3a886d6-699c-4643-b849-2cb675297d8d
SPIFFE ID        : spiffe://example.org/myservice
Parent ID        : spiffe://example.org/myagent
Revision         : 0
TTL              : default
Selector         : unix:uid:1000
```

With the token create by the server above, we can start up an agent using the
following command:
```console
$ spire-1.4.4/bin/spire-agent run -config spire-1.4.4/conf/agent/agent.conf -joinToken db38cbed-5bf9-439d-951a-e2d300b50a9d
```

And we can use the agent to get a x509 certificates:
```console
$ spire-1.4.4/bin/spire-agent api fetch x509 --write .
Received 1 svid after 2.003828ms

SPIFFE ID:		spiffe://example.org/myservice
SVID Valid After:	2022-10-24 05:51:03 +0000 UTC
SVID Valid Until:	2022-10-26 05:51:13 +0000 UTC
CA #1 Valid After:	2022-10-17 23:12:28 +0000 UTC
CA #1 Valid Until:	2022-10-24 23:12:38 +0000 UTC
CA #2 Valid After:	2022-10-23 15:16:56 +0000 UTC
CA #2 Valid Until:	2022-10-30 15:17:06 +0000 UTC

Writing SVID #0 to file svid.0.pem.
Writing key #0 to file svid.0.key.
Writing bundle #0 to file bundle.0.pem.
```

We can inspect the SVID, which is just a x509 certificate in this case:
```console
$ openssl x509 -in ./svid.0.pem -text -noout
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number:
            f3:7e:98:d3:55:55:93:6e:d7:37:f5:e4:be:48:ba:fb
        Signature Algorithm: ecdsa-with-SHA256
        Issuer: C = US, O = SPIFFE
        Validity
            Not Before: Oct 24 05:51:03 2022 GMT
            Not After : Oct 26 05:51:13 2022 GMT
        Subject: C = US, O = SPIRE, x500UniqueIdentifier = f7fa051b660f2be34e54056a8c4e281a
        Subject Public Key Info:
            Public Key Algorithm: id-ecPublicKey
                Public-Key: (256 bit)
                pub:
                    04:2f:2f:b8:f1:42:e3:d0:47:cd:f2:02:a8:25:6d:
                    c7:25:fb:da:86:dc:d1:d1:f9:c9:d7:83:ce:2d:e0:
                    d5:2e:7a:3e:15:3d:48:84:f3:50:5e:8e:72:07:75:
                    ba:19:16:0a:ee:24:09:3e:9c:25:3f:ee:27:1a:bb:
                    c4:51:f8:7c:e7
                ASN1 OID: prime256v1
                NIST CURVE: P-256
        X509v3 extensions:
            X509v3 Key Usage: critical
                Digital Signature, Key Encipherment, Key Agreement
            X509v3 Extended Key Usage: 
                TLS Web Server Authentication, TLS Web Client Authentication
            X509v3 Basic Constraints: critical
                CA:FALSE
            X509v3 Subject Key Identifier: 
                3A:9C:86:40:13:4B:8F:69:D1:3C:36:E4:DC:8C:C3:FD:F7:BB:FD:5F
            X509v3 Authority Key Identifier: 
                keyid:A5:86:AE:3F:45:B0:93:95:F2:D1:24:AA:AE:96:DF:E5:38:B1:5E:E3

            X509v3 Subject Alternative Name: 
                URI:spiffe://example.org/myservice
    Signature Algorithm: ecdsa-with-SHA256
         30:46:02:21:00:c7:79:6e:bb:ce:fa:92:d7:13:0d:18:3b:1d:
         09:22:03:c0:e7:93:c7:f0:21:5d:ed:dd:4b:4d:16:66:41:2b:
         d8:02:21:00:c1:07:1d:d0:6d:f4:f6:a4:b0:a2:b5:75:81:3f:
         fe:d6:b3:a8:41:e8:d6:67:31:00:73:65:00:a2:0b:27:d4:b4
```
Now, using this certificate we should be able to create a certificate signing
request (CSR) and sent it to Fulcio and we would get a code signing certificate
back which we can use for signing our build artifacts.
_Disclaimer_: I'm still figuring this stuff out...

