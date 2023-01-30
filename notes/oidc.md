### OpenID Connect (OICD)
Is built on top of the OAuth 2.0 framework and is an identity layer that allows
applications to verify an end user to obtain basic user profile information.
It uses JSON Web Token [JWT](./jwt.md).

OAuth 2.0 is only about authorization, about granting access to resources. For
example, you want a thirdparty application to be able to access data you own.
One way would be to give your credentials to the application that needs to
access this data, but that means that application can access all your data.
Instead we want to limit the scope of what can be accessed, and also have this
access scoped in time (token).

I'm going to start going through the OAuth 2.0 flows and then follow on with
OIDC which I hope will makes sense after going through this.

### Back channel 
Like when we have a server that we controll and make a https request to some
other server.

### Front channel
Less secure channel is something like a browser and while the browser is secure
but the code in the browser can leak information, like one can inspect the
source if there is anything stored there, it is available to everyone. 

### Client Registration
The first thing that happens is that the client application needs to register
with the authorization server.

The client needs to specify what the client_type, which can be confidential
or public. Confidential means that the client is cabable of storing credentials
securely. Public means that the client is not able to store credentials securely
and is used for native device clients, web browser-based apps.

This process will provide the client application with a client_id,
a client_secret, and provides the authorization server with a callback uri.

### Authorization Code Flow (OAuth 2.0 flow)
Imagine we have written an application and one feature is that it allows users
to show their private github repositories. In the application that we have
written there might be a button saying "Show private Github repos", which would
redirect the user browser to github.com to login, and also prompt the user
asking if they are willing to allow the application we wrote to access the users
private repositories.

