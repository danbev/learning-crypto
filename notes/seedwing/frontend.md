### Seedwing Frontend
The frontend uses Yew which is a component based framework for writing web
applications similar to React and Elm (which does not really help me as I don't
have any experience with those frameworks) using WebAssembly (Wasm).

So this enables us to write a pure Rust application which is then compiled to
wasm. Wasm has the potential to be more performat than JavaScript.

For a standalone example of yew see [yew-example].

### frontend
The frontend uses trunk to build the wasm. When we run `trunk build`, Trunk will
compile the project
[cargo](https://github.com/thedodd/trunk/blob/7769a17e517391b3b4077c21062725aa1600346d/src/pipelines/rust.rs#L233) using:
```console
$ cargo b --target=wasm32-unknown-unknown
```
And one of its dependencies is the policy engine:
```toml
seedwing-policy-engine = { path = "../engine", default-features = false, features = [] }
```
Notice that this specifies no default features and no features. Now, one thing
to keep in mind is that the frontend is a `bin` project. It uses wasm-bindgen
to take care of the bindings between JavaScript and wasm. The frontend uses
some types from the policy engine but mostly it makes Restful calls to the
server. So what will be compiled from the policy-engine is only what the
frontend uses. So don't think that because trunk/cargo can compile the
frontend project using the above command it is possible to compile the policy-
engine project using:
```console
$ cargo b --target=wasm32-unknown-unknown --no-default-features --features=""
```
This will currently not work.

Now if we start the frontend using `trunk serve` and try to accesss something
from the policy server api endpoint that will fail with a
connection refused. We need the policy server running so that it can handle
the calls. And the policy server is a normal (non-wasm) server binary but it
can embedd the frontend (the single page web app) but the Rest API calls will
still be processed by the server.

### server
The policy-server can be started using the following command:
```console
$ cargo r -p swio -- serve
```
The swio project has the `frontend` feature enabled by default. The serve
command will call `seedwing_policy_server::run` which can be found in
server/src/lib.rs.
The server's Cargo.toml has the following dependency:
```toml
seedwing-policy-server-embedded-frontend = { path = "embedded-frontend", optional = true }
```
This is optional and included by the `frontend` feature which is enabled in our
case:
```toml
[features]                                                                       
frontend = [
"seedwing-policy-server-embedded-frontend",
...
]
```
Another thing to not is that server/embedded-frontend has a build.rs defined for
it (the crate/package) which will be invoked before the package is compiled.
What build.rs does is that it will check if a list of files have been changed
in `../../frontend` and perform a `trunk build` placing the output in the `dist`
directory (embedded-frontend).
```rust
use static_files::resource_dir;

let mut assets = resource_dir(dist);                                        
assets
    .with_generated_filename(Path::new(&out_dir).join("generated-console.rs"))
    .with_generated_fn("generate_console_assets");
assets.build()?;
```
So the files in the `dist` directory will be read by the static-files crate and
the binary content of them included via the file generate_console.rs
This file will be generated somewhere in the target directory:
```console
./target/debug/build/seedwing-policy-server-embedded-frontend-10e7bb4e3e7f96f4/out/generated-console.rs
```
```rust
mod generate_console_assets_sets;                                               
pub use generate_console_assets_sets::generate_console_assets;
```
In generate_console_assets_sets/mod.rs we find the following:
```rust
use ::static_files::resource::new_resource as n;
use ::std::include_bytes as i;

use ::static_files::Resource;
use ::std::collections::HashMap;
mod set_1;

#[allow(clippy::unreadable_literal)]
pub fn generate_console_assets() -> ::std::collections::HashMap<&'static str, ::static_files::Resource> {
    let mut r = ::std::collections::HashMap::new();
    set_1::generate(&mut r);
    r
}
```
and in generate_console_assets_sets/set_1:
```rust
use super::*;

pub(crate) fn generate(r: &mut HashMap<&'static str, Resource>) {
    r.insert("images/logo-inverted.png", n(i!("/seedwing/seedwing-policy/target/debug/build/seedwing-policy-server-embedded-frontend-10e7bb4e3e7f96f4/out/dist/images/logo-inverted.png"), 1683026041, "image/png")); 
    ...
    r.insert("seedwing-policy-frontend-ddbbb3211b2f64a_bg.wasm", n(i!("/seedwing/seedwing-policy/target/debug/build/seedwing-policy-server-embedded-frontend-10e7bb4e3e7f96f4/out/dist/seedwing-policy-frontend-ddbbb3211b2f64a_bg.wasm"), 1683026063, "application/wasm"));
    ....
}
```
Now, this will be used by server/embedded-frontend/src/lib.rs:
```rust
use std::collections::HashMap;                                                     
                                                                                   
include!(concat!(env!("OUT_DIR"), "/generated-console.rs"));                    
                                                                                   
pub fn console_assets() -> HashMap<&'static str, ::static_files::Resource> {       
    generate_console_assets()                                                   
}
```
And in server/src/lib.rs we have the following code that calls
`console_assets()`:
```rust
use actix_web_static_files::ResourceFiles;

let spa = seedwing_policy_server_embedded_frontend::console_assets();
let spa = ResourceFiles::new("/", spa).resolve_not_found_to_root();
app.default_service(spa) 
```
Notice that the `actix_web_static_files` crate is being used to serve the
static resources from `/`.  So this is how the Single Page WebApplication (spa)
gets served. It will still call out to policy-server using Rest api calls and
the policy-server will use the policy-engine to fullfill those requests. 

[yew-example]: https://github.com/danbev/learning-rust/tree/master/yew-example



