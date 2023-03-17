import jwkToPem from 'jwk-to-pem';

process.stdin.resume();
process.stdin.setEncoding('utf8');
process.stdin.on('data', function(data) {
  let cert = jwkToPem(JSON.parse(data));
  process.stdout.write(cert);
});

