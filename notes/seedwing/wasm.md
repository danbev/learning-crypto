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
$ cargo b -p seedwing_policy_engine --target=wasm32-wasi --no-default-features --features="" 
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
Lets add the `-Z macro-backtrace` and see if that provides more information:
```console
$ env RUSTFLAGS="-Z macro-backtrace" cargo +nightly b -vv --target=wasm32-wasi --no-default-features --features="" 
...
  Compiling tokio v1.27.0
error: Only features sync,macros,io-util,rt,time are supported on wasm.
   --> /home/danielbevenius/.cargo/registry/src/github.com-1ecc6299db9ec823/tokio-1.27.0/src/lib.rs:488:1
    |
488 | compile_error!("Only features sync,macros,io-util,rt,time are supported on wasm.");
    | ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```
So that did not work. I tried adding trace_macros!(true); around the
macro to see if that provided any additional information. This expanded the
other macros but interestingly not the generate macro, and I still get this
error:
```console
error[E0433]: failed to resolve: could not find `mem` in `core`
  --> engine/src/lib.rs:31:1
   |
31 | wit_bindgen::generate!("component");
   | ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ could not find `mem` in `core`
   |
   = note: this error originates in the macro `wit_bindgen::generate` (in Nightly builds, run with -Z macro-backtrace for more info)

For more information about this error, try `rustc --explain E0433`.
error: could not compile `seedwing-policy-engine` due to previous error
```
So what is the difference between the [wit-bindgen-example]?  
Well it does not use the wasm32-wasi target but instead wasm32-unknown-unknown.
But I've tried changing the example to use wasm32-wasi and it compile without
this error. 

So the [core] crate contains a [mem] module and it seems to me that this is
colliding with:
```rustc
mod core; 
```
If I comment out all the modules it will compile successfully. So I'm guessing
the macro is somehow using core::mem somewhere and this crates core is being
used and it does not have `mem` module and hence the error is what I'm thinking
at the moment.

I've got wit-bindgen checked out and we can replace the git dependency with a
local path:
```toml
wit-bindgen = { path = "/home/danielbevenius/work/wasm/wit-bindgen/crates/guest-rust", version = "0.6.0" }
```
It was not obvious to me that I had to specify the path `crates/guest-rust` at
first and I though that just using "/home/danielbevenius/work/wasm/wit-bindgen/"
would have worked (and was what I tried initially). It seems like cargo will
search a git repository to find the crate with the specified dependency name.
With this I'm able to make changes to the macro which can be found in
[generate].

I was able to reproduce this by just adding a module name core in the
wit-bindgen-example as well:
```rust
mod core {                                                                       
    // something...                                                              
}
```

Adding print statement after the call of the `parse_macro_input!` macro I can
see that the TokenStream contains a core and a mem identifier:
```
Ident { ident: "core", span: #3355 bytes(544..579) },
Punct { ch: ':', spacing: Joint, span: #3355 bytes(544..579) },
Punct { ch: ':', spacing: Alone, span: #3355 bytes(544..579) },
Ident { ident: "mem", span: #3
```
Which I'm reading as `core::mem`.

With the wit-bindgen example, lets rename the core mod that I added to reproduce
this and name it core2, and then run cargo expand:
```console
$ cargo expand
...

mod core2 {}
pub trait Component {
    fn something(s: wit_bindgen::rt::string::String) -> wit_bindgen::rt::string::String;
}

#[doc(hidden)]
pub unsafe fn call_something<T: Component>(arg0: i32, arg1: i32) -> i32 {
    #[allow(unused_imports)]
    use wit_bindgen::rt::{alloc, vec::Vec, string::String};
    let len0 = arg1 as usize;
    let result1 = T::something({
        #[cfg(debug_assertions)]
        { String::from_utf8(Vec::from_raw_parts(arg0 as *mut _, len0, len0)).unwrap() }
    });
    let ptr2 = _RET_AREA.0.as_mut_ptr() as i32;
    let vec3 = (result1.into_bytes()).into_boxed_slice();
    let ptr3 = vec3.as_ptr() as i32;
    let len3 = vec3.len() as i32;

    core::mem::forget(vec3);  <--------------------------------------

    *((ptr2 + 4) as *mut i32) = len3;
    *((ptr2 + 0) as *mut i32) = ptr3;
    ptr2
}
```
Notice that there is a `core::mem::forget` call which is using the `core::mem`
module and I think this is what is causing the conflict with our crate.
The line in question is generated by [emit]. A suggestion for working
around/fixing this is to use the following in `Bindgen::emit`:
```rust
                 if realloc.is_some() {
-                    self.push_str(&format!("core::mem::forget({});\n", val));
+                    self.push_str(&format!("::core::mem::forget({});\n", val));
                 }
                 results.push(ptr);
                 results.push(len);
```
I've opened a [PR] containing this suggestion and will see what the maintainers
think. There are actually more of the same changes required but that is the one
that we encountered.

So with that out of the way and using a path dependency to a local branch of
wit-bindgen the compilation step works and we have a core wasm module generated
(../target/wasm32-wasi/debug/seedwing_policy_engine.wasm).

