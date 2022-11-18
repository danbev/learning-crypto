image_name="firmware-project"

firmware.json: create-metadata.sh firmware.bin
	./create-metadata.sh firmware.bin

image: firmware.json
	podman build -t ${image_name} -f Dockerfile .

sign:
	env COSIGN_EXPERIMENTAL=1 cosign sign-blob  \
		--bundle=firmware.bundle \
	       	--output-certificate=firmware.crt \
		--output-signature=firmware.sig firmware.bin

verify:
	env COSIGN_EXPERIMENTAL=1 cosign verify-blob --bundle=firmware.bundle \
		firmware.bin

push: 
	oras push -v localhost:5000/firmware-project:latest \
	       firmware.bin:application/octet-stream \
	       firmware.json:application/octet-stream \
	       firmware.bundle:application/json

fetch:
	oras manifest fetch localhost:5000/firmware-project:latest | jq

pull: 
	@mkdir -p pulled-images
	oras pull -o pulled-images localhost:5000/firmware-project:latest
	@echo -e "\nPulled images to pulled-images/"
	@ls pulled-images

download-image: oras-pull
	podman save -o firmware-project.tar localhost/firmware-project:latest 


.PHONY: clean
clean: 
	${RM} firmware.crt firmware.sig firmware.bundle firmware.json

start-registry:
	podman run -d -p 5000:5000 --restart always --name registry registry:2
