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

_work in progress_

[wit-bindgen]: https://github.com/danbev/learning-wasi/blob/master/notes/wit-bindgen.md
[frontend]: ./frontend.md
[branch]: https://github.com/danbev/seedwing-policy/tree/wasi
