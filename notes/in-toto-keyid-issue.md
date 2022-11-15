## in-toto ecdsa keyid issue
in-toto has support for generating and using ecdsa keys in its command line tool
suite. Currently, these command line tools take ecdsa keys in securesystemslib
json format. When generating keys with other tools like `openssl` or `cosign`
the generated keys are often in PEM format and need to be converted into the
securesystemslib json format expected by the in-toto command line tools.

in-toto does not provide any command line tools, or options to existing tools as
far as I'm aware of, and I resorted to creating a python script that does this
conversion. While writing this conversion I ran into an issue which took me
some time to figure out which I'll explain in this document.

The python script uses the module `securesystemslib.keys` which has the following
functions to import ecdsa pem keys:
```python
def import_ecdsakey_from_private_pem(
    pem, scheme="ecdsa-sha2-nistp256", password=None)

def import_ecdsakey_from_public_pem(pem, scheme="ecdsa-sha2-nistp256")
```
So I was thinking that I could read an ecdsa private key in pem format from
disk and then pass the contents of it to `import_ecdsakey_from_private_pem`
which will create a python dictionary which can then be saved to disk as json.
That part worked fine but I ran into an issue when trying to use
`import_ecdsakey_from_public_pem` in the same manner. The call itself worked
and storing the json was not an issue, the problem is the `keyid` generate by
the call to `import_ecdsakey_from_public_pem` will be incorrect.

### Reproducer
First we generate a keypair:
```console
$ in-toto-keygen -t ecdsa danbev
```
Note that the format of private and public key files will be in
a securesystemslib json format.

The private key looks like this:
```console
$ cat danbev | jq
{
  "keytype": "ecdsa",
  "scheme": "ecdsa-sha2-nistp256",
  "keyid": "9a49882710aa56d1cdd6a37aabe132e8d5b74b83c000cbd563b5d72b1f5d0963",
  "keyval": {
    "public": "-----BEGIN PUBLIC KEY-----\nMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEOZ7ew2Q6e/fXJvAdJaEFotgrOAYd\nFS4qe3rtckqk/ChU7ZDRRLoWM2zV7uZZlETaW25ikyCY1Bq2UlvuC9XuQA==\n-----END PUBLIC KEY-----\n",
    "private": "-----BEGIN EC PRIVATE KEY-----\nMHcCAQEEIFlWsHuzueMjwx8kfND2lXTLZFH7XexX8gkXbwB49CznoAoGCCqGSM49\nAwEHoUQDQgAEOZ7ew2Q6e/fXJvAdJaEFotgrOAYdFS4qe3rtckqk/ChU7ZDRRLoW\nM2zV7uZZlETaW25ikyCY1Bq2UlvuC9XuQA==\n-----END EC PRIVATE KEY-----\n"
  },
  "keyid_hash_algorithms": [
    "sha256",
    "sha512"
  ]
}
```

And the public key looks like this:
```console
$ cat danbev.pub | jq
{
  "keytype": "ecdsa",
  "scheme": "ecdsa-sha2-nistp256",
  "keyid_hash_algorithms": [
    "sha256",
    "sha512"
  ],
  "keyval": {
    "public": "-----BEGIN PUBLIC KEY-----\nMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEOZ7ew2Q6e/fXJvAdJaEFotgrOAYd\nFS4qe3rtckqk/ChU7ZDRRLoWM2zV7uZZlETaW25ikyCY1Bq2UlvuC9XuQA==\n-----END PUBLIC KEY-----\n"
  }
}
```
Notice that the public json does not have a `keyid`. More about this later.

Now, if I create an `ecdsa` keypair using `cosign` the generated keys will be
in pem format, and then convert the private key to securesystemslib json I get:

