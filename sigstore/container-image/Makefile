IMAGE_NAME="ttl.sh/danbev-simple-container:2h"

container-image:
	podman build -t ${IMAGE_NAME} .

push:
	podman push ${IMAGE_NAME}

keys:
	cosign generate-key-pair

sign:
	cosign sign --key cosign.key ${IMAGE_NAME}

triangulate:
	cosign triangulate ${IMAGE_NAME}

digest:
	crane digest ${IMAGE_NAME}

digest-manifest:
	crane manifest "ttl.sh/danbev-simple-container:2h" | sha256sum

manifest:
	crane manifest ${IMAGE_NAME} | jq

manifest-raw:
	crane manifest ${IMAGE_NAME} | jq -R
