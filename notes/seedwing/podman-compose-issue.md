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
