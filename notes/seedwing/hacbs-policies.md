# HACBS policies
This document will take a look at the Hybrid Application Cloud Build Services
(HACBS) [policies], and look at their usages of the Rego rule language usage.
The goal is to identify any missing parts in the Dogma language.

## Rules
The HACBS policy rules can be found in [policy] which contains the following
directories:
* pipeline
* release
* lib


### pipeline

#### [basic.rego](https://github.com/hacbs-contract/ec-policies/blob/main/policy/pipeline/basic.rego)
To run this test for this rule:
```console
$ opa test ./data/rule_data.yml ./policy checks -v -r data.policy.pipeline.basic.test_unexpected_kind
policy/pipeline/basic_test.rego:
data.policy.pipeline.basic.test_unexpected_kind: PASS (996.993µs)
--------------------------------------------------------------------------------
PASS: 1/1
```
This rules uses [rego.metadata.chain](https://www.openpolicyagent.org/docs/latest/policy-reference/#builtin-rego-regometadatachain) which I've not encontered before.
This will return a "chain" of metadata for the current rule.

the rule in question looks like this (with the addition of printing the
chain added):
```
#                                                                               
# METADATA                                                                       
# title: Pipeline definition sanity checks                                       
# description: |-                                                                
#   Currently there is just a check to confirm the input                         
#   appears to be a Pipeline definition. We may add additional                   
#   sanity checks in future.                                                     
#
package policy.pipeline.basic                                                      
                                                                                     
import future.keywords.contains                                                    
import future.keywords.if                                                          
                                                                                     
import data.lib                                                                    
                                                                                     
expected_kind := "Pipeline"                                                        
                                                                                     
# METADATA                                                                      
# title: Input data has unexpected kind                                         
# description: |-                                                               
#   A sanity check to confirm the input data has the kind "Pipeline"            
# custom:                                                                       
#   short_name: unexpected_kind                                                 
#   failure_msg: Unexpected kind '%s'                                           
# 
deny contains result if {                                                          
        print(rego.metadata.chain())                                               
        expected_kind != input.kind                                                
        result := lib.result_helper(rego.metadata.chain(), [input.kind])           
  }                    
```
Which will produce:
```console
[
  {
    "annotations": {
      "custom": {
        "failure_msg": "Unexpected kind '%s'",
        "short_name": "unexpected_kind"
      },
      "description": "A sanity check to confirm the input data has the kind \"Pipeline\"",
      "scope": "rule",
      "title": "Input data has unexpected kind"
    },
    "path": [
      "policy",
      "pipeline",
      "basic",
      "deny"
    ]
  },
  {
    "annotations": {
      "description": "Currently there is just a check to confirm the input\nappears to be a Pipeline definition. We may add additional\nsanity checks in future.",
      "scope": "package",
      "title": "Pipeline definition sanity checks"
    },
    "path": [
      "policy",
      "pipeline",
      "basic"
    ]
  }
]
```
Notice that the values are taken from the `METADATA` "comments" in the rule
file.
Lets take a look at the `result` as well (again by printing it):
```console
{"code": "basic.unexpected_kind", "effective_on": "2022-01-01T00:00:00Z", "msg": "Unexpected kind 'Foo'"}
```
Lets take a closer look at `lib.result_helper` which is called by this rule:
```
        result := lib.result_helper(rego.metadata.chain(), [input.kind])           
```
This can be found in `policy/lib/result_helper.rego`:
```
package lib                                                                        
                                                                                   
import data.lib.time as time_lib                                                   
                                                                                   
result_helper(chain, failure_sprintf_params) := result {                           
        with_collections := {"collections": _rule_annotations(chain).custom.collections}
        result := object.union(_basic_result(chain, failure_sprintf_params), with_collections)
} else := result {                                                                 
        result := _basic_result(chain, failure_sprintf_params)                     
}
```
Notice that `with_collections` is a rule and will be false in this case so the
else block will be executed.

* Should/Does Dogma support this kind of metadata?

__wip__

[policies]: https://github.com/hacbs-contract/ec-policies/
[policy]: https://github.com/hacbs-contract/ec-policies/tree/main/policy
[rego-builtin-functions]: https://www.openpolicyagent.org/docs/latest/policy-reference/#built-in-functions
rego
