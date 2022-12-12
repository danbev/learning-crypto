### OpenID Connect (OICD)
Is built on top of the OAuth 2.0 framework and is an identity layer that allows
applications to verify an end user to obtain basic user profile information.
It uses JSON Web Token (JWT).

OAuth 2.0 is only about authorization, about granting access to resources. For
example you want a thirdparty application to be able to access data you own. One
way would be to give you credentials to the application that needs to access
this data, but that means that application can access all you data. Instead we
want to limit the scope of what can be accessed and also have this access scoped
in time (token).


First, the client application could be some web application or mobil application
that I'm using. This application wants to access information which is available
to my github account `danbev`. So the client application sends a request to
github's authentication server requesting access for the user `danbev`:
```
   +-------------+                  +-------------+
   | Client App1 |----------------->| Github      |
   +-------------+                  +-------------+
```

This will cause the Gihub authentication server to prompts me for
authentication and also ask if I consent to what share with this application:
```                                                        
   +-------------+                  +-------------+     +-------------------+
   | Client App1 |                  | Github      |---->| Prompts user (me) |
   +-------------+                  +-------------+     | for auth/consent  |
                                                        +-------------------+
```
If I consent Github will return a token that enables access for to my github
account but only the data that I consented to:
```
   +-------------+  token           +-------------+
   | Client App1 |←---------------- | Github      |
   +-------------+                  +-------------+
```

The client application can now access Github's API using the token:
```
   +-------------+                  +-------------+
   | Client App1 |                  | Github      |
   +-------------+                  +-------------+
         |    request with token    +-------------+
         +-------------------------→| Github API  |
                                    +-------------+
```



OICD is a layer that sits on top of OAuth 2.0 and adds login and profile
information about the Resource Owner called the identity.
OICD can be used for single sign-on where one login can be used for multiple
applications.

### OAuth 2.0 Roles

#### Resource Owner
The app/person who owns the resource that we want to controll access to.

#### Resource Server
The server that hosts the above mentioned resource.

#### Client
The client that is requesting access to the resource.

#### Authorization Server
The server that authorizes the client app.

### OAuth 2.0 flow

```
              +-------------+
              | Auth Server |
              +-------------+               
                ↑                               +----------------+
          (1)   |                               | Resource Server|
        +-------+                               +----------------+
        |                                                ↑
  +------------+        access token                     |
  | Client App |←----------------------------------------+
  +------------+                                         
  | User:      |
  +------------+

```
1. The client application will contact the Authorization Server and request
access for the `User`. So the client will be able to access a resorce that the
user owns.


### Identity token
This is a token that an application gets from the authentication server when
the application asks the server to authenticate a user. After a successful
login the application will recieve an identiy token which contains information
about the user, like username, email, etc. This is signed and also contains
access information (role mappings) which say what applications this users is
allowed access to.

### Access token
This is when a client wants to gain access to a remote service. The access token
can be used to invoke a remote service on the clients behalf. The server first
authenticates the user and then asks if it consents to grant access to the
client requesting it.
