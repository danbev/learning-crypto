## Policy Engine Wasm
The purpose of this page it to investigate if/how Seedwing Policy Engine can be
compiled to wasm target (wasm32-unknown-unknown, or wasm32-wasi). The motivation
for doing this is that would make it possible to the use [wit-bindgen] with the
engine.

### Current state
If one takes a quick look over the codebase one might get the impression that
wasm support is already present. I was actually not sure of this until I looked
into how the [frontend] works. In summary of those notes, the frontend uses
Trunk which compiles the frontend project using the `wasm-unknown-unknown`
target. This works as the frontend code only uses a few modules and types from
the engine crate, so it is not actually compiling the whole engine crate to
a a wasm-unknown-unknown target.

### wasm32-wasi
We first have to install `wasm32-wasi` using the following command:
```console
$ rustup target add wasm32-wasi
```
We also have to specify that the `engine` crate produce a `cdylib` instead of
the default `rlib` by updating engine/Cargo.toml and adding:
```toml
[lib]
crate-type = ["cdylib"]
```

We can now try building using the following command:
```
$ env RUSTFLAGS="--cfg tokio_unstable" cargo b -vv --target=wasm32-wasi --no-default-features --features="" 
```
Notice that this is not using any features at all which we will need later but
this is a start.

After this we should have a .wasm file in
`../target/wasm32-wasi/debug/seedwing_policy_engine.wasm`.

This is the working [branch] that contains the changes for this investigation
work.

The next step is to create a .wit file for the engine. This is created in
a directory named `wit` the engine crates directory:
```
default world component {
  export something: func(s: string) -> string
}
```
This is copied from an example and still using the same names as the example
just to verify that things work. After that I'll rename them.

And we need to add the wit-bindgen crate as a dependency to Cargo.toml:
```toml
wit-bindgen = { git = "https://github.com/bytecodealliance/wit-bindgen", version = "0.5.0" }
```
With that in place we can add the following macro to src/lib.rs:
```rust
wit_bindgen::generate!("component");

struct Something;

impl Component for Something {
     fn something(s: String) -> String {
         format!("something was passed: {s}")
     }
}                                                                               
export_component!(Something);
```
Again, this is just the same as the [wit-bindgen-example] used to verify that
part works.

We should now be able to build the core WebAssembly module (which will then be
used to generate the WebAssembly component):
```console
$ cargo b --target=wasm32-wasi --no-default-features --features="" 
   Compiling seedwing-policy-engine v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine)
error[E0433]: failed to resolve: could not find `mem` in `core`
  --> engine/src/lib.rs:29:1
   |
29 | wit_bindgen::generate!("component");
   | ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ could not find `mem` in `core`
   |
   = note: this error originates in the macro `wit_bindgen::generate` (in Nightly builds, run with -Z macro-backtrace for more info)

For more information about this error, try `rustc --explain E0433`.
error: could not compile `seedwing-policy-engine` due to previous error
```

_work in progress_

```console
$ wasm-tools component new ../target/wasm32-wasi/debug/seedwing_policy_engine.wasm -o seedwing_policy-engine-component.wasm
```


[wit-bindgen]: https://github.com/danbev/learning-wasi/blob/master/notes/wit-bindgen.md
[frontend]: ./frontend.md
[branch]: https://github.com/danbev/seedwing-policy/tree/wasi
