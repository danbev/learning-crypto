## JSON Web Signatures

### Structure
The JWT contains 3 sections, a header, a payload, and a signature. The 
header and the payload are base64 encodes. 
```
base64(header).base64(payload).signature
```
So one can parse the header by taking a slice of the serialized string up to
the first dot.

### Header
The header contains 2 parts:
1) The signing alrgorithm that is being used  
2) The type of the token.

### Payload
Contains the claims of the JSON Object.

Claim fields:
`iss`: is the issuer of the token  .
`azp`: Authorized party. the party to which the ID token was issued.  
`aud`: Audience, the recipient this JWT is intended for.  
`sub`: The principal that is the subject of this claim.  
`iat`: Issued at is the time this JWT was issued.  
`jti`: JWT ID is a unique identifier for this JWT.  


### Signature
A string with the digest over the payload. To the payload was passed into the
hashing function, with a key and the outcome was this digest.



