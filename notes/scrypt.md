## Scrypt Password Based Key Derivative Function (PBKDF)
Is a password based key derivation function whic is defined in
[RFC-7914](https://www.rfc-editor.org/rfc/rfc7914.html).

This key derivation function is menory-intensive to prevent GPU, ASIC, and FPGA
attacks.

## Key Derivation Function
This function will take a password, a salt, a number of interations and it will
produce a password key.
Example:
```
key = scrypt(password, salt, N, R, P, derived-key-len)
```

## Script PBKDF
* passphrase
* salt
* N interation counts, which affects CPU cost parameter
* R memory cost parameter
* P parallelization parameter
* derived-key-len the length of the output key
