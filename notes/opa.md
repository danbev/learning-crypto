### Open Policy Agent


### Install
```console
$ curl -L -o opa https://github.com/open-policy-agent/opa/releases/tag/v0.45.0/opa_linux_amd64
$ chmod 744 opa
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
> data
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

### Running tests on policies
In the [opa-example](./opa-example) director there is an a policy file named
[opa-data.rego](./opa-example/opa-data.rego), and the test is in
[test-opa-data.rego](./opa-example/test-opa-data.rego). We can run the test
using: 
```console
$ cd opa-example
$ ../opa test . -v
test-opa-data.rego:
data.example.test_has_fletch: PASS (1.23484ms)
--------------------------------------------------------------------------------
PASS: 1/1
```

### Running as a server
```console
$ ./opa run --server opa-example/opa-data.rego
{"addrs":[":8181"],"diagnostic-addrs":[],"level":"info","msg":"Initializing server.","time":"2022-10-11T12:27:25+02:00"}
```

We can retreive a policy by using a GET request:
```console
$ curl --silent http://localhost:8181/v1/policies/opa-example/opa-data.rego?pretty=true
```

