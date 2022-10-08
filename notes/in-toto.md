## in-toto
[https://in-toto.io/](in-toto) is a system/specification for securing how
software is developed with the motivation to prevent software supply chain
attacks. This done by being able to verify the complete development process.

A client is someone how install a software product and they want to be able to
be sure that no steps were changed, removed, or added. And they want to
verify that the software comes from the designated product owner.

`in-toto` means the whole which is from latin and in this context refers to it
taking the whole process into account (I think).

### Installation
```console
$ pip install in-toto
```
This does not install a single executable but a number of them:
* in-toto-keygen
* in-toto-mock
* in-toto-record
* in-toto-run
* in-toto-sign
* in-toto-verify

### in-toto-keygen
```console
$ in-toto-keygen func-daniel.key
```
The above will generate a public/private key pair in two different files:
```console
$ ls func-daniel.key*
func-daniel.key  func-daniel.key.pub
```

```console
$ cd in-toto-example/

$ in-toto-mock --name vcs-1 -- git clone https://github.com/danbev/learning-crypto
Running 'vcs-1'...
Recording materials '.'...
Running command 'git clone https://github.com/danbev/learning-crypto'...
Cloning into 'learning-crypto'...
Recording products '.'...
Creating link metadata...
Storing unsigned link metadata to 'vcs-1.link'...
```

Building:
```console
$ in-toto-mock --name building-1 -- make -C learning-crypto all
```

Packaging:
```console
$ tar czf in_toto_link_files.tar.gz vcs-1.link building-1.link

$ tar tvf in_toto_link_files.tar.gz 
-rw-r--r-- danielbevenius/danielbevenius 5492 2022-10-08 09:19 vcs-1.link
-rw-r--r-- danielbevenius/danielbevenius 14198 2022-10-08 09:24 building-1.link
```
