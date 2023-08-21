## Trustification podman-compose issue
This document describes an issue I ran into when using podman-compose to bring
up a set of containers as described in
[DEVELOPING.md](https://github.com/trustification/trustification/blob/main/DEVELOPING.md)

### Issue
First, lets reset the podman environment to start fresh:
```console
$ podman system prune --all --force && podman rmi --all
```
Bring up the containers for minio, kafka, and keycloak with podman-compose:
```console
$ podman-compose -f compose.yaml  up
```
After this has completed I can access keycloak using http://localhost:8090/.

Before we can run the integration tests we need to wait for keycloak to be ready
which would be indicated by the following log message:
```console
[init-keycloak] | SSO initialization complete
```

We can inspect the keycloak container logs to see if we can find this log
message:
```console
$ podman logs compose_keycloak_1  | grep SSO
```
Keycloak also has a second container which we can try to inspect:
```console
$ podman logs compose_init-keycloak_1  | grep SSO
/usr/bin/bash: /init-sso/init.sh: Permission denied
```
Hmm, so I wonder if this is the cause of the error I'm seeing?  
Actually, if I search the original log I can see the following:
```console
[minio]
podman start -a compose_init-keycloak_1                                            
/usr/bin/bash: /init-sso/init.sh: Permission denied 
```


I've not been able to see the `SSO initialization complete` message and running
the integration tests without it will fail:
```console
$ cargo test -p integration-tests
   Compiling integration-tests v0.1.0 (/home/danielbevenius/work/security/trustification/trustification/integration-tests)
    Finished test [unoptimized + debuginfo] target(s) in 46.22s
     Running unittests src/lib.rs (target/debug/deps/integration_tests-5369399be06249d6)

running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s

     Running tests/bombastic.rs (target/debug/deps/bombastic-d0e4987307c96c7b)

running 16 tests
test test_delete_unauthorized ... FAILED
test test_invalid_type ... FAILED
test test_get_sbom_with_invalid_id ... FAILED
test test_upload ... FAILED
test test_delete ... FAILED
test test_invalid_encoding ... FAILED
test test_delete_missing ... FAILED
test bombastic_search ... FAILED
test test_delete_user_not_allowed ... FAILED
test test_get_sbom_with_missing_id ... FAILED
test test_upload_user_not_allowed ... FAILED
test test_upload_empty_json ... FAILED
test test_upload_empty_file ... FAILED
test test_upload_sbom_existing_with_change ... FAILED
test test_upload_sbom_existing_without_change ... FAILED
test test_upload_unauthorized ... FAILED

failures:

---- test_delete_unauthorized stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_delete_unauthorized' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:395:1

---- test_invalid_type stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_invalid_type' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:206:1

---- test_get_sbom_with_invalid_id stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_get_sbom_with_invalid_id' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:412:1

---- test_upload stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace
thread 'test_upload' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:14:1

---- test_delete stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_delete' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:37:1

---- test_invalid_encoding stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_invalid_encoding' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:227:1

---- test_delete_missing stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_delete_missing' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:84:1

---- bombastic_search stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'bombastic_search' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:121:1

---- test_delete_user_not_allowed stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_delete_user_not_allowed' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:375:1

---- test_get_sbom_with_missing_id stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_get_sbom_with_missing_id' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:421:1

---- test_upload_user_not_allowed stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_upload_user_not_allowed' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:340:1

---- test_upload_empty_json stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_upload_empty_json' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:294:1

---- test_upload_empty_file stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_upload_empty_file' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:317:1

---- test_upload_sbom_existing_with_change stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_upload_sbom_existing_with_change' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:271:1

---- test_upload_sbom_existing_without_change stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_upload_sbom_existing_without_change' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:249:1

---- test_upload_unauthorized stdout ----
thread '<unnamed>' panicked at 'called `Result::unwrap()` on an `Err` value: Http(reqwest::Error { kind: Status(404), url: Url { scheme: "http", cannot_be_a_base: false, username: "", password: None, host: Some(Domain("localhost")), port: Some(8090), path: "/realms/chicken/.well-known/openid-configuration", query: None, fragment: None } })', integration-tests/src/provider.rs:26:6
thread 'test_upload_unauthorized' panicked at 'explicit panic', integration-tests/tests/bombastic.rs:359:1


failures:
    bombastic_search
    test_delete
    test_delete_missing
    test_delete_unauthorized
    test_delete_user_not_allowed
    test_get_sbom_with_invalid_id
    test_get_sbom_with_missing_id
    test_invalid_encoding
    test_invalid_type
    test_upload
    test_upload_empty_file
    test_upload_empty_json
    test_upload_sbom_existing_with_change
    test_upload_sbom_existing_without_change
    test_upload_unauthorized
    test_upload_user_not_allowed

test result: FAILED. 0 passed; 16 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.42s

error: test failed, to rerun pass `-p integration-tests --test bombastic`
```

### Possible solution

Updating compose.yaml and adding the SELinux option `:Z` to the volume mount
so that contents are accessible by the container (private unshared)
```console
    volumes:
      - ./container_files/init-sso:/init-sso:Z
```
After this I was able to see the output in the logs
```console
+ echo SSO initialization complete
[init-keycloak] | SSO initialization complete
exit code: 0
```
And the integration test pass as well.

This might be a problem for non Linux systems though so we should probable try
to find a better solution for this. Actually, after reading up on this it sounds
like this option will simply be ignored on other systems so perhaps this would
be an acceptable solution after all. This was not the case and it would cause
an error on mac os for example. Instead we have proposed a solution using an
environment variable for Linus systems in this [PR](https://github.com/trustification/trustification/pull/408)

### Second issue
After being able to run the integration test I wanted to try out the user
interface which have containers declared in trustification.yaml. The logs
fly by pretty fast and it is difficult to notice if there is an error just by
looking at the output. But we can look at the individual logs for the containers
and specifically the spog-api container which is the one that is failing:
```console
$ podman logs compose_spog-api_1
[2023-08-21T08:12:18.624Z INFO  trustification_infrastructure::infra] Setting up infrastructure endpoint
[2023-08-21T08:12:18.626Z INFO  trustification_infrastructure::infra] Running infrastructure endpoint on:
[2023-08-21T08:12:18.626Z INFO  trustification_infrastructure::infra]    http://[::1]:9010
[2023-08-21T08:12:18.626Z INFO  trustification_infrastructure::infra]    http://127.0.0.1:9010
[2023-08-21T08:12:18.626Z INFO  actix_server::builder] starting 1 workers
[2023-08-21T08:12:18.626Z INFO  actix_server::server] Tokio runtime found; starting in existing Tokio runtime
Error: error sending request for url (http://keycloak:8080/realms/chicken/.well-known/openid-configuration): error trying to connect: tcp connect error: Connection refused (os error 111)
Caused by:
	error trying to connect: tcp connect error: Connection refused (os error 111)
	tcp connect error: Connection refused (os error 111)
	Connection refused (os error 111)
```

Now, we can use curl to inspect the openid configuration:
```console
$ curl http://localhost:8090/realms/chicken/.well-known/openid-configuration | jq
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100  5844  100  5844    0     0  1270k      0 --:--:-- --:--:-- --:--:-- 1426k
{
  "issuer": "http://localhost:8090/realms/chicken",
  "authorization_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/auth",
  "token_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/token",
  "introspection_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/token/introspect",
  "userinfo_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/userinfo",
  "end_session_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/logout",
  "frontchannel_logout_session_supported": true,
  "frontchannel_logout_supported": true,
  "jwks_uri": "http://localhost:8090/realms/chicken/protocol/openid-connect/certs",
  "check_session_iframe": "http://localhost:8090/realms/chicken/protocol/openid-connect/login-status-iframe.html",
  "grant_types_supported": [
    "authorization_code",
    "implicit",
    "refresh_token",
    "password",
    "client_credentials",
    "urn:ietf:params:oauth:grant-type:device_code",
    "urn:openid:params:grant-type:ciba"
  ],
  "acr_values_supported": [
    "0",
    "1"
  ],
  "response_types_supported": [
    "code",
    "none",
    "id_token",
    "token",
    "id_token token",
    "code id_token",
    "code token",
    "code id_token token"
  ],
  "subject_types_supported": [
    "public",
    "pairwise"
  ],
  "id_token_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512"
  ],
  "id_token_encryption_alg_values_supported": [
    "RSA-OAEP",
    "RSA-OAEP-256",
    "RSA1_5"
  ],
  "id_token_encryption_enc_values_supported": [
    "A256GCM",
    "A192GCM",
    "A128GCM",
    "A128CBC-HS256",
    "A192CBC-HS384",
    "A256CBC-HS512"
  ],
  "userinfo_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512",
    "none"
  ],
  "userinfo_encryption_alg_values_supported": [
    "RSA-OAEP",
    "RSA-OAEP-256",
    "RSA1_5"
  ],
  "userinfo_encryption_enc_values_supported": [
    "A256GCM",
    "A192GCM",
    "A128GCM",
    "A128CBC-HS256",
    "A192CBC-HS384",
    "A256CBC-HS512"
  ],
  "request_object_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512",
    "none"
  ],
  "request_object_encryption_alg_values_supported": [
    "RSA-OAEP",
    "RSA-OAEP-256",
    "RSA1_5"
  ],
  "request_object_encryption_enc_values_supported": [
    "A256GCM",
    "A192GCM",
    "A128GCM",
    "A128CBC-HS256",
    "A192CBC-HS384",
    "A256CBC-HS512"
  ],
  "response_modes_supported": [
    "query",
    "fragment",
    "form_post",
    "query.jwt",
    "fragment.jwt",
    "form_post.jwt",
    "jwt"
  ],
  "registration_endpoint": "http://localhost:8090/realms/chicken/clients-registrations/openid-connect",
  "token_endpoint_auth_methods_supported": [
    "private_key_jwt",
    "client_secret_basic",
    "client_secret_post",
    "tls_client_auth",
    "client_secret_jwt"
  ],
  "token_endpoint_auth_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512"
  ],
  "introspection_endpoint_auth_methods_supported": [
    "private_key_jwt",
    "client_secret_basic",
    "client_secret_post",
    "tls_client_auth",
    "client_secret_jwt"
  ],
  "introspection_endpoint_auth_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512"
  ],
  "authorization_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512"
  ],
  "authorization_encryption_alg_values_supported": [
    "RSA-OAEP",
    "RSA-OAEP-256",
    "RSA1_5"
  ],
  "authorization_encryption_enc_values_supported": [
    "A256GCM",
    "A192GCM",
    "A128GCM",
    "A128CBC-HS256",
    "A192CBC-HS384",
    "A256CBC-HS512"
  ],
  "claims_supported": [
    "aud",
    "sub",
    "iss",
    "auth_time",
    "name",
    "given_name",
    "family_name",
    "preferred_username",
    "email",
    "acr"
  ],
  "claim_types_supported": [
    "normal"
  ],
  "claims_parameter_supported": true,
  "scopes_supported": [
    "openid",
    "profile",
    "web-origins",
    "roles",
    "address",
    "email",
    "microprofile-jwt",
    "phone",
    "offline_access",
    "acr"
  ],
  "request_parameter_supported": true,
  "request_uri_parameter_supported": true,
  "require_request_uri_registration": true,
  "code_challenge_methods_supported": [
    "plain",
    "S256"
  ],
  "tls_client_certificate_bound_access_tokens": true,
  "revocation_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/revoke",
  "revocation_endpoint_auth_methods_supported": [
    "private_key_jwt",
    "client_secret_basic",
    "client_secret_post",
    "tls_client_auth",
    "client_secret_jwt"
  ],
  "revocation_endpoint_auth_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "HS256",
    "HS512",
    "ES256",
    "RS256",
    "HS384",
    "ES512",
    "PS256",
    "PS512",
    "RS512"
  ],
  "backchannel_logout_supported": true,
  "backchannel_logout_session_supported": true,
  "device_authorization_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/auth/device",
  "backchannel_token_delivery_modes_supported": [
    "poll",
    "ping"
  ],
  "backchannel_authentication_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/ext/ciba/auth",
  "backchannel_authentication_request_signing_alg_values_supported": [
    "PS384",
    "ES384",
    "RS384",
    "ES256",
    "RS256",
    "ES512",
    "PS256",
    "PS512",
    "RS512"
  ],
  "require_pushed_authorization_requests": false,
  "pushed_authorization_request_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/ext/par/request",
  "mtls_endpoint_aliases": {
    "token_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/token",
    "revocation_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/revoke",
    "introspection_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/token/introspect",
    "device_authorization_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/auth/device",
    "registration_endpoint": "http://localhost:8090/realms/chicken/clients-registrations/openid-connect",
    "userinfo_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/userinfo",
    "pushed_authorization_request_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/ext/par/request",
    "backchannel_authentication_endpoint": "http://localhost:8090/realms/chicken/protocol/openid-connect/ext/ciba/auth"
  }
}
```
But notice that the url that the spod-api is using is this:
```console
Error: error sending request for url (http://keycloak:8080/realms/chicken/.well-known/openid-configuration): error trying to connect: tcp connect error: Connection refused (os error 111)
```

Can we try starting the api and the ui manually:

First, spog-api:
```console
$ podman start -a compose_spog-api_1
[2023-08-21T08:58:26.008Z INFO  trustification_infrastructure::infra] Setting up infrastructure endpoint
[2023-08-21T08:58:26.009Z INFO  trustification_infrastructure::infra] Running infrastructure endpoint on:
[2023-08-21T08:58:26.009Z INFO  trustification_infrastructure::infra]    http://[::1]:9010
[2023-08-21T08:58:26.009Z INFO  trustification_infrastructure::infra]    http://127.0.0.1:9010
[2023-08-21T08:58:26.009Z INFO  actix_server::builder] starting 1 workers
[2023-08-21T08:58:26.009Z INFO  actix_server::server] Tokio runtime found; starting in existing Tokio runtime
[2023-08-21T08:58:26.071Z INFO  actix_web::middleware::logger] ::1 "GET / HTTP/1.1" 200 145 "-" "curl/7.76.1" 0.000045
[2023-08-21T08:58:26.101Z INFO  actix_server::builder] starting 6 workers
[2023-08-21T08:58:26.101Z INFO  actix_server::server] Tokio runtime found; starting in existing Tokio runtime
```

Then, spog-ui:
```console
$ podman start -a compose_spog-ui_1
+ set -o pipefail
Setting backend endpoint:
+ : http://localhost:8083
+ : '{}'
+ : /etc/config/console/backend.json
+ echo 'Setting backend endpoint:'
+ '[' -f /etc/config/console/backend.json ']'
+ echo '{}'
+ jq --arg url http://localhost:8083 '. + {url: $url}'
+ jq --arg url http://localhost:8082 '. + {bombastic: $url}'
+ jq --arg url http://localhost:8081 '. + {vexination: $url}'
+ jq --arg url http://localhost:8090/realms/chicken '. + {oidc: {issuer: $url}}'
+ tee /endpoints/backend.json
{
  "url": "http://localhost:8083",
  "bombastic": "http://localhost:8082",
  "vexination": "http://localhost:8081",
  "oidc": {
    "issuer": "http://localhost:8090/realms/chicken"
  }
}
Final backend information:
---
+ echo 'Final backend information:'
+ echo ---
+ cat /endpoints/backend.json
{
  "url": "http://localhost:8083",
  "bombastic": "http://localhost:8082",
  "vexination": "http://localhost:8081",
  "oidc": {
    "issuer": "http://localhost:8090/realms/chicken"
  }
}
---
+ echo ---
+ exec /usr/sbin/nginx -g 'daemon off;'
2023/08/21 08:59:01 [notice] 1#1: using the "epoll" event method
2023/08/21 08:59:01 [notice] 1#1: nginx/1.20.1
2023/08/21 08:59:01 [notice] 1#1: built by gcc 11.3.1 20221121 (Red Hat 11.3.1-4) (GCC) 
2023/08/21 08:59:01 [notice] 1#1: OS: Linux 6.2.9-200.fc37.x86_64
2023/08/21 08:59:01 [notice] 1#1: getrlimit(RLIMIT_NOFILE): 524288:524288
2023/08/21 08:59:01 [notice] 1#1: start worker processes
2023/08/21 08:59:01 [notice] 1#1: start worker process 9
2023/08/21 08:59:01 [notice] 1#1: start worker process 10
2023/08/21 08:59:01 [notice] 1#1: start worker process 11
2023/08/21 08:59:01 [notice] 1#1: start worker process 12
2023/08/21 08:59:01 [notice] 1#1: start worker process 13
2023/08/21 08:59:01 [notice] 1#1: start worker process 14
2023/08/21 08:59:01 [notice] 1#1: start worker process 15
2023/08/21 08:59:01 [notice] 1#1: start worker process 16
2023/08/21 08:59:01 [notice] 1#1: start worker process 17
2023/08/21 08:59:01 [notice] 1#1: start worker process 18
2023/08/21 08:59:01 [notice] 1#1: start worker process 19
2023/08/21 08:59:01 [notice] 1#1: start worker process 20
```

Doing that I'm then able to access the ui at http://localhost:8084/ and
log in using the `admin` user name and the password.

This seems to be an issue where the spog-api, bombastic and vexination and the
spog-ui are not able to connect to the keycloak instance because it is not
availble at that point, the container named `keycloak` is running but it was
not completed its start up and the url is not yet available when the other
containers start. One was around this is to start the compose.yaml separately
and after seeing the Keycloak instance is up and running, start the other
containers in compose-trustification.yaml.
