### Open Policy Agent


### Install
```console
$ curl -L -o opa https://github.com/open-policy-agent/opa/releases/tag/v0.45.0/opa_linux_amd64
$ chmod 744 opa
$ ./opa version
Version: 0.11.0
Build Commit: e7a34319
Build Timestamp: 2019-05-21T06:30:29Z
Build Hostname: e4b849da0bbb
```

### Rego
Is the policy language used in OPA which was inspired by Datalog which is
extendes to operate on json.

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
> something := { "one": 1, "two": 2}
Rule 'something' re-defined in package example. Type 'show' to see rules.
> data
{
  "example": {
    "something": {
      "one": 1,
      "two": 2
    }
  }
}
> something
{
  "one": 1,
  "two": 2
}
> something.one
1
> something.two
2
> 
```

Arrays:
```console
> people := [{"name": "Fletch"}, {"name": "DrRosen"}, {"name": "MrSinilinden"}]
```
```console
> people[i]
+---+-------------------------+
| i |        people[i]        |
+---+-------------------------+
| 0 | {"name":"Fletch"}       |
| 1 | {"name":"DrRosen"}      |
| 2 | {"name":"MrSinilinden"} |
+---+-------------------------+
```
```console> import future.keywords
> q contains name if {
|  some p in people
|  name := p.name
| }
| 
Rule 'q' defined in package repl. Type 'show' to see rules.
> q
[
  "DrRosen",
  "Fletch",
  "MrSinilinden"
]
```