The next step to turn that core wasm module into a wasm component which we can
do using the `wasm-tools component` command:
```console
$ wasm-tools component new \ 
  ../target/wasm32-wasi/debug/seedwing_policy_engine.wasm \
  --adapt wasi_snapshot_preview1.wasm \ 
  -o seedwing_policy-engine-wasi.wasm

Error: failed to encode a component from module

Caused by:
    module requires an import interface named `__wbindgen_placeholder__`
```
If we take a look at the wasm module we can see that is has the following 
imports:
```
  (import "__wbindgen_placeholder__" "__wbindgen_describe"
    (func $wasm_bindgen::__wbindgen_describe (type 10))
  )
  (import "__wbindgen_externref_xform__" "__wbindgen_externref_table_grow" 
    (func $wasm_bindgen::externref::__wbindgen_externref_table_grow (type 9))
  )
  (import "__wbindgen_externref_xform__" "__wbindgen_externref_table_set_null" 
    (func $wasm_bindgen::externref::__wbindgen_externref_table_set_null (type 10))
  )
  (import "__wbindgen_placeholder__" "__wbindgen_throw" 
    (func $wasm_bindgen::__wbindgen_throw (type 12))
  )
```
It is complaining about the first and last import above but not the other ones
which might be something to consider. And the functions are describe and
throw. This looks strange to me as I was not expecting anything named
`wasm_bindgen`. The engine crate does not depend on wasm-bindgen yet we are
still seeing those imports in the core wasm file generated by the cargo build.

We can display the dependencies and specify the the target as well to get the
correct information:
```console
$ cargo tree --target wasm32-wasi

│   │   ├── reqwest v0.11.16
│   │   │   ├── base64 v0.21.0
│   │   │   ├── bytes v1.4.0
│   │   │   ├── futures-core v0.3.28
│   │   │   ├── futures-util v0.3.28 (*)
│   │   │   ├── http v0.2.9
│   │   │   │   ├── bytes v1.4.0
│   │   │   │   ├── fnv v1.0.7
│   │   │   │   └── itoa v1.0.6
│   │   │   ├── js-sys v0.3.61
│   │   │   │   └── wasm-bindgen v0.2.84
│   │   │   │       ├── cfg-if v1.0.0
│   │   │   │       └── wasm-bindgen-macro v0.2.84 (proc-macro)
│   │   │   │           ├── quote v1.0.26 (*)
│   │   │   │           └── wasm-bindgen-macro-support v0.2.84
│   │   │   │               ├── proc-macro2 v1.0.56 (*)
│   │   │   │               ├── quote v1.0.26 (*)
│   │   │   │               ├── syn v1.0.109 (*)
│   │   │   │               ├── wasm-bindgen-backend v0.2.84
│   │   │   │               │   ├── bumpalo v3.12.0
│   │   │   │               │   ├── log v0.4.17
│   │   │   │               │   │   └── cfg-if v1.0.0
│   │   │   │               │   ├── once_cell v1.17.1
│   │   │   │               │   ├── proc-macro2 v1.0.56 (*)
│   │   │   │               │   ├── quote v1.0.26 (*)
│   │   │   │               │   ├── syn v1.0.109 (*)
│   │   │   │               │   └── wasm-bindgen-shared v0.2.84
│   │   │   │               └── wasm-bindgen-shared v0.2.84
│   │   │   ├── mime_guess v2.0.4
```
So `reqwest` does have a dependency to wasm-bindgen and perhaps somehow causing
the imports to be generated? 

If we look in [Cargo.toml](https://github.com/seanmonstar/reqwest/blob/eeca649a3d70c353043b2e42684c6d74f4ba5cae/Cargo.toml#L163) we can see the following dependencies:
```toml
[target.'cfg(target_arch = "wasm32")'.dependencies]
js-sys = "0.3.45"
serde_json = "1.0"
wasm-bindgen = "0.2.68"
wasm-bindgen-futures = "0.4.18"
wasm-streams = { version = "0.2", optional = true }
```
And if we look in [errors.rs] we can see the following:
```rust
#[cfg(target_arch = "wasm32")]
impl From<crate::error::Error> for wasm_bindgen::JsValue {
    fn from(err: Error) -> wasm_bindgen::JsValue {
        js_sys::Error::from(err).into()
    }
}
```
And this is indeed the case as the target_arch in our case is wasm32. So that
at least explains why we are seeing wasm-bindgen imports. But in our case we
don't want this. 

To verify this tried commenting out the `reqwest` dependency, and also `guac`
which also has `reqwest` as a dependency then the above import is gone and the
`wasm-tools` command succeeds.

_work in progress_

[wit-bindgen]: https://github.com/danbev/learning-wasi/blob/master/notes/wit-bindgen.md
[frontend]: ./frontend.md
[branch]: https://github.com/danbev/seedwing-policy/tree/wasi
[wit-bindgen-example]: https://github.com/danbev/learning-wasi/tree/master/wit-bindgen-example
[creating-components-wasi]: https://github.com/bytecodealliance/wit-bindgen#creating-components-wasi
[core]: https://doc.rust-lang.org/core/
[mem]: https://doc.rust-lang.org/core/mem/index.html
[generate]: https://github.com/bytecodealliance/wit-bindgen/blob/8bd0fb32ed68a32e1661c630725d886470fdb632/crates/rust-macro/src/lib.rs#L10
[emit]: https://github.com/bytecodealliance/wit-bindgen/blob/8bd0fb32ed68a32e1661c630725d886470fdb632/crates/rust/src/lib.rs#L1365
[PR]: https://github.com/bytecodealliance/wit-bindgen/pull/568
[errors.rs]: https://github.com/seanmonstar/reqwest/blob/eeca649a3d70c353043b2e42684c6d74f4ba5cae/src/error.rs#L218
