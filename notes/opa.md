### Open Policy Agent


### Install
```console
$ curl -L -o opa https://github.com/open-policy-agent/opa/releases/download/v0.11.0/opa_linux_amd64
$ chmod 744 opa
$ ./opa version
Version: 0.11.0
Build Commit: e7a34319
Build Timestamp: 2019-05-21T06:30:29Z
Build Hostname: e4b849da0bbb
```

### REPL
```console
$ ./opa run
```

When we start there will be nothing:
```console
> designated
{}
```

We can add a `package` which is like adding a field to a json object:
```console
> package example:
package example
> data
{
  "example": {}
}
```
And is if add properties to it using:
```console
> data
{
  "example": {
    "nr": 18
  }
}
```
And we can also add object:
```console
> obj = { "one":  1, "two": 2 }
> data
{
  "example": {
    "nr": 18,
    "obj": {
      "one": 1,
      "two": 2
    }
  }
}
```
