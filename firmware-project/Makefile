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

download-image: pull
	podman save -o firmware-project.tar localhost/firmware-project:latest 

push-single:
	oras push -v localhost:5000/firmware-project-single:latest \
	       firmware.bin:application/octet-stream

fetch-single:
	oras manifest fetch localhost:5000/firmware-project-single:latest | jq

attach-bundle:
	oras attach localhost:5000/firmware-project-single:latest \
	       --artifact-type=application/json firmware.bundle

pull-single: 
	@${RM} -r pulled-images
	@mkdir pulled-images
	oras pull -o pulled-images localhost:5000/firmware-project-single:latest
	@echo -e "\nPulled image to pulled-images/"
	@ls pulled-images

pull-attachment:
	@${RM} -r pulled-images
	@mkdir pulled-images
	oras pull -o pulled-images localhost:5000/firmware-project-single@sha256:aefd8d54134812098c8662a6ee971d3d70f6ed9708e3efb2ef7fb268b2530a4d
	@echo -e "\nPulled images to pulled-images/"
	@ls pulled-images
	@echo -e "\nVerify the binary using the bundle"
	env COSIGN_EXPERIMENTAL=1 cosign verify-blob \
		--bundle=pulled-images/firmware.bundle \
		pulled-images/firmware.bin




.PHONY: clean
clean: 
	${RM} firmware.crt firmware.sig firmware.bundle firmware.json

start-registry:
	podman run -d -p 5000:5000 --restart always --name registry registry:2
