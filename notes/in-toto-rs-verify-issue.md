### in-toto-verify issue
When generating layout and steps in in-toto-rs it can be handy to verify the
Metablock's afterwards to make sure that the signatures generated can be
verified. This might sound obvious that the signatures should match as we have
just generated them, but I ran into an issue where this was not the case, the
verification failed due to the signatures not matching. 

The call to verify looks like this:
```rust
  let layout = serde_json::from_slice::<Metablock>(&layout_bytes).expect("Could not deserialize Metablock");
  let layout_keys = HashMap::from([(key_id, pub_key)]);
  in_toto_verify(&layout, layout_keys, verify_dir.to_str().unwrap(), None).expect("verify failed");
```
Notice that `layout_keys` is a `HashMap` where there key_id is the key and the
public key the value.

A `Metablock` is a struct that has two memebers:
```rust
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]                     
pub struct Metablock {                                                             
     pub signatures: Vec<Signature>,                                                
     #[serde(rename = "signed")]                                                    
     pub metadata: MetadataWrapper,                                                 
 }     
```
This same structure is used for the root layout file, and also for the link
files.

```console
$ rust-gdb --args ./target/debug/cargo-verify -d source-distributed -a sscs/in-toto/artifacts/in-toto-rs -p $PWD
Reading symbols from ./target/debug/cargo-verify...

(gdb) br metadata.rs:163
Breakpoint 2 at 0x5555558ad42c: metadata.rs:163. (5 locations)
(gdb) r
```

The first thing that `in_toto_verify` does is that it calls
`verify_layout_signatures` passing in the layout (Metablock) instance and also
the hashmap of key_id/public_key pairs.
This part passes and the signatures can be verified successfully using the
public key.
This will then return back to verfiylib.rs which will proceed to check the
following things:
```rust
// Verify layout expiration date                                            
      verify_layout_expiration(&layout)?;                                         
                                                                                     
      // Load metadata files for steps of layout                                  
      let steps_links_metadata = load_links_for_layout(&layout, link_dir)?;          
                                                                                  
      // Verify signatures and signature thresholds for steps of layout           
      let link_files = verify_link_signature_thresholds(&layout, steps_links_metadata)?;
                                                                                     
      // Verify sublayouts recursively                                             
      let link_files = verify_sublayouts(&layout, link_files, link_dir)?;         
                                                                                  
      // Verify command alignment for steps of layout (only warns)                
      verify_all_steps_command_alignment(&layout, &link_files)?;                     
                                                                                  
      // Verify threshold                                                         
      verify_threshold_constraints(&layout, &link_files)?;                        
                                                                                  
      // Reduce link files                                                        
      let mut reduced_link_files = reduce_chain_links(link_files)?;
```
The expiration check succeeds, and the loading of the links from disk also
succeeds.
Next we have the verify_link_signature_thresholds and notice that
steps_links_metadata is passed in.
```rust
  fn verify_link_signature_thresholds(                                               
      layout: &LayoutMetadata,                                                       
      steps_links_metadata: HashMap<String, HashMap<KeyId, Metablock>>,              
  ) -> Result<HashMap<String, HashMap<KeyId, Metablock>>> {                          
      let mut metadata_verified = HashMap::new();                                    
                                                                                     
      for step in &layout.steps {                                                    
+         println!("verify_link step: {:?}", &step);                                 
          // Verify this single step, return verified links.                         
          let metadata_per_step_verified = verify_link_signature_thresholds_step( 
              step,                                                                  
              steps_links_metadata                                                   
                  .get(&step.name)                                                   
                  .unwrap_or(&HashMap::new()),                                       
              &layout.keys,                                                          
          )?;                                                                        
                                                                                     
          metadata_verified.insert(step.name.clone(), metadata_per_step_verified);
      }                                                                              
                                                                                     
      Ok(metadata_verified)                                                          
  }
```
So this is going to iterator over the steps in the layout file. The first step
is:
```console
(gdb) n
verify_link step: Step {
typ: "step",
threshold: 1,
name: "clone-project",
expected_materials: [],
expected_products: [Create(VirtualTargetPath("source-distributed")),
Allow(VirtualTargetPath("source-distributed/*")),
Allow(VirtualTargetPath("source-distributed-layout.json"))],
pub_keys: [KeyId("1d5d1a28f8cad707cb1a8d4290dda905ccf03f776a4ad142a02f56304b48031a")],
expected_command: Command(["git", "clone", "git@github.com:trustification/source-distributed.git"])
}

(gdb) p step.name
$10 = "clone-project"
(gdb) p step.typ
$11 = "step"
(gdb) p step.threshold
$12 = 1


(gdb) p steps_links_metadata
$13 = HashMap(size=2) = {["clone-project"] = HashMap(size=1) = {
    [in_toto::crypto::KeyId ("1d5d1a28f8cad707cb1a8d4290dda905ccf03f776a4ad142a02f56304b48031a")] = in_toto::models::metadata::Metablock {signatures: Vec(size=1) = {
        in_toto::crypto::Signature {key_id: in_toto::crypto::KeyId ("1d5d1a28f8cad707cb1a8d4290dda905ccf03f776a4ad142a02f56304b48031a"), value: in_toto::crypto::SignatureValue (Vec(size=72) = {48, 70, 2, 33, 0, 229, 137, 224, 138, 
              1, 203, 80, 101, 174, 79, 126, 173, 19, 152, 60, 27, 106, 237, 97, 219, 202, 126, 35, 160, 26, 233, 
              66, 91, 241, 17, 198, 170, 2, 33, 0, 253, 176, 213, 100, 175, 184, 117, 82, 85, 97, 59, 202, 87, 25, 
              24, 103, 100, 220, 155, 140, 97, 218, 248, 107, 12, 205, 202, 35, 15, 156, 234
```

