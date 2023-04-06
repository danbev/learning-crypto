IMAGE_NAME="ttl.sh/danbev-spdx-example-container:2h"

build-image:
	podman build -t ${IMAGE_NAME} .

push:
	podman push ${IMAGE_NAME}

genkeys:
	cosign generate-key-pair

sign:
	cosign sign -y --key cosign.key ${IMAGE_NAME}

spdx:
	../spdx-sbom-generator

spdx-json:
	../spdx-sbom-generator -f json

attest:
	cosign attest -y --no-upload --key cosign.key --type spdx --predicate bom-cargo.spdx ${IMAGE_NAME}

attest-json:
	cosign attest -y --no-upload --key cosign.key --type spdx --predicate bom-cargo.json ${IMAGE_NAME}

triangulate:
	cosign triangulate ${IMAGE_NAME}

