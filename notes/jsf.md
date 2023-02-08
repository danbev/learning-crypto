## JSON Signature Format (JSF)
When using JSON Web Signature we need to make sure that the a JSON object is
serialized/deserialized in the same way, otherwise using them to in crypto
can lead to different results. For example, a hash will be different if there
are differences in the serialized format. So often libraries will have a
conanoicaliztion function before hashing/encrypting, and likewise have the same
when hasing/decrypting/verifying. On issue is that JSON does not specify that
the order of elements be preserved, so it could be possible for one library to
serialize/deserialize but with a different order.  With the introduction of
ECMAScript V6 the order of elements must be respected.