```rust
fn verify_link_signature_thresholds_step(                                       
      step: &Step,                                                                
      links: &HashMap<KeyId, Metablock>,                                          
      pubkeys: &HashMap<KeyId, PublicKey>,                                        
  ) -> Result<HashMap<KeyId, Metablock>> {                                        
      let mut metablocks = HashMap::new();                                        
                                                                                  
      // Get all links for the given step, verify them, and record the good          
      // links in the HashMap.                                                    
      for (signer_key_id, link_metablock) in links {                              
          if let Some(authorized_key) = pubkeys.get(signer_key_id) {              
              let authorized_key = vec![authorized_key];                          
              if link_metablock.verify(1, authorized_key).is_ok() {               
                  metablocks.insert(signer_key_id.clone(), link_metablock.clone());
              } 
          }                                                                          
      }
      ...
}
```
We can check the signer key and the authorized_key: 
```console
(gdb) p *signer_key_id
$17 = in_toto::crypto::KeyId ("1d5d1a28f8cad707cb1a8d4290dda905ccf03f776a4ad142a02f56304b48031a")
(gdb) 

(gdb) p *authorized_key
$19 = in_toto::crypto::PublicKey {typ: in_toto::crypto::KeyType::Ecdsa, key_id: in_toto::crypto::KeyId ("1d5d1a28f8cad707cb1a8d4290dda905ccf03f776a4ad142a02f56304b48031a"), scheme: in_toto::crypto::SignatureScheme::EcdsaP256Sha256, keyid_hash_algorithms: core::option::Option<alloc::vec::Vec<alloc::string::String, alloc::alloc::Global>>::Some(Vec(size=2) = {"sha256", "sha512"}), value: in_toto::crypto::PublicKeyValue (Vec(size=65) = {4, 217, 15, 81, 52, 193, 204, 
      119, 163, 173, 130, 168, 59, 104, 39, 69, 165, 236, 121, 213, 104, 164, 183, 135, 53, 155, 221, 204, 104, 
      246, 138, 210, 84, 61, 228, 123, 32, 214, 16, 61, 200, 54, 177, 29, 105, 212, 195, 225, 25, 169, 255, 223, 
      89, 114, 25, 140, 40, 63, 186, 142, 168, 24, 187, 234, 173})}

```

This is where things fail, the verification fails that is.

But, I'm able to verify the layout with the private and public key, and I'm
using the same keys for the link files. So why does the layout pass verification
but not the link files?



$ rm -rf  sscs/in-toto/artifacts/in-toto-rs/work/
$ cargo r --bin cargo-in-toto-gen -- -o trustification -r source-distributed
warning: source-distributed v0.1.0 (/home/danielbevenius/work/security/source-distributed) ignoring invalid dependency `source-distributed` which is missing a lib target
   Compiling in-toto v0.3.0 (/home/danielbevenius/work/security/in-toto-rs)
warning: unused imports: `debug`, `warn`
  --> /home/danielbevenius/work/security/in-toto-rs/./src/models/metadata.rs:17:11
   |
17 | use log::{debug, warn};
   |           ^^^^^  ^^^^
   |
   = note: `#[warn(unused_imports)]` on by default

   Compiling source-distributed v0.1.0 (/home/danielbevenius/work/security/source-distributed)
warning: `in-toto` (lib) generated 1 warning
    Finished dev [unoptimized + debuginfo] target(s) in 15.21s
     Running `target/debug/cargo-in-toto-gen -o trustification -r source-distributed`
branch: "in-toto-rs"
commit: 8fabc2d184d9e53274de9eccf0253fa36bf5d76d
Open this URL in a browser if it does not automatically open for you:
https://oauth2.sigstore.dev/auth/auth?response_type=code&client_id=sigstore&state=NN_A_6tfGIWdKxPV5_7k6A&code_challenge=vCXVEv2NgoOYBc0f8VjzB1uhPHFLr0QycDxrGtkH12I&code_challenge_method=S256&redirect_uri=http%3A%2F%2Flocalhost%3A8080&scope=openid+email&nonce=0DeL8K-yX2blaMvVU9oSTw

