#!/usr/bin/env bash

ARTIFACT=$1

REVISION=$(git rev-parse --short HEAD | tr -d '\n')
SZ=$(du -b ${ARTIFACT} | cut -f1)
CHECKSUM=$(sha256sum ${ARTIFACT} | awk '{ print $1 }')

cat<<EOF > firmware.json
{
  "version": "${REVISION}",
  "size": ${SZ},
  "checksum": "${CHECKSUM}"
}
EOF

echo "Generated metadata:"
cat firmware.json
