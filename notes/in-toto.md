## in-toto
[https://in-toto.io/](in-toto) is a system/specification for securing how
software is developed with the motivation to prevent software supply chain
attacks. This done by being able to verify the complete development process.

A client is someone who installs a software product and they want to be able to
make sure that no steps were changed, removed, or added. And they want to
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
  Simliar to in-toto-run but creates unsigned link metadata.
* in-toto-record
* in-toto-run
* in-toto-sign
* in-toto-verify

### in-toto-mock
The format of this command looks like this:
```console
$ in-toto-mock --name ouput_file_name -- <command> [args]
```
We can run this to create unsigned link metadata. My first thought when I was
this command was that it perhaps was for testing because of having 'mock' in its
name, but it looks very useful. What it does is that it executed the command
passed in, and records all the files, and generates a link file under
output_file_name.link.
```console
$ mkdir tmp && cd tmp/
$ echo "bajja" > file1.txt
$ in-toto-mock --name testing -- ls file1.txt
Running 'testing'...
Recording materials '.'...
Running command 'ls file1.txt'...
file1.txt
Recording products '.'...
Creating link metadata...
Storing unsigned link metadata to 'testing.link'...
```
Lets take a look at `testing.link`:
```console
$ cat testing.link 
{
 "signatures": [],
 "signed": {
  "_type": "link",
  "byproducts": {
   "return-value": 0,
   "stderr": "",
   "stdout": "file1.txt\n"
  },
  "command": [
   "ls",
   "file1.txt"
  ],
  "environment": {},
  "materials": {
   "file1.txt": {
    "sha256": "311375908b2a10688fb8841b61d8b1daa9a3e904f84d2ed88d0a35cb4f0e1a95"
   }
  },
  "name": "testing",
  "products": {
   "file1.txt": {
    "sha256": "311375908b2a10688fb8841b61d8b1daa9a3e904f84d2ed88d0a35cb4f0e1a95"
   }
  }
 }
}
```
So there are now signatures here so there is no way of verifying but that task
can be performed afterwards. So we first have to have a keypair to use:
```console
$ in-toto-keygen func-test
```
And then we can use the private key to generate signatures:
```console
$ in-toto-sign -f testing.link -k func-test
```
That will create a file name 'testing.fd223b69.link' which will have a populated
`signatures` element:
```
$ cat testing.fd223b69.link 
{
 "signatures": [
  {
   "keyid": "fd223b69409d956857bfb9a26a322ea6302e7855e8b2c4c2ba377d70c4014d2b",
   "sig": "1125c8af3e836dfc2520134d9c902c4c4d2c67fbdca72d80f1cd58a6cd4c59aa3ae4a02c78f73e156065decdb1fc5503fa18d2c50a9d920c30e1878236287c15f164e3a45b1b065da6b0cc74f6282cf0661de8587ae7c5a22fc475e42a3bf398a5fbce3e20b7fb2b61d0003800dea0f3bb82f25a0db420758a1e11347a2dbe506ad21422fcc17d212d9a645b9ada55e699061680cad616f0cc10ce36c047aa773aa25d4e2fdc69ed02bf0ffeed03da1d11595cb17ecc6f9da7a4b90b183d3f4639b17293c9fdc897bee0eed5fa9fb3327603fbb6de477e71d6209f5c474d532c605fbe6f61b0030255717f6bf1fb2b56b4cf0503e4482835c8b75db909c4bc501ad2532515de1b858caec7bf485bb7cd1f9bbd82f1b896888c861fce87d6b9de1fbcecf3840cbc19547ec5402f6a946c19ac95bf9691d23ce9f43b6654f1f7cdfefc68a32f6c52ed0a6bf095994ed55705bbded42d1259d74d5ccbf60dd4342bb28818d36d46846ec9063fff1fb4417426444eae62b230a599af51ff1579aee0"
  }
 ],
 "signed": {
  "_type": "link",
  "byproducts": {
   "return-value": 0,
   "stderr": "",
   "stdout": "file1.txt\n"
  },
  "command": [
   "ls",
   "file1.txt"
  ],
  "environment": {},
  "materials": {
   "file1.txt": {
    "sha256": "311375908b2a10688fb8841b61d8b1daa9a3e904f84d2ed88d0a35cb4f0e1a95"
   }
  },
  "name": "testing",
  "products": {
   "file1.txt": {
    "sha256": "311375908b2a10688fb8841b61d8b1daa9a3e904f84d2ed88d0a35cb4f0e1a95"
   }
  }
 }
}
```
Now, since we only need are private key to sign we can actually delete it now:
```console
$ rm func-test
```
And to verify we use the same command:
```console
$ in-toto-sign -k func-test.pub -f testing.fd223b69.link --verify
```



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