```console
>>> private_key_pem = '''-----BEGIN PRIVATE KEY-----
... MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgTQ+q1RXtxOInY5/u
... d5khNI3ielgyPDqy+P4a7StgB3ShRANCAASZWPjkb22l3TrloTgDJowf64zzXOs0
... QF/5Ka87JEFAf7az2oHddwPH9skLz2qD0Yl2OvjA5CrQ3fxolkej8sDF
... -----END PRIVATE KEY-----'''
>>> import securesystemslib.keys

>>> private_key = securesystemslib.keys.import_ecdsakey_from_private_pem(private_key_pem)
>>> private_key
{'keytype': 'ecdsa', 'scheme': 'ecdsa-sha2-nistp256', 'keyid': '97916c88905c5cb257b2023fee6e11d13e94b1cf2489838fd8ae3c78e9ef8af2', 'keyval': {'public': '-----BEGIN PUBLIC KEY-----\nMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEmVj45G9tpd065aE4AyaMH+uM81zr\nNEBf+SmvOyRBQH+2s9qB3XcDx/bJC89qg9GJdjr4wOQq0N38aJZHo/LAxQ==\n-----END PUBLIC KEY-----\n', 'private': '-----BEGIN EC PRIVATE KEY-----\nMHcCAQEEIE0PqtUV7cTiJ2Of7neZITSN4npYMjw6svj+Gu0rYAd0oAoGCCqGSM49\nAwEHoUQDQgAEmVj45G9tpd065aE4AyaMH+uM81zrNEBf+SmvOyRBQH+2s9qB3XcD\nx/bJC89qg9GJdjr4wOQq0N38aJZHo/LAxQ==\n-----END EC PRIVATE KEY-----\n'}, 'keyid_hash_algorithms': ['sha256', 'sha512']}
```

```console
$ ./private_key_to_securesystemlib_json.py cosign.key | jq
{
  "keytype": "ecdsa",
  "scheme": "ecdsa-sha2-nistp256",
  "keyid": "a088cd15e5dd0dfc2fa22672847d920207e0ee933d3bdd7e485d9baca4c421ce",
  "keyval": {
    "public": "-----BEGIN PUBLIC KEY-----\nMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEWpgkrysOUvDRqEQd9dKjlYH5IffK\nht9HIZG4LGiG7y4gD/Xf8mCUq15HZ6pmQzKOPOr1Vba8jc51UuI3bMZcTw==\n-----END PUBLIC KEY-----\n",
    "private": "-----BEGIN EC PRIVATE KEY-----\nMHcCAQEEINHoTHISFG6h88/CR7dmfOOT+iWh7/+p0QVWBmFHpnILoAoGCCqGSM49\nAwEHoUQDQgAEWpgkrysOUvDRqEQd9dKjlYH5IffKht9HIZG4LGiG7y4gD/Xf8mCU\nq15HZ6pmQzKOPOr1Vba8jc51UuI3bMZcTw==\n-----END EC PRIVATE KEY-----\n"
  },
  "keyid_hash_algorithms": [
    "sha256",
    "sha512"
  ]
}
```
And the public key looks like this:

```console
{
  "keytype": "ecdsa",
  "scheme": "ecdsa-sha2-nistp256",
  "keyid": "adb2e660a546311719f5f0aaa62032c95a61f1e826690b83c2003b5895a862f2",
  "keyval": {
    "public": "-----BEGIN PUBLIC KEY-----\\nMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEw20TD96o+hpBJ3dbw3ACr8ECJb7t\\nqkZjL7IpG0gEKI9Ky7fvK6rspL376/u9BizKPtnwcvsyE/LyTAVfyXHeNg==\\n-----END PUBLIC KEY-----",
    "private": ""
  },
  "keyid_hash_algorithms": [
    "sha256",
    "sha512"
  ]
}
```
The issue here is with the `keyid` which is generated using the public key pem.
When the private key is generated this field may contain a final new line which
is included in the hash/digest, which becomes the keyid. But when the public
key is generated any final new line is striped, which causes a different
hash/digest to be generated and different keyid's are the result.

[#453](https://github.com/secure-systems-lab/securesystemslib/pull/453)
[#457](https://github.com/secure-systems-lab/securesystemslib/pull/457)
