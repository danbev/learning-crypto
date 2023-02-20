### Open Policy Agent


### Install
```console
$ curl -L -o opa https://github.com/open-policy-agent/opa/releases/tag/v0.45.0/opa_linux_amd64
$ chmod 744 opa
```

### Rego
Is the policy language used in OPA which was inspired by Datalog which it
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
And we can add properties to it using:
```console
> data
{
  "example": {
    "nr": 18
  }
}
```
And we can also add an object:
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

```console
> import future.keywords

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

### OPA eval
This allows us to evaluate a rule.

The following example will return true if the rule specified, which in this
case is `allow_if_has_fletch`, is true:
```console
$ ../opa eval --data policy.rego 'data.example.allow_if_has_fletch' --input input.txt 
{
  "result": [
    {
      "expressions": [
        {
          "value": true,
          "text": "data.example.allow_if_has_fletch",
          "location": {
            "row": 1,
            "col": 1
          }
        }
      ]
    }
  ]
}
```
But rules don't have to only return boolean values, they can return pretty much
anything. 

For example, here is a rule named `get_names` which returns a string:
```console
$ ../opa eval --data policy.rego 'data.example.get_names' --input input.json 
{
  "result": [
    {
      "expressions": [
        {
          "value": "Fletch",
          "text": "data.example.get_names",
          "location": {
            "row": 1,
            "col": 1
          }
        }
      ]
    }
  ]
}
```
The output can also be shown in `raw` format:
```console
$ ../opa eval --format raw --data policy.rego 'data.example.get_names' --input input.txt
Fletch
```

### Input vs Data
I was a little confused about input vs data. But data is simply a json that can
be used in a policy rules file from an external source, similar to input. These
objects are exposed via the global `data` variable, similar to the global `input`
variable.

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
$ cd opa-example
$ ./opa run --server policy.rego
{"addrs":[":8181"],"diagnostic-addrs":[],"level":"info","msg":"Initializing server.","time":"2022-10-11T12:27:25+02:00"}
```

We can retreive a policy by using a GET request:
```console
$ curl --silent http://localhost:8181/v1/policies/policy.rego?pretty=true
```

And we can evaluate a policy, similar to what we did above with the `opa eval`
command, using the following curl command:
```console
$ curl -d '{"input": [{"name": "Fletch"}, {"name": "DrRosen"}, {"name": "MrSinilinden"}]}' -H "Content-Type: application/json" -X POST http://localhost:8181/v1/data/example/get_names
{"result":"Fletch"}
```

### Building as wasm
```console
$ ../opa build -t wasm -e example/hello policy.rego 

$ tar tvf bundle.tar.gz 
tar: Removing leading `/' from member names
-rw------- 0/0               3 1970-01-01 01:00 /data.json
-rw------- 0/0             281 1970-01-01 01:00 /policy.rego
-rw------- 0/0          131583 1970-01-01 01:00 /policy.wasm
-rw------- 0/0              93 1970-01-01 01:00 /.manifest
```
The `data.json` file in this case will just be an empty json object. The file
`policy.rego` is our rules file. The `.manifest` contains the following:
```console
{"revision":"","roots":[""],"wasm":[{"entrypoint":"example/hello","module":"/policy.wasm"}]}
```
The .wasm file is the compiled rego policy as a wasm (module).

