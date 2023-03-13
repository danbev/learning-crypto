### StoneSoup SSCS
The purpose of this document is to understand and hopefully be able to run
a pipline/task where the result of the tasks are signed and have attestations.
The predicates in the attestations will then be validated agains the enterprise
policy contract (EPC) rules defined by StoneSoup.

StoneSoup's [Pipeline service] utilizes [Tekton Chains] to perform signing and
attestation of TaskRuns: 
```
* Signing and attestation of TaskRuns with Tekton Chains
```
I've created an [example] using Tekton chains which might be useful as a
reference

### Prerequisites
First we need a create a cluster to work with:
```console
$ kind create cluster
```

### Installing Enterprise Contract Policy (ECP) controller
Install the ECP Custom Resource Definitions (CRD):
```console
$ git clone git@github.com:hacbs-contract/enterprise-contract-controller.git
$ cd enterprise-contract-controller
$ make install
go run -modfile /home/danielbevenius/work/security/hacbs/enterprise-contract-controller/tools/go.mod sigs.k8s.io/kustomize/kustomize/v4 build config/crd | kubectl apply -f -
customresourcedefinition.apiextensions.k8s.io/enterprisecontractpolicies.appstudio.redhat.com configured
```
### Deploy the ECP sample
```console
$ cd stonestoup
$ kubectl apply -f appstudio.redhat.com_v1alpha1_enterprisecontractpolicy.yaml 
enterprisecontractpolicy.appstudio.redhat.com/enterprisecontractpolicy-sample configured
```

Verify that it worked correctly:
```console
$ kubectl get ecp
NAME                              AGE
enterprisecontractpolicy-sample   1s
```

We can also inspect the resource using the following command:
```console
$ kubectl describe ecp enterprisecontractpolicy-sample
Name:         enterprisecontractpolicy-sample
Namespace:    default
Labels:       <none>
Annotations:  <none>
API Version:  appstudio.redhat.com/v1alpha1
Kind:         EnterpriseContractPolicy
Metadata:
  Creation Timestamp:  2023-03-12T12:49:15Z
  Generation:          1
  Managed Fields:
    API Version:  appstudio.redhat.com/v1alpha1
    Fields Type:  FieldsV1
    fieldsV1:
      f:metadata:
        f:annotations:
          .:
          f:kubectl.kubernetes.io/last-applied-configuration:
      f:spec:
        .:
        f:configuration:
          .:
          f:collections:
            .:
            v:"salsa_one_collection":
          f:exclude:
            .:
            v:"not_useful":
            v:"test:conftest-clair":
          f:include:
            .:
            v:"always_checked":
        f:description:
        f:exceptions:
          .:
          f:nonBlocking:
            .:
            v:"not_useful":
            v:"test:conftest-clair":
        f:sources:
    Manager:         kubectl-client-side-apply
    Operation:       Update
    Time:            2023-03-12T12:49:15Z
  Resource Version:  291910
  UID:               010d0f7a-0fd0-4daf-ba4e-4740103918f9
Spec:
  Configuration:
    Collections:
      salsa_one_collection
    Exclude:
      not_useful
      test:conftest-clair
    Include:
      always_checked
  Description:  My custom enterprise contract policy configuration
  Exceptions:
    Non Blocking:
      not_useful
      test:conftest-clair
  Sources:
    Name:  Policies object
    Policy:
      quay.io/hacbs-contract/ec-release-policy:latest
Events:  <none
```

__wip__

[pipeline service]: https://redhat-appstudio.github.io/book/book/pipeline-service.html
[tekton chains]: https://tekton.dev/docs/chains/
[example]: https://github.com/danbev/learning-tekton#tekton-chains
