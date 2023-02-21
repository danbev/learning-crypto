# HACBS policies
This document will take a look at the Hybrid Application Cloud Build Services
(HACBS) [policies], and look at their usages of the Rego rule language usage.

The goal is to make sure that the policy rules could be written in the Dogma
language, and if not open issues for functionality that may be missing.

## Working repository
[hacbs-dogma-policies](https://github.com/danbev/hacbs-dogma-policies)

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
$ cargo t -- --show-output at_least_one_task
   Compiling hacbs-dogma-policies v0.1.0 (/home/danielbevenius/work/security/seedwing/hacbs-dogma-policies)
    Finished test [unoptimized + debuginfo] target(s) in 7.42s
     Running tests/tests.rs (target/debug/deps/tests-16c82c8c48b6735c)

running 2 tests
test pipeline::required_tasks::at_least_one_task ... ok
test pipeline::required_tasks::at_least_one_task_no_tasks ... ok

successes:

successes:
    pipeline::required_tasks::at_least_one_task
    pipeline::required_tasks::at_least_one_task_no_tasks

test result: ok. 2 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.19s
```

__wip__

[policies]: https://github.com/hacbs-contract/ec-policies/
[policy]: https://github.com/hacbs-contract/ec-policies/tree/main/policy
[rego-builtin-functions]: https://www.openpolicyagent.org/docs/latest/policy-reference/#built-in-functions
