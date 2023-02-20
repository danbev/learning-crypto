# HACBS policies
This document will take a look at the Hybrid Application Cloud Build Services
(HACBS) [policies], and look at their usages of the Rego rule language usage.

The goal is to make sure that the policy rules could be written in the Dogma
language, and if not open issues for functionality that may be missing.

## Working branch
[hacbs-policy-translation](https://github.com/danbev/seedwing-policy/tree/hacbs-policy-translation)

## HACBS Policy Rules
The HACBS policy rules can be found in [policy] which contains the following
directories:
* pipeline
* release
* lib

### pipeline

#### [basic.rego](https://github.com/hacbs-contract/ec-policies/blob/main/policy/pipeline/basic.rego)
Looking at the comment in this file it does not seem to be a policy rule that
is expected to be run by an external consumer:
```
# (Not sure if we need this, but I'm using it to test the docs build.)
```

#### [required_tasks.rego](https://github.com/hacbs-contract/ec-policies/blob/main/policy/pipeline/required_tasks.rego)
This rule file contains polices related to Tekton pipelines.

The first policy rule is tested by `tasks.test_required_tasks_met`
```console
$ opa test ./data/rule_data.yml ./policy checks -v -r data.policy.pipeline.required_tasks.test_required_tasks_met
policy/pipeline/required_tasks_test.rego:
data.policy.pipeline.required_tasks.test_required_tasks_met: PASS (9.46248ms)
--------------------------------------------------------------------------------
PASS: 1/1
```
Keep in mind tests don't call a specific rule to be evaulated, instead all the
rules in the policy file, policy/pipeline/required_tasks.rego above, will be
evaluated.

So this should verify that the input json document contains at least on task
element in `tasks` array.
```console
$ cargo r -q --bin seedwing-policy-cli -- \
    --data ./hacbs/data \
     --policy ./hacbs/pipeline/required_tasks.dog \
     eval --input ./hacbs/test/input/pipeline/required_tasks_no_tasks.json \
     --name required_tasks::at-least-one-task
evaluate pattern: required_tasks::at-least-one-task
Type: required_tasks::at-least-one-task
Satisfied: false
Value:
  kind: <<string>>
  metadata: <<object>>
  spec: <<object>>
Rationale:
  ...

pattern match failed
```
And testing with input that does have entries in the `tasks` array:
```console
$ cargo r -q --bin seedwing-policy-cli -- \
    --data ./hacbs/data \
     --policy ./hacbs/pipeline/required_tasks.dog \
     eval --input ./hacbs/test/input/pipeline/required_tasks.json \
     --name required_tasks::at-least-one-task

evaluate pattern: required_tasks::at-least-one-task
Type: required_tasks::at-least-one-task
Satisfied: true
Value:
  kind: <<string>>
  metadata: <<object>>
  spec: <<object>>
Rationale:
  ...

ok!
```
While running test this way works it will mean that there will be alot of
different input files for different tests. Writing the test in Rust would avoid
this I'm gettin a feeling that that might be the prefered option.

__wip__

[policies]: https://github.com/hacbs-contract/ec-policies/
[policy]: https://github.com/hacbs-contract/ec-policies/tree/main/policy
[rego-builtin-functions]: https://www.openpolicyagent.org/docs/latest/policy-reference/#built-in-functions
rego
