const fs = require('fs');
const crypto = require('crypto');
const pub = crypto.createPublicKey(fs.readFileSync('test.pub'));
const input = fs.readFileSync('input.txt');
const signature = fs.readFileSync('signature.bin');
console.log('verify successful: ', crypto.verify(null, input, pub, signature));
