## in-toto attestation
When we sign an artifact, like a blob, the signature proves that we were in
possesion of the private key. When we verify, we use the signature, the public
key, and the blob, and we are verifying that this was in fact the case.
But it does not say anything else about the artifact, we don't know what was
actually signed.

By providing, and signing, a document specifying `statements` about the artifact
we can say things about the artifact as well. Statements could be anything which
we will address later in this document. A signed Statement is called an
Attestation.

An attestation is authenticated metadata about software artifacts and follows
the [Software-chain Levels for Software Artifacts attestation model](https://slsa.dev/attestation-model).

An attestation is a json object and the outer-most layer is the `Envelope`:
```json
{
  "payloadType": "application/vnd.in-toto+json",
  "payload": "<Base64(Statement)>",
  "signatures": [
    { "keyid": "<KEYID", "sig": "<Base64(SIGNATURE)>" },
  ]
```

Notice that the `payload` is a base64 encoded `Statement`. This format follows
the [DSSE](./dsse.md) format. As mentioned in the DSSE notes the signatures
is the `signatures` array  of signature elements.

The `payloadType` could be JSON, CBOR, or ProtoBuf.

The structure of the `Statement`, payload above, looks something like this
before it is base64 encoded:
```json
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "subject": [ {
      "name": "<NAME>",
      "digest": {"<ALGORITHM>": "<HEX_VALUE>"}
    },
  ],
  "predicateType": "<URI>",
  "predicate": {}
}
```
The subjects bind this attestation to a set of software artifacts, notice that
this is an array of objects.

Each software artifact is given a name and a digest. The digest contains the
name of the hashing algorithm used and the digest (the outcome of the hash
function).
The name could be a file name but it can also be left unspecified using `_`.

This leads us to the `predicate` fields, which like shown above has one field
for the type of the predicate, and an object as the content of the predicate.

The predicate can contain pretty much any metadata related to the Statement
object's subjects. The `predicateType` provides a way of knowing how to
interpret the `predicate` field.

Examples of predicate types are
[SLSA Provenance](https://slsa.dev/provenance/v0.1#example),
[in-toto Link](https://github.com/in-toto/attestation/blob/main/spec/predicates/link.md)
, [SPDX](https://github.com/in-toto/attestation/blob/main/spec/predicates/spdx.md)
, [Software Supply Chain Attribute Integrity (SCAI)](https://github.com/in-toto/attestation/blob/main/spec/predicates/scai.md).

NPM also uses this for it publish [attestation](https://github.com/npm/rfcs/blob/main/accepted/0049-link-packages-to-source-and-build.md#slsa-provenance-schema):
```json
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "subject": [{
    "name": "pkg:npm/@scope/package-foo@1.4.3",
    "digest": { "sha512": "41o0P/CEffYGDqvo2pHQXRBOfFOxvYY3WkwkQTy..." }
  }],
  "predicateType": "https://github.com/npm/attestation/tree/main/specs/publish/v0.1",
  "predicate": {
    "name": "@scope/package-foo",
    "version": "1.4.3",
    "registry": "https://registry.npmjs.org",
  }
}
```
The `digest` in this case is the sha512sum of the published tar file.

So, we mentioned that the predicate type is used by the consumer of the
predicate so it knows how to interpret the contents of the predicate. But who
is the consumer?  
Most often this would be a Policy Engine. The Policy engine would be passed
the contents of the Statement related to the predicate, and rules written in
the Policy Engine's language would process the predicate as input. The outcome
would be a true/false result (remember that a predicate is a statement/function
that returns true or false).

Let's take a look at a concrete example.

### Attestation example
To try to make this a little more concrete lets take a look at an example
that creates an attestation.

For this example I'm going to use a GitHub Action named
[slsa-github-generator](https://github.com/slsa-framework/slsa-github-generator/blob/main/internal/builders/generic/README.md) which can generate SLSA provenance attestations for github
native projects. This generator can generate SLSA provenance for SLSA level 3.

The example project I'm going to use is [tuf-keyid](https://github.com/danbev/tuf-keyid)
but the actual project is not important in this case, any Rust project could
have been used.

So we need to set up a GitHub Action which can been seen in
[provenance.yaml](https://github.com/danbev/tuf-keyid/blob/main/.github/workflows/provenance.yaml).

After that workflow has run it will produce an attestation and a binary which
we will use to verify.

First, we need to download the binary from the [workflow run](https://github.com/danbev/tuf-keyid/actions/runs/4015220869) (we should really be able to be download this from the releases page too, but I've not been able to
get that to work just yet):
```console
$ unzip tuf-keyid.zip
$ file tuf-keyid
tuf-keyid: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=ceaa62d49b024798ebd7fe7d021f3ade5925b1f9, for GNU/Linux 3.2.0, with debug_info, not stripped
```
And then we need to download the attestation file:
```console
$ curl -L https://github.com/danbev/tuf-keyid/releases/download/v0.2.0/tuf-keyid.intoto.jsonl --output tuf-keyid.intoto.jsonl
```

These files have been downloaded and are available in [in-toto](../in-toto/).

Lets inspect the attestation file:
```console
$ cat tuf-keyid.intoto.jsonl | jq
{
  "payloadType": "application/vnd.in-toto+json",
  "payload": "...",
  "signatures": [
    {
      "keyid": "",
      "sig": "MEUCIHwmJopmrXWqi+rKIeTlWW0r027hLL1nO7xEj0mW8czsAiEAhdc6SDlhWo3m0YOtsUSoIYSlvw3Xu7ts3S8btHzdMpw=",
      "cert": "-----BEGIN CERTIFICATE-----\nMIIDtjCCAzygAwIBAgIUCeak2sfkfZbS0IMRSbK4+BHcUzAwCgYIKoZIzj0EAwMw\nNzEVMBMGA1UEChMMc2lnc3RvcmUuZGV2MR4wHAYDVQQDExVzaWdzdG9yZS1pbnRl\ncm1lZGlhdGUwHhcNMjMwMTI2MTAxMzQxWhcNMjMwMTI2MTAyMzQxWjAAMFkwEwYH\nKoZIzj0CAQYIKoZIzj0DAQcDQgAEZMurC3H80wzo+Xn7uifeTDV/AAFnye8uFwEj\n5VmxJb30VzuEw8gD8/Dj4V79bIW9sePcZjvREhFWak+PhUZVMqOCAlswggJXMA4G\nA1UdDwEB/wQEAwIHgDATBgNVHSUEDDAKBggrBgEFBQcDAzAdBgNVHQ4EFgQUncOT\nSyRyKgylBYlUHwPF+EyemfkwHwYDVR0jBBgwFoAU39Ppz1YkEZb5qNjpKFWixi4Y\nZD8wgYQGA1UdEQEB/wR6MHiGdmh0dHBzOi8vZ2l0aHViLmNvbS9zbHNhLWZyYW1l\nd29yay9zbHNhLWdpdGh1Yi1nZW5lcmF0b3IvLmdpdGh1Yi93b3JrZmxvd3MvZ2Vu\nZXJhdG9yX2dlbmVyaWNfc2xzYTMueW1sQHJlZnMvdGFncy92MS40LjAwOQYKKwYB\nBAGDvzABAQQraHR0cHM6Ly90b2tlbi5hY3Rpb25zLmdpdGh1YnVzZXJjb250ZW50\nLmNvbTASBgorBgEEAYO/MAECBARwdXNoMDYGCisGAQQBg78wAQMEKDUxM2IwZTA2\nMGM3NmExZGVkN2IxYTQxNzMxNjUxMDM4MzhmOGRkZTcwFQYKKwYBBAGDvzABBAQH\nUHVibGlzaDAeBgorBgEEAYO/MAEFBBBkYW5iZXYvdHVmLWtleWlkMB4GCisGAQQB\ng78wAQYEEHJlZnMvdGFncy92MC4yLjAwgYoGCisGAQQB1nkCBAIEfAR6AHgAdgDd\nPTBqxscRMmMZHhyZZzcCokpeuN48rf+HinKALynujgAAAYXtkZ+vAAAEAwBHMEUC\nIQDgO+S94sXq3wcfg344IV8FRhynvsJsVFEfHmwOHGqAVgIgArfX+7pnaLrplJ0u\nXB6tlWaCxQJ7GAo9YByqXCa0b2gwCgYIKoZIzj0EAwMDaAAwZQIxAOYkXbpLbSqC\njdORW6lWGWB/Ts2aOhK7VAHaQCRgRHQGiZx4Pe/LCwqkQF/1W2BAEQIwLB9Ic2jt\nIiEjtw8xKFDQAfnUleNUtZ51LXgXEkdpIX9cnj4UdR6k4gu/wul16Bd8\n-----END CERTIFICATE-----\n"
    }
  ]
}
```
If we look back at the beginning of this document we will see that this format
matches the `Envelope` of the attestation, and we have the `payloadType`,
a `payload`, and `signatures`.

The certificate can be inspected using:
```console
$ cat tuf-keyid.intoto.jsonl | jq -r '.signatures[].cert' | openssl x509 --text
```

Recall that the payload is a base64 encoded `Statement`. Let's decode the
`Statement` and take a closer a look at it:
```console
$ cat tuf-keyid.intoto.jsonl | jq -r '.payload' | base64 -d | jq
{
  "_type": "https://in-toto.io/Statement/v0.1",
  "predicateType": "https://slsa.dev/provenance/v0.2",
  "subject": [
    {
      "name": "tuf-keyid",
      "digest": {
        "sha256": "470c549740f98fe1b1977d48e014031ed5183785fd459df7e04605daefe8e293"
      }
    }
  ],
  "predicate": {
    "builder": {
      "id": "https://github.com/slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@refs/tags/v1.4.0"
    },
    "buildType": "https://github.com/slsa-framework/slsa-github-generator/generic@v1",
    "invocation": {
      "configSource": {
        "uri": "git+https://github.com/danbev/tuf-keyid@refs/tags/v0.2.0",
        "digest": {
          "sha1": "513b0e060c76a1ded7b1a4173165103838f8dde7"
        },
        "entryPoint": ".github/workflows/provenance.yaml"
      },
      "parameters": {},
      "environment": {
        "github_actor": "danbev",
        "github_actor_id": "432351",
        "github_base_ref": "",
        "github_event_name": "push",
        "github_event_payload": {
          "after": "13c69c54cbd04d1920cc5e42441f0a693a371494",
          "base_ref": null,
          "before": "0000000000000000000000000000000000000000",
          "commits": [],
          "compare": "https://github.com/danbev/tuf-keyid/compare/v0.2.0",
          "created": true,
          "deleted": false,
          "forced": false,
          "head_commit": {
            "author": {
              "email": "daniel.bevenius@gmail.com",
              "name": "Daniel Bevenius",
              "username": "danbev"
            },
            "committer": {
              "email": "daniel.bevenius@gmail.com",
              "name": "Daniel Bevenius",
              "username": "danbev"
            },
            "distinct": true,
            "id": "513b0e060c76a1ded7b1a4173165103838f8dde7",
            "message": "Add content(releases) write permission\n\nSigned-off-by: Daniel Bevenius <daniel.bevenius@gmail.com>",
            "timestamp": "2023-01-26T11:09:02+01:00",
            "tree_id": "5030177fa47fc8b8252c26e8556083b4abc5df71",
            "url": "https://github.com/danbev/tuf-keyid/commit/513b0e060c76a1ded7b1a4173165103838f8dde7"
          },
          "pusher": {
            "email": "daniel.bevenius@gmail.com",
            "name": "danbev"
          },
          "ref": "refs/tags/v0.2.0",
          "repository": {
            "allow_forking": true,
            "archive_url": "https://api.github.com/repos/danbev/tuf-keyid/{archive_format}{/ref}",
            "archived": false,
            "assignees_url": "https://api.github.com/repos/danbev/tuf-keyid/assignees{/user}",
            "blobs_url": "https://api.github.com/repos/danbev/tuf-keyid/git/blobs{/sha}",
            "branches_url": "https://api.github.com/repos/danbev/tuf-keyid/branches{/branch}",
            "clone_url": "https://github.com/danbev/tuf-keyid.git",
            "collaborators_url": "https://api.github.com/repos/danbev/tuf-keyid/collaborators{/collaborator}",
            "comments_url": "https://api.github.com/repos/danbev/tuf-keyid/comments{/number}",
            "commits_url": "https://api.github.com/repos/danbev/tuf-keyid/commits{/sha}",
            "compare_url": "https://api.github.com/repos/danbev/tuf-keyid/compare/{base}...{head}",
            "contents_url": "https://api.github.com/repos/danbev/tuf-keyid/contents/{+path}",
            "contributors_url": "https://api.github.com/repos/danbev/tuf-keyid/contributors",
            "created_at": 1674117641,
            "default_branch": "main",
            "deployments_url": "https://api.github.com/repos/danbev/tuf-keyid/deployments",
            "description": "A command line tool to print the key id for a TUF public key in JSON format.",
            "disabled": false,
            "downloads_url": "https://api.github.com/repos/danbev/tuf-keyid/downloads",
            "events_url": "https://api.github.com/repos/danbev/tuf-keyid/events",
            "fork": false,
            "forks": 0,
            "forks_count": 0,
            "forks_url": "https://api.github.com/repos/danbev/tuf-keyid/forks",
            "full_name": "danbev/tuf-keyid",
            "git_commits_url": "https://api.github.com/repos/danbev/tuf-keyid/git/commits{/sha}",
            "git_refs_url": "https://api.github.com/repos/danbev/tuf-keyid/git/refs{/sha}",
            "git_tags_url": "https://api.github.com/repos/danbev/tuf-keyid/git/tags{/sha}",
            "git_url": "git://github.com/danbev/tuf-keyid.git",
            "has_discussions": false,
            "has_downloads": true,
            "has_issues": true,
            "has_pages": false,
            "has_projects": true,
            "has_wiki": true,
            "homepage": null,
            "hooks_url": "https://api.github.com/repos/danbev/tuf-keyid/hooks",
            "html_url": "https://github.com/danbev/tuf-keyid",
            "id": 590801502,
            "is_template": false,
            "issue_comment_url": "https://api.github.com/repos/danbev/tuf-keyid/issues/comments{/number}",
            "issue_events_url": "https://api.github.com/repos/danbev/tuf-keyid/issues/events{/number}",
            "issues_url": "https://api.github.com/repos/danbev/tuf-keyid/issues{/number}",
            "keys_url": "https://api.github.com/repos/danbev/tuf-keyid/keys{/key_id}",
            "labels_url": "https://api.github.com/repos/danbev/tuf-keyid/labels{/name}",
            "language": "Rust",
            "languages_url": "https://api.github.com/repos/danbev/tuf-keyid/languages",
            "license": null,
            "master_branch": "main",
            "merges_url": "https://api.github.com/repos/danbev/tuf-keyid/merges",
            "milestones_url": "https://api.github.com/repos/danbev/tuf-keyid/milestones{/number}",
            "mirror_url": null,
            "name": "tuf-keyid",
            "node_id": "R_kgDOIzbqXg",
            "notifications_url": "https://api.github.com/repos/danbev/tuf-keyid/notifications{?since,all,participating}",
            "open_issues": 0,
            "open_issues_count": 0,
            "owner": {
              "avatar_url": "https://avatars.githubusercontent.com/u/432351?v=4",
              "email": "daniel.bevenius@gmail.com",
              "events_url": "https://api.github.com/users/danbev/events{/privacy}",
              "followers_url": "https://api.github.com/users/danbev/followers",
              "following_url": "https://api.github.com/users/danbev/following{/other_user}",
              "gists_url": "https://api.github.com/users/danbev/gists{/gist_id}",
              "gravatar_id": "",
              "html_url": "https://github.com/danbev",
              "id": 432351,
              "login": "danbev",
              "name": "danbev",
              "node_id": "MDQ6VXNlcjQzMjM1MQ==",
              "organizations_url": "https://api.github.com/users/danbev/orgs",
              "received_events_url": "https://api.github.com/users/danbev/received_events",
              "repos_url": "https://api.github.com/users/danbev/repos",
              "site_admin": false,
              "starred_url": "https://api.github.com/users/danbev/starred{/owner}{/repo}",
              "subscriptions_url": "https://api.github.com/users/danbev/subscriptions",
              "type": "User",
              "url": "https://api.github.com/users/danbev"
            },
            "private": false,
            "pulls_url": "https://api.github.com/repos/danbev/tuf-keyid/pulls{/number}",
            "pushed_at": 1674727856,
            "releases_url": "https://api.github.com/repos/danbev/tuf-keyid/releases{/id}",
            "size": 21,
            "ssh_url": "git@github.com:danbev/tuf-keyid.git",
            "stargazers": 1,
            "stargazers_count": 1,
            "stargazers_url": "https://api.github.com/repos/danbev/tuf-keyid/stargazers",
            "statuses_url": "https://api.github.com/repos/danbev/tuf-keyid/statuses/{sha}",
            "subscribers_url": "https://api.github.com/repos/danbev/tuf-keyid/subscribers",
            "subscription_url": "https://api.github.com/repos/danbev/tuf-keyid/subscription",
            "svn_url": "https://github.com/danbev/tuf-keyid",
            "tags_url": "https://api.github.com/repos/danbev/tuf-keyid/tags",
            "teams_url": "https://api.github.com/repos/danbev/tuf-keyid/teams",
            "topics": [],
            "trees_url": "https://api.github.com/repos/danbev/tuf-keyid/git/trees{/sha}",
            "updated_at": "2023-01-19T10:26:19Z",
            "url": "https://github.com/danbev/tuf-keyid",
            "visibility": "public",
            "watchers": 1,
            "watchers_count": 1,
            "web_commit_signoff_required": false
          },
          "sender": {
            "avatar_url": "https://avatars.githubusercontent.com/u/432351?v=4",
            "events_url": "https://api.github.com/users/danbev/events{/privacy}",
            "followers_url": "https://api.github.com/users/danbev/followers",
            "following_url": "https://api.github.com/users/danbev/following{/other_user}",
            "gists_url": "https://api.github.com/users/danbev/gists{/gist_id}",
            "gravatar_id": "",
            "html_url": "https://github.com/danbev",
            "id": 432351,
            "login": "danbev",
            "node_id": "MDQ6VXNlcjQzMjM1MQ==",
            "organizations_url": "https://api.github.com/users/danbev/orgs",
            "received_events_url": "https://api.github.com/users/danbev/received_events",
            "repos_url": "https://api.github.com/users/danbev/repos",
            "site_admin": false,
            "starred_url": "https://api.github.com/users/danbev/starred{/owner}{/repo}",
            "subscriptions_url": "https://api.github.com/users/danbev/subscriptions",
            "type": "User",
            "url": "https://api.github.com/users/danbev"
          }
        },
        "github_head_ref": "",
        "github_ref": "refs/tags/v0.2.0",
        "github_ref_type": "tag",
        "github_repository_id": "590801502",
        "github_repository_owner": "danbev",
        "github_repository_owner_id": "432351",
        "github_run_attempt": "1",
        "github_run_id": "4014167952",
        "github_run_number": "13",
        "github_sha1": "513b0e060c76a1ded7b1a4173165103838f8dde7"
      }
    },
    "metadata": {
      "buildInvocationID": "4014167952-1",
      "completeness": {
        "parameters": true,
        "environment": false,
        "materials": false
      },
      "reproducible": false
    },
    "materials": [
      {
        "uri": "git+https://github.com/danbev/tuf-keyid@refs/tags/v0.2.0",
        "digest": {
          "sha1": "513b0e060c76a1ded7b1a4173165103838f8dde7"
        }
      }
    ]
  }
}
```
So that gives us a concrete example of an attestation and in this case it is
a [SLSA Provenance](https://slsa.dev/provenance/v0.1) predicate.

Notice that the `digest`in the subject array is the sha256sum of the tuf-keyid
binary:
```console
$ cat tuf-keyid.intoto.jsonl | jq -r '.payload' | base64 -d | jq -r '.subject[].digest.sha256'
470c549740f98fe1b1977d48e014031ed5183785fd459df7e04605daefe8e293
$ sha256sum tuf-keyid
470c549740f98fe1b1977d48e014031ed5183785fd459df7e04605daefe8e293  tuf-keyid
```

Alright, so next step if to verify the binary that we produced, using the
attestation.

There is project named [slsa-verifier](https://github.com/slsa-framework/slsa-verifier#example)
which can be used to verify the artifact.

Installing `slsa-verifier`:
```console
$ go install github.com/slsa-framework/slsa-verifier/v2/cli/slsa-verifier@v2.0.1
```

Let's try verifying the attestation using `slsa-verifier` and using a local
build of the binary, that is a local build on my laptop:
```console
$ slsa-verifier verify-artifact --provenance-path tuf-keyid.intoto.jsonl \
  --source-uri github.com/danbev/tuf-keyid \
   ~/work/rust/tuf-keyid/target/release/tuf-keyid
Verified signature against tlog entry index 11978552 at URL: https://rekor.sigstore.dev/api/v1/log/entries/24296fb24b8ad77a217b8f07bccab3dc8caa1c7badf65f104a762647e5e355db23ccc13a22e275dd
FAILED: SLSA verification failed: expected hash '32dcff46ec4be5462a66aeb5d82366da3b870d36796f3d1fe6fec6245f21ce6f' not found: artifact hash does not match provenance subject
```

And now let's see what happens when we try with the binary that was produced by
the GitHub action:
```console
$ slsa-verifier verify-artifact --provenance-path tuf-keyid.intoto.jsonl \
  --source-uri github.com/danbev/tuf-keyid \
   tuf-keyid
Verified signature against tlog entry index 11978552 at URL: https://rekor.sigstore.dev/api/v1/log/entries/24296fb24b8ad77a217b8f07bccab3dc8caa1c7badf65f104a762647e5e355db23ccc13a22e275dd
Verified build using builder https://github.com/slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@refs/tags/v1.4.0 at commit 513b0e060c76a1ded7b1a4173165103838f8dde7
PASSED: Verified SLSA provenance
```
This has shown an example of in-toto attestations, namely
[SLSA Provenance](https://slsa.dev/provenance/v0.1).

`slsa-verifier` can also print out the predicate information after validation
, using `--print-provenance`, which could then be passed to a Policy Engine:
```console
$ slsa-verifier verify-artifact --provenance-path tuf-keyid.intoto.jsonl   --source-uri github.com/danbev/tuf-keyid    tuf-keyid --print-provenance
Verified signature against tlog entry index 11978552 at URL: https://rekor.sigstore.dev/api/v1/log/entries/24296fb24b8ad77a217b8f07bccab3dc8caa1c7badf65f104a762647e5e355db23ccc13a22e275dd
Verified build using builder https://github.com/slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@refs/tags/v1.4.0 at commit 513b0e060c76a1ded7b1a4173165103838f8dde7
{"_type":"https://in-toto.io/Statement/v0.1","predicateType":"https://slsa.dev/provenance/v0.2","subject":[{"name":"tuf-keyid","digest":{"sha256":"470c549740f98fe1b1977d48e014031ed5183785fd459df7e04605daefe8e293"}}],"predicate":{"builder":{"id":"https://github.com/slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@refs/tags/v1.4.0"},"buildType":"https://github.com/slsa-framework/slsa-github-generator/generic@v1","invocation":{"configSource":{"uri":"git+https://github.com/danbev/tuf-keyid@refs/tags/v0.2.0","digest":{"sha1":"513b0e060c76a1ded7b1a4173165103838f8dde7"},"entryPoint":".github/workflows/provenance.yaml"},"parameters":{},"environment":{"github_actor":"danbev","github_actor_id":"432351","github_base_ref":"","github_event_name":"push","github_event_payload":{"after":"13c69c54cbd04d1920cc5e42441f0a693a371494","base_ref":null,"before":"0000000000000000000000000000000000000000","commits":[],"compare":"https://github.com/danbev/tuf-keyid/compare/v0.2.0","created":true,"deleted":false,"forced":false,"head_commit":{"author":{"email":"daniel.bevenius@gmail.com","name":"Daniel Bevenius","username":"danbev"},"committer":{"email":"daniel.bevenius@gmail.com","name":"Daniel Bevenius","username":"danbev"},"distinct":true,"id":"513b0e060c76a1ded7b1a4173165103838f8dde7","message":"Add content(releases) write permission\n\nSigned-off-by: Daniel Bevenius \u003cdaniel.bevenius@gmail.com\u003e","timestamp":"2023-01-26T11:09:02+01:00","tree_id":"5030177fa47fc8b8252c26e8556083b4abc5df71","url":"https://github.com/danbev/tuf-keyid/commit/513b0e060c76a1ded7b1a4173165103838f8dde7"},"pusher":{"email":"daniel.bevenius@gmail.com","name":"danbev"},"ref":"refs/tags/v0.2.0","repository":{"allow_forking":true,"archive_url":"https://api.github.com/repos/danbev/tuf-keyid/{archive_format}{/ref}","archived":false,"assignees_url":"https://api.github.com/repos/danbev/tuf-keyid/assignees{/user}","blobs_url":"https://api.github.com/repos/danbev/tuf-keyid/git/blobs{/sha}","branches_url":"https://api.github.com/repos/danbev/tuf-keyid/branches{/branch}","clone_url":"https://github.com/danbev/tuf-keyid.git","collaborators_url":"https://api.github.com/repos/danbev/tuf-keyid/collaborators{/collaborator}","comments_url":"https://api.github.com/repos/danbev/tuf-keyid/comments{/number}","commits_url":"https://api.github.com/repos/danbev/tuf-keyid/commits{/sha}","compare_url":"https://api.github.com/repos/danbev/tuf-keyid/compare/{base}...{head}","contents_url":"https://api.github.com/repos/danbev/tuf-keyid/contents/{+path}","contributors_url":"https://api.github.com/repos/danbev/tuf-keyid/contributors","created_at":1674117641,"default_branch":"main","deployments_url":"https://api.github.com/repos/danbev/tuf-keyid/deployments","description":"A command line tool to print the key id for a TUF public key in JSON format.","disabled":false,"downloads_url":"https://api.github.com/repos/danbev/tuf-keyid/downloads","events_url":"https://api.github.com/repos/danbev/tuf-keyid/events","fork":false,"forks":0,"forks_count":0,"forks_url":"https://api.github.com/repos/danbev/tuf-keyid/forks","full_name":"danbev/tuf-keyid","git_commits_url":"https://api.github.com/repos/danbev/tuf-keyid/git/commits{/sha}","git_refs_url":"https://api.github.com/repos/danbev/tuf-keyid/git/refs{/sha}","git_tags_url":"https://api.github.com/repos/danbev/tuf-keyid/git/tags{/sha}","git_url":"git://github.com/danbev/tuf-keyid.git","has_discussions":false,"has_downloads":true,"has_issues":true,"has_pages":false,"has_projects":true,"has_wiki":true,"homepage":null,"hooks_url":"https://api.github.com/repos/danbev/tuf-keyid/hooks","html_url":"https://github.com/danbev/tuf-keyid","id":590801502,"is_template":false,"issue_comment_url":"https://api.github.com/repos/danbev/tuf-keyid/issues/comments{/number}","issue_events_url":"https://api.github.com/repos/danbev/tuf-keyid/issues/events{/number}","issues_url":"https://api.github.com/repos/danbev/tuf-keyid/issues{/number}","keys_url":"https://api.github.com/repos/danbev/tuf-keyid/keys{/key_id}","labels_url":"https://api.github.com/repos/danbev/tuf-keyid/labels{/name}","language":"Rust","languages_url":"https://api.github.com/repos/danbev/tuf-keyid/languages","license":null,"master_branch":"main","merges_url":"https://api.github.com/repos/danbev/tuf-keyid/merges","milestones_url":"https://api.github.com/repos/danbev/tuf-keyid/milestones{/number}","mirror_url":null,"name":"tuf-keyid","node_id":"R_kgDOIzbqXg","notifications_url":"https://api.github.com/repos/danbev/tuf-keyid/notifications{?since,all,participating}","open_issues":0,"open_issues_count":0,"owner":{"avatar_url":"https://avatars.githubusercontent.com/u/432351?v=4","email":"daniel.bevenius@gmail.com","events_url":"https://api.github.com/users/danbev/events{/privacy}","followers_url":"https://api.github.com/users/danbev/followers","following_url":"https://api.github.com/users/danbev/following{/other_user}","gists_url":"https://api.github.com/users/danbev/gists{/gist_id}","gravatar_id":"","html_url":"https://github.com/danbev","id":432351,"login":"danbev","name":"danbev","node_id":"MDQ6VXNlcjQzMjM1MQ==","organizations_url":"https://api.github.com/users/danbev/orgs","received_events_url":"https://api.github.com/users/danbev/received_events","repos_url":"https://api.github.com/users/danbev/repos","site_admin":false,"starred_url":"https://api.github.com/users/danbev/starred{/owner}{/repo}","subscriptions_url":"https://api.github.com/users/danbev/subscriptions","type":"User","url":"https://api.github.com/users/danbev"},"private":false,"pulls_url":"https://api.github.com/repos/danbev/tuf-keyid/pulls{/number}","pushed_at":1674727856,"releases_url":"https://api.github.com/repos/danbev/tuf-keyid/releases{/id}","size":21,"ssh_url":"git@github.com:danbev/tuf-keyid.git","stargazers":1,"stargazers_count":1,"stargazers_url":"https://api.github.com/repos/danbev/tuf-keyid/stargazers","statuses_url":"https://api.github.com/repos/danbev/tuf-keyid/statuses/{sha}","subscribers_url":"https://api.github.com/repos/danbev/tuf-keyid/subscribers","subscription_url":"https://api.github.com/repos/danbev/tuf-keyid/subscription","svn_url":"https://github.com/danbev/tuf-keyid","tags_url":"https://api.github.com/repos/danbev/tuf-keyid/tags","teams_url":"https://api.github.com/repos/danbev/tuf-keyid/teams","topics":[],"trees_url":"https://api.github.com/repos/danbev/tuf-keyid/git/trees{/sha}","updated_at":"2023-01-19T10:26:19Z","url":"https://github.com/danbev/tuf-keyid","visibility":"public","watchers":1,"watchers_count":1,"web_commit_signoff_required":false},"sender":{"avatar_url":"https://avatars.githubusercontent.com/u/432351?v=4","events_url":"https://api.github.com/users/danbev/events{/privacy}","followers_url":"https://api.github.com/users/danbev/followers","following_url":"https://api.github.com/users/danbev/following{/other_user}","gists_url":"https://api.github.com/users/danbev/gists{/gist_id}","gravatar_id":"","html_url":"https://github.com/danbev","id":432351,"login":"danbev","node_id":"MDQ6VXNlcjQzMjM1MQ==","organizations_url":"https://api.github.com/users/danbev/orgs","received_events_url":"https://api.github.com/users/danbev/received_events","repos_url":"https://api.github.com/users/danbev/repos","site_admin":false,"starred_url":"https://api.github.com/users/danbev/starred{/owner}{/repo}","subscriptions_url":"https://api.github.com/users/danbev/subscriptions","type":"User","url":"https://api.github.com/users/danbev"}},"github_head_ref":"","github_ref":"refs/tags/v0.2.0","github_ref_type":"tag","github_repository_id":"590801502","github_repository_owner":"danbev","github_repository_owner_id":"432351","github_run_attempt":"1","github_run_id":"4014167952","github_run_number":"13","github_sha1":"513b0e060c76a1ded7b1a4173165103838f8dde7"}},"metadata":{"buildInvocationID":"4014167952-1","completeness":{"parameters":true,"environment":false,"materials":false},"reproducible":false},"materials":[{"uri":"git+https://github.com/danbev/tuf-keyid@refs/tags/v0.2.0","digest":{"sha1":"513b0e060c76a1ded7b1a4173165103838f8dde7"}}]}}
PASSED: Verified SLSA provenance
```
It might not be clear but I've saved the attestation in a file named
[provenance.json](../in-toto/provenance.json).

Now, that worked well for a binary artifact but in Rust the releases are more
often source code and would not have a binary to sign. 
TODO: Take a closer look at what NPM does.


### in-toto sbom attestations
Similar to the provenance discussed above there is also a need to be able to
sign sboms to that they cannot be tampered with. SBOMs can be a added as a
predicate, just like the SLSA provenance predicate.

Currently in-toto has specifications for
[SPDX](https://github.com/in-toto/attestation/blob/main/spec/predicates/spdx.md)
and there is an open pull issue for adding
[CycloneDX](https://github.com/in-toto/attestation/issues/82). 

---------------

### Validation
This section contains notes about in-toto [attestation validation].
That document defines the input to the validation as follows

* blob to verify
* the DSSE envelope
* recognizedAttesters
  This is a collection of name/publicKey pairs
* acceptableDigestAlgorithms

The `recognizedAttesters` I think allow us to specify a name and associate a
publickey or certificate with that name.

For example, we might have a envelope that looks like this:
```
{
  "payload": "<Base64(SERIALIZED_BODY)>",
  "payloadType": "<PAYLOAD_TYPE>",
  "signatures": [
    { "keyid": "", "cert": "<Base64(CERTIFICATE1)>, "sig": "<Base64(SIGNATURE1)>" },
    { "keyid": "", "cert": "<Base64(CERTIFICATE2)>, "sig": "<Base64(SIGNATURE2)>" },
  ]
}
```
And we could provide `recognizedAttesters` like:
```
"dan": "<Base64(CERTIFICATE1)>"  //
```
Now, we have two signatures and lets say we only want to verify the first one.
We don't have a keyid to specify as it is empty, so we need to specify the
base64 encoded certificate. And we also give it a name that is easier to
refer to.



### SLSA github generator notes
So how does the above builder generate the in-toto predicate?  
Lets take a look at https://github.com/slsa-framework/slsa-github-generator.

If we look at the [workflow](https://github.com/danbev/tuf-keyid/blob/main/.github/workflows/provenance.yaml)
definition:
```yaml
 provenance:
    needs: [build]
    permissions:
      actions: read # To read the workflow path.
      id-token: write # To sign the provenance.
      contents: write # To add assets to a release.
    uses: slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@v1.4.0
    with:
      base64-subjects: "${{ needs.build.outputs.hashes }}"
      upload-assets: true # Optional: Upload to a new release
```
We can see that this step `uses` [slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@v1.4.0](https://github.com/slsa-framework/slsa-github-generator/blob/main/.github/workflows/generator_generic_slsa3.yml).
```
BUILDER_DIR: internal/builders/generic # Source directory if we compile the builder.
```

Now, recall that the oidc token is provided 
So I'm thinking that the main entry point will be
https://github.com/slsa-framework/slsa-github-generator/blob/main/internal/builders/generic/main.go:
```go
c.AddCommand(attestCmd(nil, checkExit, sigstore.NewDefaultFulcio(), sigstore.NewDefaultRekor()))
```

https://github.com/slsa-framework/slsa-github-generator/blob/main/internal/builders/generic/attest.go#L37
```go
func attestCmd(provider slsa.ClientProvider, check func(error),
	signer signing.Signer, tlog signing.TransparencyLog,
) *cobra.Command {
  ...
  g := slsa.NewHostedActionsGenerator(&b)
  ...
  p, err := g.Generate(ctx) 
```
Notice that `signer` in this case is what is returned from
`sigstore.NewDefaultFulcio`.
```go
type Fulcio struct {
  fulcioAddr   string
  oidcIssuer   string
  oidcClientID string
}
```

[NewHostedActionsGenerator](https://github.com/slsa-framework/slsa-github-generator/blob/main/slsa/provenance.go#L45) is the type and [Generate](https://github.com/slsa-framework/slsa-github-generator/blob/main/slsa/provenance.go#L53) is method.
```go
func (g *HostedActionsGenerator) Generate(ctx context.Context) (*intoto.ProvenanceStatement, error) {
  // NOTE: Use buildType as the audience as that closely matches the intended
  // recipient of the OIDC token.
  // NOTE: GitHub doesn't allow github.com in the audience so remove it.
  audience := githubComReplace.ReplaceAllString(g.buildType.URI(), "")

    if oidcClient != nil {
      t, err := oidcClient.Token(ctx, []string{audience})
      if err != nil {
        return nil, err
      }
      if t.JobWorkflowRef != "" {
        builderID = fmt.Sprintf("https://github.com/%s", t.JobWorkflowRef)
      }
   }
   ...

  return &intoto.ProvenanceStatement{
    StatementHeader: intoto.StatementHeader{
      Type:          intoto.StatementInTotoV01,
      PredicateType: slsa02.PredicateSLSAProvenance,
      Subject:       subject,
     },
     Predicate: slsa02.ProvenancePredicate{
       BuildType: g.buildType.URI(),
       Builder: slsacommon.ProvenanceBuilder{
         ID: builderID,
       },
       Invocation:  invocation,
       BuildConfig: buildConfig,
       Materials:   materials,
       Metadata:    metadata,
     },
  }, nil
}
```
Looking at the code notice `oidcClient.Token` which is what requests an access
token from githubs OIDC provider:
```go
func (c *OIDCClient) Token(ctx context.Context, audience []string) (*OIDCToken, error) {
  tokenBytes, err := c.requestToken(ctx, audience)
  ...
```
The `requestToken` function is simliar to what we have done manually using
curl to get an oidc access code/token from githubs OIDC authorization server:
```console
     - name: Generate OIDC Token                                               
        id: token                                                               
        run: |                                                                  
          echo oidc_token=$(curl -sLS "${ACTIONS_ID_TOKEN_REQUEST_URL}" \          
            -H "User-Agent: actions/oidc-client" \                                 
            -H "Authorization: Bearer $ACTIONS_ID_TOKEN_REQUEST_TOKEN" \           
            | jq '.value' | tr '"' ' ') >> $GITHUB_OUTPUT 
```
This will return an OIDC jwt token. And notice that they are using the
`audience` when requesting the token.
Now, what I seem to be missing is that I can see that they are requesting an
OIDC token in exchange for the access grant code/token. Then they are taking
the job_workflow_ref and using that in the builderId. But I don't see where
the token is actually used, I mean where is it sent to Fulcio?
This is used later when signing, which is done back in attest.go
```go
att, err := signer.Sign(ctx, &intoto.Statement{
  StatementHeader: p.StatementHeader,
  Predicate:       p.Predicate,
})
```
And signer was created using `sigstore.NewDefaultFulcio()` and the `sign`
function  can be found in https://github.com/slsa-framework/slsa-github-generator/blob/main/signing/sigstore/fulcio.go#L79.
```go
func (s *Fulcio) Sign(ctx context.Context, p *intoto.Statement) (signing.Attestation, error) {
  ...
  k, err := fulcio.NewSigner(ctx, options.KeyOpts{
    OIDCIssuer:   s.oidcIssuer,
    OIDCClientID: s.oidcClientID,
    FulcioURL:    s.fulcioAddr,
  })
}
```

https://github.com/sigstore/cosign/blob/main/cmd/cosign/cli/fulcio/fulcio.go
```go
func NewSigner(ctx context.Context, ko options.KeyOpts) (*Signer, error) {
  fClient, err := NewClient(ko.FulcioURL)
  if err != nil {
    return nil, fmt.Errorf("creating Fulcio client: %w", err)
  }
  idToken, err := idToken(ko.IDToken)
  ...
  var provider providers.Interface
  // If token is not set in the options, get one from the provders
  if idToken == "" && providers.Enabled(ctx) && !ko.OIDCDisableProviders {
	if ko.OIDCProvider != "" {
		provider, err = providers.ProvideFrom(ctx, ko.OIDCProvider)
		if err != nil {
			return nil, fmt.Errorf("getting provider: %w", err)
		}
		idToken, err = provider.Provide(ctx, "sigstore")
	} else {
		idToken, err = providers.Provide(ctx, "sigstore")
	}
	if err != nil {
		return nil, fmt.Errorf("fetching ambient OIDC credentials: %w", err)
	}
  }
}
```
In our case the token is already there I think, but I'm not exactly sure how
this works yet. 
What I think is happening is that since we are passing in the OIDCIssuer, this
is getting looked up from the context using the `providers.ProvideFrom` called
and then the `providers.Provide` function will be invoked.
https://github.com/sigstore/cosign/blob/main/pkg/providers/github/github.go#L54

https://github.com/sigstore/cosign/blob/main/cmd/cosign/cli/fulcio/fulcio.go#L61
```go
func getCertForOauthID(priv *ecdsa.PrivateKey, fc api.LegacyClient, connector oidcConnector, oidcIssuer, oidcClientID, oidcClientSecret, oidcRedirectURL string) (*api.CertificateResponse, error) {
  ...
  tok, err := connector.OIDConnect(oidcIssuer, oidcClientID, oidcClientSecret, oidcRedirectURL)
}
```
https://github.com/sigstore/sigstore/blob/main/pkg/oauthflow/flow.go#L84


https://github.com/sigstore/cosign/blob/main/pkg/providers/github/github.go#L54

```go
func OIDConnect(issuer, id, secret, redirectURL string, tg TokenGetter) (*OIDCIDToken, error) {
  if sg, ok := tg.(*StaticTokenGetter); ok {
    return sg.GetIDToken(nil, oauth2.Config{})
  } 

}
```
```go
// StaticTokenGetter is a token getter that works on a JWT that is already known
type StaticTokenGetter struct {
	RawToken string
}
```
https://github.com/sigstore/sigstore/blob/main/pkg/oauthflow/flow.go#L140
```go
func (stg *StaticTokenGetter) GetIDToken(_ *oidc.Provider, _ oauth2.Config) (*OIDCIDToken, error) {
	unsafeTok, err := jose.ParseSigned(stg.RawToken)
	if err != nil {
		return nil, err
	}
	// THIS LOGIC IS GENERALLY UNSAFE BUT OK HERE
	// We are only parsing the id-token passed directly to a command line tool by a user, so it is trusted locally.
	// We need to extract the email address to attach an additional signed proof to the server.
	// THE SERVER WILL DO REAL VERIFICATION HERE
	unsafePayload := unsafeTok.UnsafePayloadWithoutVerification()
	claims := claims{}
	if err := json.Unmarshal(unsafePayload, &claims); err != nil {
		return nil, err
	}

	subj, err := subjectFromClaims(claims)
	if err != nil {
		return nil, err
	}

	return &OIDCIDToken{
		RawString: stg.RawToken,
		Subject:   subj,
	}, nil
}

```

### ITE 10
This section contains notes related to [ITE-10] from a policy engine
perspective and mainly me trying to understand ITE-10.

Currently `steps`, and `inspect` elements can contains rules, for example:
```
  "steps": [
      {
        "_type": "step",
        "threshold": 1,
        "name": "clone-project",
        "expected_materials": [],
        "expected_products": [
          [
            "CREATE",
            "source-distributed"
          ],
          ...
```
Here, `expected_products` contains `CREATE` which is a rule that states that
the directory source-distributed should be created by the command (not shown
above). 

The proposal is that the following schema be used for step declarations to
verify predicates (signed attestations about the subject), for example:
```
name: string
command: string
predicates: [TypeURI]
expectedMaterials: [ConstraintType]
expectedProducts: [ConstraintType]
expectedAttributes: [ConstraintType]
functionaries: [string]
threshold: int

ConstraintType:
language: string  // Rego/CUE/CEL/Dogma(Seedwing)
rule: {}
```

As an example, lets say we have a step that generates a SPDX document for the
subject/artifact in a project. We could then have a step that looked something
like this:
```json
"steps": [
  {
    "name": "generate SPDX",
    "predicates": ["https://spdx.dev/Document/v2.3"],
    "expectedProducts": [ { "language": "dogma", "rule": "spdx::compatible<["OSL-2.0", "GPL-2.0"}],
    "functionaries": "dan",
    "threshold": 1
  }
],
```
Here we want to verify that the SPDX document produced using the rule specified
in `expectedProducts`. The rule is specified using Dogma which is the rule
language of the Seedwing policy engine.

So I think that in-toto would have integration points with the policy engines
(OPA/CUE/CEL?) to be able to write rules in the languages that they support?

[ITE-6]: https://github.com/in-toto/ITE/blob/4a73086a33eea8262e26070238318fd0e39900d0/ITE/6/README.md
[ITE-10]: https://github.com/in-toto/ITE/blob/4a73086a33eea8262e26070238318fd0e39900d0/ITE/10/README.adoc
[attestation validation]: https://github.com/in-toto/attestation/blob/main/docs/validation.md