The client application starts by issueing a request to the auth server and this
request contains the type of the repsonse the client expects, the is HTTP
request parameter named [response_type](https://www.rfc-editor.org/rfc/rfc6749#section-3.1.1).
In our case this will be `code` which can be thought of as Authorization Grant
Code. The client also indicates which scopes that it want to have access to.
These scopes are defined by the auth server and are specific to it. These will
be shown when the user is prompted about allowing access to their resource.

The [redirect_uri](https://www.rfc-editor.org/rfc/rfc6749#section-3.1.2) can
be specified or the redirect used when registrating the client can be used.

Client issues a get request to the auth server:
```
                             (front channel)
 +------------------------+                    +----------------+
 |  +-------+             |                    | Auth Server    |
 |  | App   |             |                    |                |
 |  +-------+             |                    |                |
 |   |                    |                    | Prompt to login|
 |   ↓                    |                    |                |
 |  +-----------+         |                    |                |
 |  | Browser   |         |   request          | Prompt to allow|
 |  |           |----------------------------->| access scope   |
 |  +-----------+         |                    | `repo`         |
 |                        |                    |                |
 +------------------------+                    +----------------+

https://github.com/login/oauth/authorize
  client_id=<client_id from registration>
  redirect_uri=https://something.com/callback
  scope=repo
  response_type=code
  state=bajja
```
Notice that the client_id is not sensitive and can be public, but the
client_secret is which is only used on the back channel (more on this later).
The state is just some string that the server will send back to the client in
the response.

The auth server responds, using the redirect_uri, with an authorization code:
```
                            (front channel)
 +------------------------+                    +----------------+
 |  +-------+             |                    | Auth Server    |
 |  | App   |             |                    |                |
 |  +-------+             |                    |                |
 |       ^                |                    |                |
 |       |                |                    |                |
 |  +-----------+         |                    |                |
 |  | Browser   |         |                    |                |
 |  |           |<-----------------------------|                |
 |  +-----------+         |     code           |                |
 |                        |                    |                |
 +------------------------+                    +----------------+

GET https://something.com/callback
  code=<authorization code grant>
  state=bajja
```
The authorization code is opaque to the client, it contains information about
the what the user has granted the holder of the code to do.

Keep in mind that these are HTTPS requests and the parameter are sent in the
query, and these are available to the browser side code, or my reading the
address bar if that is possible.

So we have a code grant which we can then use request a access token. In this
case our application, which remember is a server application with callback uri
able to handle callbacks from the browser, will this time a direct https
request to the authorization server (not via the brower). This will be a post
request and the client will have to specify credentials. If someone was able
to get their hands on the code grant in the browser in some way, they would
still not be able to make the post request as this is on the highly secure
back channel. But is we did not have the back channel and only used the front
channel then what would have been possible. I'm trying to show the motivation
of having this first request on the front channel and then the reason when we
have another request over the back channel. The secret key cannot be in the
browser.

This code can be used to request an access token from the authorization server
using the [authorization endpoint](https://www.rfc-editor.org/rfc/rfc6749#section-3.1):
```
                             (back channel)
 +------------------------+                    +----------------+
 |  +-------+             |                    | Auth Server    |
 |  | App   |--------------------------------->|                |
 |  +-------+             |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                | 
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 +------------------------+                    +----------------+

POST https://github.com/login/oauth/access_token
  code=<from response above>
  client_id=<client_id from registration>
  client_secret=<client_id from registration>
  grant_type=authorization_code
```

The authorization server will verify the code passed to is and if it looks ok
it will return an access token:
```
                             (back channel)
 +------------------------+                    +----------------+
 |  +-------+             |                    | Auth Server    |
 |  | App   |<---------------------------------|                |
 |  +-------+             |  access token      |                |
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                | 
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 +------------------------+                    +----------------+

{
  "access_token": "",
  "expires_in": 1234,  //  seconds
  "token_type": "Bearer"

}
```
The client can now use the access token to talk to the resource server and
now be able to access the data allowed by the code grant:
```
                             (back channel)
 +------------------------+                    +----------------+
 |  +-------+             |                    | Auth Server    |
 |  | App   |--------------------------------->| Validate token |
 |  +-------+             |  access token      | Authorize scope|
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                | 
 |                        |                    |                |
 |                        |                    |                |
 |                        |                    |                |
 +------------------------+                    +----------------+

GET https://github.com/repo
Authorization: Bearer <access_token>
```

### Implicit Grant Flow (OAuth 2.0 flow)
This is intended for public clients, those that only use the front channel like
a web browser. In this flow there is no extra step where we use the back channel
, because there is none, and instead of getting back the code we get back the
access token directly. For example, this could be use for a single page webapp
written in JavaScript.

### Resource Owner Password Credentials Grant (OAuth 2.0 flow)
This can be used when we have no front channel, but instead only have a back
channel. In this case the resource owner is able to provide their username and
password to the application, this is also a back channel only flow, and these
are passed to the authentication server, and will then get back the access
token directly.
Must sent a `grant_type` set to `password`.

### Client Credentials Grant (OAuth 2.0 flow)
This is similar to the previous flow and also a back channel only. In this case
an access token is requested using only the clients credentials.
Must sent a `grant_type` set to `client_credentials`.


### Summary OAuth flows
Notice that the above flows only deal with authorization and not authentication
which is not handled by the spec. The spec deals with granting permissions, that
is authorization and was the original purpose of OAuth. But in the beginning
people also used OAuth for authentication (which can be very confusing).
And also notice in the above flows we have only been dealing with code and
access tokens which only deal with scopes and are not about who is accessing
the resource (I mean there is no information in tokens about the user). 

The led to different implementation from Facebook, Google, Microsoft, for
getting user information with OAuth and that is not great for interoperability.
So a standard was needed to specify this kind of user information data. The
solution was extend OAuth 2.0 with authentication which led to Open ID Connect.

### OICD
OICD is a layer that sits on top of OAuth 2.0, and adds login and profile
information about the Resource Owner called the identity.

It adds an ID Token which represent the users information. There is also a
userinfo endpoint where additional information about the user can be requested.
So when we use OAuth to request an access token, we can also specify that we
want an id_token in addition to the access token.
This is specified on the initial request to the authentication server using
the Scope HTTP header with a value of `openid`.

OICD can be used for single sign-on where one login can be used for multiple
applications.

The IdToken can be use by the client application to access information about
the user.

The ID Token is a [jwt](./jwt.md).

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
Are JWTs that conforms to the OIDC spec. These are made up on key-value pairs.

This is a token that an application gets from the authentication server when
the application asks the server to authenticate a user. After a successful
login the application will recieve an identiy token which contains information
about the user, like username, email, etc. This is signed, and also contains
access information (role mappings) which say what applications this users is
allowed access to.

### Access token
This is when a client wants to gain access to a remote service. The access token
can be used to invoke a remote service on the clients behalf. The server first
authenticates the user and then asks if it consents to grant access to the
client requesting it.
These are opaque tokens and are specific to the server that provides them. So
there is no way to inspect/parse them in a standard way.