Generated keypair Zeroizing("-----BEGIN PRIVATE KEY-----\nMIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgkGngnAgFITGEQ4pU\nKhzRofSFYXyFh0vFJYpGUaz+c4+hRANCAAQqAKpQZutQEJZVoz/BAZevxVqv+JbD\nq7Sp3O+nwPKc1PE/ChyECu8HyDL+iWqVo6JYuG3+pF5Jb9/e7pZn25+c\n-----END PRIVATE KEY-----\n")
------------------> BEVE key_id: "dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"
private keyid: KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8")
public keyid: KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8")
in verify...raw: [123, 34, 95, 116, 121, 112, 101, 34, 58, 34, 108, 97, 121, 111, 117, 116, 34, 44, 34, 101, 120, 112, 105, 114, 101, 115, 34, 58, 34, 50, 48, 50, 52, 45, 48, 49, 45, 48, 51, 84, 49, 50, 58, 51, 55, 58, 48, 55, 90, 34, 44, 34, 105, 110, 115, 112, 101, 99, 116, 34, 58, 91, 123, 34, 95, 116, 121, 112, 101, 34, 58, 34, 105, 110, 115, 112, 101, 99, 116, 105, 111, 110, 34, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 109, 97, 116, 101, 114, 105, 97, 108, 115, 34, 58, 91, 91, 34, 77, 65, 84, 67, 72, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 42, 34, 44, 34, 87, 73, 84, 72, 34, 44, 34, 80, 82, 79, 68, 85, 67, 84, 83, 34, 44, 34, 70, 82, 79, 77, 34, 44, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 116, 97, 114, 103, 101, 116, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 99, 111, 115, 105, 103, 110, 46, 107, 101, 121, 46, 112, 117, 98, 46, 106, 115, 111, 110, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 45, 108, 97, 121, 111, 117, 116, 46, 106, 115, 111, 110, 34, 93, 44, 91, 34, 68, 73, 83, 65, 76, 76, 79, 87, 34, 44, 34, 42, 34, 93, 93, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 91, 91, 34, 77, 65, 84, 67, 72, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 44, 34, 87, 73, 84, 72, 34, 44, 34, 80, 82, 79, 68, 85, 67, 84, 83, 34, 44, 34, 70, 82, 79, 77, 34, 44, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 93, 44, 91, 34, 77, 65, 84, 67, 72, 34, 44, 34, 42, 34, 44, 34, 87, 73, 84, 72, 34, 44, 34, 80, 82, 79, 68, 85, 67, 84, 83, 34, 44, 34, 70, 82, 79, 77, 34, 44, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 116, 97, 114, 103, 101, 116, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 99, 111, 115, 105, 103, 110, 46, 107, 101, 121, 46, 112, 117, 98, 46, 106, 115, 111, 110, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 45, 108, 97, 121, 111, 117, 116, 46, 106, 115, 111, 110, 34, 93, 93, 44, 34, 110, 97, 109, 101, 34, 58, 34, 99, 97, 114, 103, 111, 45, 102, 101, 116, 99, 104, 34, 44, 34, 114, 117, 110, 34, 58, 91, 34, 103, 105, 116, 34, 44, 34, 99, 108, 111, 110, 101, 34, 44, 34, 103, 105, 116, 64, 103, 105, 116, 104, 117, 98, 46, 99, 111, 109, 58, 116, 114, 117, 115, 116, 105, 102, 105, 99, 97, 116, 105, 111, 110, 47, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 46, 103, 105, 116, 34, 93, 125, 93, 44, 34, 107, 101, 121, 115, 34, 58, 123, 34, 100, 99, 102, 53, 98, 48, 100, 55, 102, 97, 100, 51, 57, 53, 57, 99, 100, 100, 102, 53, 53, 52, 57, 102, 97, 49, 56, 51, 52, 101, 50, 50, 57, 102, 54, 100, 99, 102, 98, 50, 53, 49, 97, 48, 101, 100, 53, 52, 48, 52, 100, 100, 99, 101, 98, 55, 98, 56, 97, 101, 53, 50, 100, 56, 34, 58, 123, 34, 107, 101, 121, 105, 100, 34, 58, 34, 100, 99, 102, 53, 98, 48, 100, 55, 102, 97, 100, 51, 57, 53, 57, 99, 100, 100, 102, 53, 53, 52, 57, 102, 97, 49, 56, 51, 52, 101, 50, 50, 57, 102, 54, 100, 99, 102, 98, 50, 53, 49, 97, 48, 101, 100, 53, 52, 48, 52, 100, 100, 99, 101, 98, 55, 98, 56, 97, 101, 53, 50, 100, 56, 34, 44, 34, 107, 101, 121, 105, 100, 95, 104, 97, 115, 104, 95, 97, 108, 103, 111, 114, 105, 116, 104, 109, 115, 34, 58, 91, 34, 115, 104, 97, 50, 53, 54, 34, 44, 34, 115, 104, 97, 53, 49, 50, 34, 93, 44, 34, 107, 101, 121, 116, 121, 112, 101, 34, 58, 34, 101, 99, 100, 115, 97, 34, 44, 34, 107, 101, 121, 118, 97, 108, 34, 58, 123, 34, 112, 114, 105, 118, 97, 116, 101, 34, 58, 34, 34, 44, 34, 112, 117, 98, 108, 105, 99, 34, 58, 34, 48, 52, 50, 97, 48, 48, 97, 97, 53, 48, 54, 54, 101, 98, 53, 48, 49, 48, 57, 54, 53, 53, 97, 51, 51, 102, 99, 49, 48, 49, 57, 55, 97, 102, 99, 53, 53, 97, 97, 102, 102, 56, 57, 54, 99, 51, 97, 98, 98, 52, 97, 57, 100, 99, 101, 102, 97, 55, 99, 48, 102, 50, 57, 99, 100, 52, 102, 49, 51, 102, 48, 97, 49, 99, 56, 52, 48, 97, 101, 102, 48, 55, 99, 56, 51, 50, 102, 101, 56, 57, 54, 97, 57, 53, 97, 51, 97, 50, 53, 56, 98, 56, 54, 100, 102, 101, 97, 52, 53, 101, 52, 57, 54, 102, 100, 102, 100, 101, 101, 101, 57, 54, 54, 55, 100, 98, 57, 102, 57, 99, 34, 125, 44, 34, 115, 99, 104, 101, 109, 101, 34, 58, 34, 101, 99, 100, 115, 97, 45, 115, 104, 97, 50, 45, 110, 105, 115, 116, 112, 50, 53, 54, 34, 125, 125, 44, 34, 114, 101, 97, 100, 109, 101, 34, 58, 34, 105, 110, 45, 116, 111, 116, 111, 32, 108, 97, 121, 111, 117, 116, 32, 102, 111, 114, 32, 116, 114, 117, 115, 116, 105, 102, 105, 99, 97, 116, 105, 111, 110, 47, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 46, 34, 44, 34, 115, 116, 101, 112, 115, 34, 58, 91, 123, 34, 95, 116, 121, 112, 101, 34, 58, 34, 115, 116, 101, 112, 34, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 99, 111, 109, 109, 97, 110, 100, 34, 58, 91, 34, 103, 105, 116, 34, 44, 34, 99, 108, 111, 110, 101, 34, 44, 34, 103, 105, 116, 64, 103, 105, 116, 104, 117, 98, 46, 99, 111, 109, 58, 116, 114, 117, 115, 116, 105, 102, 105, 99, 97, 116, 105, 111, 110, 47, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 46, 103, 105, 116, 34, 93, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 109, 97, 116, 101, 114, 105, 97, 108, 115, 34, 58, 91, 93, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 91, 91, 34, 67, 82, 69, 65, 84, 69, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 42, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 45, 108, 97, 121, 111, 117, 116, 46, 106, 115, 111, 110, 34, 93, 93, 44, 34, 110, 97, 109, 101, 34, 58, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 44, 34, 112, 117, 98, 107, 101, 121, 115, 34, 58, 91, 34, 100, 99, 102, 53, 98, 48, 100, 55, 102, 97, 100, 51, 57, 53, 57, 99, 100, 100, 102, 53, 53, 52, 57, 102, 97, 49, 56, 51, 52, 101, 50, 50, 57, 102, 54, 100, 99, 102, 98, 50, 53, 49, 97, 48, 101, 100, 53, 52, 48, 52, 100, 100, 99, 101, 98, 55, 98, 56, 97, 101, 53, 50, 100, 56, 34, 93, 44, 34, 116, 104, 114, 101, 115, 104, 111, 108, 100, 34, 58, 49, 125, 44, 123, 34, 95, 116, 121, 112, 101, 34, 58, 34, 115, 116, 101, 112, 34, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 99, 111, 109, 109, 97, 110, 100, 34, 58, 91, 34, 99, 97, 114, 103, 111, 34, 44, 34, 116, 101, 115, 116, 34, 44, 34, 45, 45, 109, 97, 110, 105, 102, 101, 115, 116, 45, 112, 97, 116, 104, 61, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 93, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 109, 97, 116, 101, 114, 105, 97, 108, 115, 34, 58, 91, 91, 34, 77, 65, 84, 67, 72, 34, 44, 34, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 47, 42, 34, 44, 34, 87, 73, 84, 72, 34, 44, 34, 80, 82, 79, 68, 85, 67, 84, 83, 34, 44, 34, 70, 82, 79, 77, 34, 44, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 93, 44, 91, 34, 68, 73, 83, 65, 76, 76, 79, 87, 34, 44, 34, 42, 34, 93, 93, 44, 34, 101, 120, 112, 101, 99, 116, 101, 100, 95, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 91, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 67, 97, 114, 103, 111, 46, 108, 111, 99, 107, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 99, 111, 115, 105, 103, 110, 46, 107, 101, 121, 46, 106, 115, 111, 110, 34, 93, 44, 91, 34, 65, 76, 76, 79, 87, 34, 44, 34, 99, 111, 115, 105, 103, 110, 46, 107, 101, 121, 46, 112, 117, 98, 46, 106, 115, 111, 110, 34, 93, 44, 91, 34, 68, 73, 83, 65, 76, 76, 79, 87, 34, 44, 34, 42, 34, 93, 93, 44, 34, 110, 97, 109, 101, 34, 58, 34, 114, 117, 110, 45, 116, 101, 115, 116, 115, 34, 44, 34, 112, 117, 98, 107, 101, 121, 115, 34, 58, 91, 34, 100, 99, 102, 53, 98, 48, 100, 55, 102, 97, 100, 51, 57, 53, 57, 99, 100, 100, 102, 53, 53, 52, 57, 102, 97, 49, 56, 51, 52, 101, 50, 50, 57, 102, 54, 100, 99, 102, 98, 50, 53, 49, 97, 48, 101, 100, 53, 52, 48, 52, 100, 100, 99, 101, 98, 55, 98, 56, 97, 101, 53, 50, 100, 56, 34, 93, 44, 34, 116, 104, 114, 101, 115, 104, 111, 108, 100, 34, 58, 49, 125, 93, 125]
metadata: "{\"_type\":\"layout\",\"expires\":\"2024-01-03T12:37:07Z\",\"inspect\":[{\"_type\":\"inspection\",\"expected_materials\":[[\"MATCH\",\"source-distributed/*\",\"WITH\",\"PRODUCTS\",\"FROM\",\"clone-project\"],[\"ALLOW\",\"source-distributed/target\"],[\"ALLOW\",\"cosign.key.pub.json\"],[\"ALLOW\",\"source-distributed-layout.json\"],[\"DISALLOW\",\"*\"]],\"expected_products\":[[\"MATCH\",\"source-distributed/Cargo.toml\",\"WITH\",\"PRODUCTS\",\"FROM\",\"clone-project\"],[\"MATCH\",\"*\",\"WITH\",\"PRODUCTS\",\"FROM\",\"clone-project\"],[\"ALLOW\",\"source-distributed/target\"],[\"ALLOW\",\"cosign.key.pub.json\"],[\"ALLOW\",\"source-distributed-layout.json\"]],\"name\":\"cargo-fetch\",\"run\":[\"git\",\"clone\",\"git@github.com:trustification/source-distributed.git\"]}],\"keys\":{\"dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8\":{\"keyid\":\"dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8\",\"keyid_hash_algorithms\":[\"sha256\",\"sha512\"],\"keytype\":\"ecdsa\",\"keyval\":{\"private\":\"\",\"public\":\"042a00aa5066eb50109655a33fc10197afc55aaff896c3abb4a9dcefa7c0f29cd4f13f0a1c840aef07c832fe896a95a3a258b86dfea45e496fdfdeee9667db9f9c\"},\"scheme\":\"ecdsa-sha2-nistp256\"}},\"readme\":\"in-toto layout for trustification/source-distributed.\",\"steps\":[{\"_type\":\"step\",\"expected_command\":[\"git\",\"clone\",\"git@github.com:trustification/source-distributed.git\"],\"expected_materials\":[],\"expected_products\":[[\"CREATE\",\"source-distributed\"],[\"ALLOW\",\"source-distributed/*\"],[\"ALLOW\",\"source-distributed-layout.json\"]],\"name\":\"clone-project\",\"pubkeys\":[\"dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8\"],\"threshold\":1},{\"_type\":\"step\",\"expected_command\":[\"cargo\",\"test\",\"--manifest-path=source-distributed/Cargo.toml\"],\"expected_materials\":[[\"MATCH\",\"source-distributed/*\",\"WITH\",\"PRODUCTS\",\"FROM\",\"clone-project\"],[\"ALLOW\",\"Cargo.toml\"],[\"DISALLOW\",\"*\"]],\"expected_products\":[[\"ALLOW\",\"Cargo.lock\"],[\"ALLOW\",\"cosign.key.json\"],[\"ALLOW\",\"cosign.key.pub.json\"],[\"DISALLOW\",\"*\"]],\"name\":\"run-tests\",\"pubkeys\":[\"dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8\"],\"threshold\":1}]}"
signatures: {KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"): Signature { key_id: KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"), value: SignatureValue("3046022100edc5a8f7f385aa408d6637f6ad6914f95bd842af917788769990595906434eab022100cfc9f8ca8760531cc73ef5c8cec68a48ab8e499b954a9a24377253c92904a534") }}
public key verifiy...alg: ECDSA_P256_SHA256_ASN1
Good signature from key ID KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8")
Generate sscs/in-toto/artifacts/in-toto-rs/source-distributed-layout.json
Creating work dir "sscs/in-toto/artifacts/in-toto-rs/work"
key_id: "dcf5b0d7"
Cloning into 'source-distributed'...
LinkMetadataBuilder::signed, private_key: PublicKey { typ: Ecdsa, key_id: KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"), scheme: EcdsaP256Sha256, keyid_hash_algorithms: Some(["sha256", "sha512"]), value: PublicKeyValue("042a00aa5066eb50109655a33fc10197afc55aaff896c3abb4a9dcefa7c0f29cd4f13f0a1c840aef07c832fe896a95a3a258b86dfea45e496fdfdeee9667db9f9c") }
Metablock::new signing with key...raw: [123, 34, 95, 116, 121, 112, 101, 34, 58, 34, 108, 105, 110, 107, 34, 44, 34, 98, 121, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 123, 34, 114, 101, 116, 117, 114, 110, 45, 118, 97, 108, 117, 101, 34, 58, 48, 44, 34, 115, 116, 100, 101, 114, 114, 34, 58, 34, 67, 108, 111, 110, 105, 110, 103, 32, 105, 110, 116, 111, 32, 39, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 39, 46, 46, 46, 92, 110, 34, 44, 34, 115, 116, 100, 111, 117, 116, 34, 58, 34, 34, 125, 44, 34, 99, 111, 109, 109, 97, 110, 100, 34, 58, 91, 93, 44, 34, 101, 110, 118, 105, 114, 111, 110, 109, 101, 110, 116, 34, 58, 110, 117, 108, 108, 44, 34, 109, 97, 116, 101, 114, 105, 97, 108, 115, 34, 58, 123, 34, 67, 97, 114, 103, 111, 46, 108, 111, 99, 107, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 98, 54, 50, 97, 53, 97, 53, 56, 97, 100, 99, 97, 99, 98, 52, 50, 97, 48, 50, 54, 100, 50, 51, 56, 101, 57, 102, 102, 49, 56, 57, 54, 98, 99, 57, 49, 54, 101, 99, 57, 54, 102, 53, 50, 50, 50, 102, 97, 98, 57, 54, 52, 100, 54, 49, 57, 56, 97, 49, 55, 49, 53, 52, 102, 34, 125, 44, 34, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 56, 100, 99, 98, 102, 50, 48, 55, 100, 57, 99, 55, 50, 98, 51, 100, 53, 55, 102, 54, 99, 97, 102, 98, 56, 53, 53, 49, 53, 102, 100, 97, 53, 56, 53, 49, 97, 100, 55, 102, 55, 52, 53, 50, 50, 53, 97, 54, 53, 98, 102, 101, 100, 56, 55, 49, 55, 98, 100, 98, 98, 56, 57, 34, 125, 44, 34, 82, 69, 65, 68, 77, 69, 46, 109, 100, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 51, 100, 55, 54, 51, 102, 97, 57, 102, 53, 54, 102, 53, 55, 97, 101, 97, 54, 101, 50, 48, 56, 55, 57, 98, 50, 99, 55, 97, 57, 53, 53, 99, 56, 51, 52, 101, 51, 54, 100, 54, 49, 52, 57, 50, 102, 98, 98, 51, 51, 101, 101, 48, 50, 55, 102, 100, 51, 48, 97, 102, 51, 50, 34, 125, 44, 34, 115, 114, 99, 47, 107, 101, 121, 103, 101, 110, 46, 114, 115, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 53, 54, 54, 50, 53, 53, 54, 57, 55, 98, 50, 49, 98, 99, 48, 102, 99, 53, 100, 56, 99, 54, 53, 55, 99, 98, 99, 97, 52, 97, 56, 99, 53, 99, 52, 50, 100, 56, 102, 102, 54, 52, 98, 51, 52, 49, 57, 52, 51, 52, 50, 101, 101, 50, 57, 101, 49, 97, 49, 98, 49, 97, 97, 97, 34, 125, 125, 44, 34, 110, 97, 109, 101, 34, 58, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 44, 34, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 123, 34, 67, 97, 114, 103, 111, 46, 108, 111, 99, 107, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 98, 54, 50, 97, 53, 97, 53, 56, 97, 100, 99, 97, 99, 98, 52, 50, 97, 48, 50, 54, 100, 50, 51, 56, 101, 57, 102, 102, 49, 56, 57, 54, 98, 99, 57, 49, 54, 101, 99, 57, 54, 102, 53, 50, 50, 50, 102, 97, 98, 57, 54, 52, 100, 54, 49, 57, 56, 97, 49, 55, 49, 53, 52, 102, 34, 125, 44, 34, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 56, 100, 99, 98, 102, 50, 48, 55, 100, 57, 99, 55, 50, 98, 51, 100, 53, 55, 102, 54, 99, 97, 102, 98, 56, 53, 53, 49, 53, 102, 100, 97, 53, 56, 53, 49, 97, 100, 55, 102, 55, 52, 53, 50, 50, 53, 97, 54, 53, 98, 102, 101, 100, 56, 55, 49, 55, 98, 100, 98, 98, 56, 57, 34, 125, 44, 34, 82, 69, 65, 68, 77, 69, 46, 109, 100, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 51, 100, 55, 54, 51, 102, 97, 57, 102, 53, 54, 102, 53, 55, 97, 101, 97, 54, 101, 50, 48, 56, 55, 57, 98, 50, 99, 55, 97, 57, 53, 53, 99, 56, 51, 52, 101, 51, 54, 100, 54, 49, 52, 57, 50, 102, 98, 98, 51, 51, 101, 101, 48, 50, 55, 102, 100, 51, 48, 97, 102, 51, 50, 34, 125, 44, 34, 115, 114, 99, 47, 107, 101, 121, 103, 101, 110, 46, 114, 115, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 53, 54, 54, 50, 53, 53, 54, 57, 55, 98, 50, 49, 98, 99, 48, 102, 99, 53, 100, 56, 99, 54, 53, 55, 99, 98, 99, 97, 52, 97, 56, 99, 53, 99, 52, 50, 100, 56, 102, 102, 54, 52, 98, 51, 52, 49, 57, 52, 51, 52, 50, 101, 101, 50, 57, 101, 49, 97, 49, 98, 49, 97, 97, 97, 34, 125, 125, 125]
Generated sscs/in-toto/artifacts/in-toto-rs/clone-project.dcf5b0d7.link
Verifiy the clone-project step...
in verify...raw: [123, 34, 95, 116, 121, 112, 101, 34, 58, 34, 108, 105, 110, 107, 34, 44, 34, 98, 121, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 123, 34, 114, 101, 116, 117, 114, 110, 45, 118, 97, 108, 117, 101, 34, 58, 48, 44, 34, 115, 116, 100, 101, 114, 114, 34, 58, 34, 67, 108, 111, 110, 105, 110, 103, 32, 105, 110, 116, 111, 32, 39, 115, 111, 117, 114, 99, 101, 45, 100, 105, 115, 116, 114, 105, 98, 117, 116, 101, 100, 39, 46, 46, 46, 92, 110, 34, 44, 34, 115, 116, 100, 111, 117, 116, 34, 58, 34, 34, 125, 44, 34, 99, 111, 109, 109, 97, 110, 100, 34, 58, 91, 93, 44, 34, 101, 110, 118, 105, 114, 111, 110, 109, 101, 110, 116, 34, 58, 110, 117, 108, 108, 44, 34, 109, 97, 116, 101, 114, 105, 97, 108, 115, 34, 58, 123, 34, 67, 97, 114, 103, 111, 46, 108, 111, 99, 107, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 98, 54, 50, 97, 53, 97, 53, 56, 97, 100, 99, 97, 99, 98, 52, 50, 97, 48, 50, 54, 100, 50, 51, 56, 101, 57, 102, 102, 49, 56, 57, 54, 98, 99, 57, 49, 54, 101, 99, 57, 54, 102, 53, 50, 50, 50, 102, 97, 98, 57, 54, 52, 100, 54, 49, 57, 56, 97, 49, 55, 49, 53, 52, 102, 34, 125, 44, 34, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 56, 100, 99, 98, 102, 50, 48, 55, 100, 57, 99, 55, 50, 98, 51, 100, 53, 55, 102, 54, 99, 97, 102, 98, 56, 53, 53, 49, 53, 102, 100, 97, 53, 56, 53, 49, 97, 100, 55, 102, 55, 52, 53, 50, 50, 53, 97, 54, 53, 98, 102, 101, 100, 56, 55, 49, 55, 98, 100, 98, 98, 56, 57, 34, 125, 44, 34, 82, 69, 65, 68, 77, 69, 46, 109, 100, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 51, 100, 55, 54, 51, 102, 97, 57, 102, 53, 54, 102, 53, 55, 97, 101, 97, 54, 101, 50, 48, 56, 55, 57, 98, 50, 99, 55, 97, 57, 53, 53, 99, 56, 51, 52, 101, 51, 54, 100, 54, 49, 52, 57, 50, 102, 98, 98, 51, 51, 101, 101, 48, 50, 55, 102, 100, 51, 48, 97, 102, 51, 50, 34, 125, 44, 34, 115, 114, 99, 47, 107, 101, 121, 103, 101, 110, 46, 114, 115, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 53, 54, 54, 50, 53, 53, 54, 57, 55, 98, 50, 49, 98, 99, 48, 102, 99, 53, 100, 56, 99, 54, 53, 55, 99, 98, 99, 97, 52, 97, 56, 99, 53, 99, 52, 50, 100, 56, 102, 102, 54, 52, 98, 51, 52, 49, 57, 52, 51, 52, 50, 101, 101, 50, 57, 101, 49, 97, 49, 98, 49, 97, 97, 97, 34, 125, 125, 44, 34, 110, 97, 109, 101, 34, 58, 34, 99, 108, 111, 110, 101, 45, 112, 114, 111, 106, 101, 99, 116, 34, 44, 34, 112, 114, 111, 100, 117, 99, 116, 115, 34, 58, 123, 34, 67, 97, 114, 103, 111, 46, 108, 111, 99, 107, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 98, 54, 50, 97, 53, 97, 53, 56, 97, 100, 99, 97, 99, 98, 52, 50, 97, 48, 50, 54, 100, 50, 51, 56, 101, 57, 102, 102, 49, 56, 57, 54, 98, 99, 57, 49, 54, 101, 99, 57, 54, 102, 53, 50, 50, 50, 102, 97, 98, 57, 54, 52, 100, 54, 49, 57, 56, 97, 49, 55, 49, 53, 52, 102, 34, 125, 44, 34, 67, 97, 114, 103, 111, 46, 116, 111, 109, 108, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 56, 100, 99, 98, 102, 50, 48, 55, 100, 57, 99, 55, 50, 98, 51, 100, 53, 55, 102, 54, 99, 97, 102, 98, 56, 53, 53, 49, 53, 102, 100, 97, 53, 56, 53, 49, 97, 100, 55, 102, 55, 52, 53, 50, 50, 53, 97, 54, 53, 98, 102, 101, 100, 56, 55, 49, 55, 98, 100, 98, 98, 56, 57, 34, 125, 44, 34, 82, 69, 65, 68, 77, 69, 46, 109, 100, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 101, 51, 100, 55, 54, 51, 102, 97, 57, 102, 53, 54, 102, 53, 55, 97, 101, 97, 54, 101, 50, 48, 56, 55, 57, 98, 50, 99, 55, 97, 57, 53, 53, 99, 56, 51, 52, 101, 51, 54, 100, 54, 49, 52, 57, 50, 102, 98, 98, 51, 51, 101, 101, 48, 50, 55, 102, 100, 51, 48, 97, 102, 51, 50, 34, 125, 44, 34, 115, 114, 99, 47, 107, 101, 121, 103, 101, 110, 46, 114, 115, 34, 58, 123, 34, 115, 104, 97, 50, 53, 54, 34, 58, 34, 53, 54, 54, 50, 53, 53, 54, 57, 55, 98, 50, 49, 98, 99, 48, 102, 99, 53, 100, 56, 99, 54, 53, 55, 99, 98, 99, 97, 52, 97, 56, 99, 53, 99, 52, 50, 100, 56, 102, 102, 54, 52, 98, 51, 52, 49, 57, 52, 51, 52, 50, 101, 101, 50, 57, 101, 49, 97, 49, 98, 49, 97, 97, 97, 34, 125, 125, 125]
metadata: "{\"_type\":\"link\",\"byproducts\":{\"return-value\":0,\"stderr\":\"Cloning into 'source-distributed'...\n\",\"stdout\":\"\"},\"command\":[],\"environment\":null,\"materials\":{\"Cargo.lock\":{\"sha256\":\"b62a5a58adcacb42a026d238e9ff1896bc916ec96f5222fab964d6198a17154f\"},\"Cargo.toml\":{\"sha256\":\"e8dcbf207d9c72b3d57f6cafb85515fda5851ad7f745225a65bfed8717bdbb89\"},\"README.md\":{\"sha256\":\"e3d763fa9f56f57aea6e20879b2c7a955c834e36d61492fbb33ee027fd30af32\"},\"src/keygen.rs\":{\"sha256\":\"566255697b21bc0fc5d8c657cbca4a8c5c42d8ff64b34194342ee29e1a1b1aaa\"}},\"name\":\"clone-project\",\"products\":{\"Cargo.lock\":{\"sha256\":\"b62a5a58adcacb42a026d238e9ff1896bc916ec96f5222fab964d6198a17154f\"},\"Cargo.toml\":{\"sha256\":\"e8dcbf207d9c72b3d57f6cafb85515fda5851ad7f745225a65bfed8717bdbb89\"},\"README.md\":{\"sha256\":\"e3d763fa9f56f57aea6e20879b2c7a955c834e36d61492fbb33ee027fd30af32\"},\"src/keygen.rs\":{\"sha256\":\"566255697b21bc0fc5d8c657cbca4a8c5c42d8ff64b34194342ee29e1a1b1aaa\"}}}"
signatures: {KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"): Signature { key_id: KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"), value: SignatureValue("3045022100be0c8f562f08ea31aa90ca5ee50076a7a9e26840953891c261f9b33df9af8dd402202fc6a2f9648361b5a4107d6acc8a2c2219f1086dcc5dfe61bf74f0c2aba23d44") }}
public key verifiy...alg: ECDSA_P256_SHA256_ASN1
Bad signature from key ID KeyId("dcf5b0d7fad3959cddf5549fa1834e229f6dcfb251a0ed5404ddceb7b8ae52d8"): BadSignature
thread 'main' panicked at 'called `Result::unwrap()` on an `Err` value: VerificationFailure("Signature threshold not met: 0/1")', src/bin/cargo-in-toto-gen.rs:154:54
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace

To me it looks like when the contents are signed, they are signed using the raw
bytes:
```rust
  impl Metablock {                                                                
    /// Create a new Metablock, using data of metadata. And the signatures are  
    /// generated by using private-keys to sign the metadata.                   
    pub fn new(metadata: MetadataWrapper, private_keys: &[&PrivateKey]) -> Result<Self> {
      let raw = metadata.to_bytes()?;                                         
  
      // sign and collect signatures                                          
      let mut signatures = Vec::new();                                        
      private_keys.iter().try_for_each(|key| -> Result<()> {                  
      let sig = key.sign(&raw)?;
```
But later when verifying, the verification is done on utf8 string:
```rust
pub fn verify<'a, I>(&self, threshold: u32, authorized_keys: I) -> Result<MetadataWrapper>
where                                                                       
          I: IntoIterator<Item = &'a PublicKey>,                                  
      {                                                                           
          let raw = self.metadata.to_bytes()?;                                    
          let metadata = String::from_utf8(raw)                                   
              .map_err(|e| Error::Encoding(format!("Cannot convert metadata into a string: {}", e)))?
              .replace("\\n", "\n");                                              
          let mut signatures_needed = threshold;                                  
                                                                                  
          // Create a key_id->signature map to deduplicate the key_ids.           
          let signatures = self                                                   
              .signatures                                                         
              .iter()                                                             
              .map(|sig| (sig.key_id(), sig))                                     
              .collect::<HashMap<&KeyId, &Signature>>();                          
                                                                                  
          // check the signatures, if is signed by an authorized key,             
          // signatures_needed - 1                                                
                                                                                  
          for (key_id, sig) in signatures {                                       
              match authorized_keys.get(key_id) {                                 
                  Some(pub_key) => match pub_key.verify(metadata.as_bytes(), sig) {
              }
           }
           ...
}
```
And notice that there is a string replacement of the newline characters going
on as well. So, when signing metadata.to_bytes() is called which is implemented
as:
```rust
   /// Standard serialize for MetadataWrapper by its metadata                  
   pub fn to_bytes(&self) -> Result<Vec<u8>> {                                 
       Json::canonicalize(&Json::serialize(self)?)                             
   }
```
