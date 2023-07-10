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
Notice that this is not using any features at all, which we will need later but
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
$ cargo tree --target wasm32-wasi --no-default-features

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

So, perhaps we can add a feature to `reqwest` which will allow the `wasm-bindgen`
dependency, and usages, to be avoided even if the target arch is wasm32.

To add the feature we update Cargo.toml:
```toml
# TODO: add description
wit-bindgen = []
```
And we can inspect the features using:
```console
$ cargo rustc --features="wit-bindgen" --target=wasm32-wasi -- --print cfg
debug_assertions
feature="__tls"
feature="default"
feature="default-tls"
feature="hyper-tls"
feature="native-tls-crate"
feature="tokio-native-tls"
feature="wit-bindgen"
panic="abort"
target_abi=""
target_arch="wasm32"
target_endian="little"
target_env=""
target_family="wasm"
target_feature="crt-static"
target_has_atomic
target_has_atomic="16"
target_has_atomic="32"
target_has_atomic="64"
target_has_atomic="8"
target_has_atomic="ptr"
target_has_atomic_equal_alignment="16"
target_has_atomic_equal_alignment="32"
target_has_atomic_equal_alignment="64"
target_has_atomic_equal_alignment="8"
target_has_atomic_equal_alignment="ptr"
target_has_atomic_load_store
target_has_atomic_load_store="16"
target_has_atomic_load_store="32"
target_has_atomic_load_store="64"
target_has_atomic_load_store="8"
target_has_atomic_load_store="ptr"
target_os="wasi"
target_pointer_width="32"
target_thread_local
target_vendor="unknown"
    Finished dev [unoptimized + debuginfo] target(s) in 7.41s
```
So we can see the feature we added is available, and also not that `target_arch`
is `wasm32`.

We need to update any cfg's in reqwest that use the target_arch=wasm32. Two
can be found in src/errors.rs which we can update to include a condition that
the feature `wit-bindgen` has not been specified.:
```rust
#[cfg(all(not(feature = "wit-bindgen"), target_arch = "wasm32"))]
```
So we are saying that the item below this line should only be included if
the target_arch is wasm32 and that the feature wit-bindgen has not been set.

Hmm...Thinking about this some more, perhaps the should use the `target_os`
instead. The motivation here being that when wasi is specified we are not
assuming the web as the host and there could be other choices. But reqwest
expects to either use hyper (when the target_arch is not wasm32) or
wasm-bindgen.
For now, I'm going to add cfg "guards" to the module that use reqwest to move
forward and then see if I can take some time to try to figure this out.

So after adding the [cfg guards] we can compile successfully using:
```console
$ cargo b -p seedwing-policy-engine --target=wasm32-wasi --no-default-features --features=""
    Finished dev [unoptimized + debuginfo] target(s) in 0.29s
```
And then we generate/make our core WebAssembly module into a WebAssembly
component:
```
$ wasm-tools component new -v ../target/wasm32-wasi/debug/seedwing_policy_engine.wasm --adapt wasi_snapshot_preview1.wasm -o seedwing_policy-engine-component.wasm
```
And we can verify that the imports that were generated by reqwest are not
present anymore:
```console
$ wasm2wat ../target/wasm32-wasi/debug/seedwing_policy_engine.wasm | rustfilt | grep placeholder
```
Now, with that we can try running this module and we can start with a
JavasScript version first:
```console
$ cd js
$ npm i
```
Now we generate the JavaScript bindings for the component using:
```console
$ npm run bindings

> js@1.0.0 bindings
> npx jco transpile $npm_package_config_wasm_file -o dist -w


Transpiled JS Component Files:

 - dist/imports/environment.d.ts                     0.09 KiB
 - dist/imports/exit.d.ts                            0.16 KiB
 - dist/imports/filesystem.d.ts                      2.31 KiB
 - dist/imports/preopens.d.ts                        0.47 KiB
 - dist/imports/streams.d.ts                         0.39 KiB
 - dist/seedwing_policy-engine-component.core.wasm    152 MiB
 - dist/seedwing_policy-engine-component.core2.wasm  12.9 KiB
 - dist/seedwing_policy-engine-component.d.ts        0.37 KiB
 - dist/seedwing_policy-engine-component.js 
```
And we can the run this Node.js using:
```console
$ npm run example

> js@1.0.0 example
> node index.mjs

something was passed: bajja
```
Alright, so we have not yet created any interfaces or types specific to the
policy engine but this at least provides a start and structure around it to and
that we can build upon.

Now, while I'd like to take a look at coming up with a "fix" for reqwest I think
that it might make sense to move forward and create some basic interface for
the policy engine even if it is limited in funtionality. The motivation for this
is that the modules that are more advanced and use features like reqwest and
possibly sigstore might not always be required. For example, for the in-toto
potential usage it would probably be enough to the the basic rule language
evaluation features.

So lets start real simple and start with exposing the version which is a
function that is in `src/lib.rs`, the same file that we added the something
example function to test wit-bindgen out above.
Lets rename the wit to engine.wit add also remove the something function and
instead export the version function:
```
default world engine {
  export engine: self.wit
}

interface wit {
  version: func() -> string
}
```
And we will also need to update src/wit.rs to:
```rust
use crate::wit::engine::Engine;

wit_bindgen::generate!("engine");

struct Exports;

impl Engine for Exports {
    fn version() -> String {
        crate::version().to_string()
    }
}

export_engine!(Exports);
```
I need to dig into the requirement of the Core struct here as it is not clear
to me but it seems like we need something to export. With those changes we
can compile using cargo, generate the component using wasm-tools.
We also need to update the JavaScript example to reflect these changes:
```js
import { engine  } from './dist/seedwing_policy-engine-component.js';

console.log(`Seedwing Policy Engine version: ${engine.version()}`);
```
And after running the `jco` tools we can invoke this example using:
```console
$ npm run example

> js@1.0.0 example
> node index.mjs

Seedwing Policy Engine version: 0.1.0
```
So that is the first integration of the policy engine using bindgen. Next,
I need to to some exploration work with the wit format and figure out how to
structure it (we can separate types/interfaces into separate .wit file and
use/import them into other to create a wit package).

After some exploration coding there is an initial example of evaluating a
policy. For this another function has been exported name eval:
```
  /// Runs a policy.
  ///
  /// policies: additional policies to be include/available to the policy rules.
  /// data: this is a list of data that will be available to the policies .
  /// policy: the policy containing the patterns to be evaluated.
  /// name: the name of the patter to evaluate.
  /// input: the input to be evaluated.
  eval: func(policies: list<string>,
            data: list<string>,
            policy: string,
            name: string,
            input: string,) -> string
```
The `policies` and `data` parameters are currently not supported but I'll look
into them later as they are not needed for the simplest possible example. The
issue with them is that they currently take a list of directories and in this
case I think they should just be list of strings to avoid any filesytem calls.
Anyway, with this we can the update `src/wit.rs` and add the `eval` function
and then build (there is a temp Makefile) to help development of this:
```console
$ make wit-build
cd js && npm run bindings && npm run example

> js@1.0.0 bindings
> npx jco transpile $npm_package_config_wasm_file -o dist -w


Transpiled JS Component Files:

 - dist/exports/engine.d.ts                          0.17 KiB
 - dist/imports/environment.d.ts                     0.09 KiB
 - dist/imports/exit.d.ts                            0.16 KiB
 - dist/imports/filesystem.d.ts                      2.31 KiB
 - dist/imports/preopens.d.ts                        0.47 KiB
 - dist/imports/random.d.ts                           0.1 KiB
 - dist/imports/streams.d.ts                         0.39 KiB
 - dist/seedwing_policy-engine-component.core.wasm    166 MiB
 - dist/seedwing_policy-engine-component.core2.wasm  14.2 KiB
 - dist/seedwing_policy-engine-component.d.ts        0.47 KiB
 - dist/seedwing_policy-engine-component.js            17 KiB


> js@1.0.0 example
> node index.mjs

Seedwing Policy Engine version: 0.1.0
Result:  {
  input: '{"name":"goodboy","trained":true}',
  pattern: 'Pattern { name: Some(PatternName { package: Some(PackagePath { path: [PackageName("wit")] }), name: "dog" }), metadata: PatternMeta { documentation: Documentation(None), unstable: false, deprecation: None, reporting: Reporting { severity: None, explanation: None, authoritative: false } }, examples: [], parameters: [], inner: ObjectPattern {\n' +
    '    fields: [\n' +
    '        Field {\n' +
    '            name: "name",\n' +
    '            ty: Pattern {\n' +
    '                name: None,\n' +
    '                metadata: PatternMeta {\n' +
    '                    documentation: Documentation(\n' +
    '                        None,\n' +
    '                    ),\n' +
    '                    unstable: false,\n' +
    '                    deprecation: None,\n' +
    '                    reporting: Reporting {\n' +
    '                        severity: None,\n' +
    '                        explanation: None,\n' +
    '                        authoritative: false,\n' +
    '                    },\n' +
    '                },\n' +
    '                examples: [],\n' +
    '                parameters: [],\n' +
    '                inner: ref 1<[]>,\n' +
    '            },\n' +
    '            optional: false,\n' +
    '        },\n' +
    '        Field {\n' +
    '            name: "trained",\n' +
    '            ty: Pattern {\n' +
    '                name: None,\n' +
    '                metadata: PatternMeta {\n' +
    '                    documentation: Documentation(\n' +
    '                        None,\n' +
    '                    ),\n' +
    '                    unstable: false,\n' +
    '                    deprecation: None,\n' +
    '                    reporting: Reporting {\n' +
    '                        severity: None,\n' +
    '                        explanation: None,\n' +
    '                        authoritative: false,\n' +
    '                    },\n' +
    '                },\n' +
    '                examples: [],\n' +
    '                parameters: [],\n' +
    '                inner: ref 2<[]>,\n' +
    '            },\n' +
    '            optional: false,\n' +
    '        },\n' +
    '    ],\n' +
    '} }',
  rationale: 'NotAnObject',
  output: 'Identity'
}
```
The fields in the result are still strings but I'll take a stab at adding
actual types to the .wit so that the users can work with types and not just
strings.

I ran into an [issue] when adding a `record` type named `evaluation-result` but
can work around it for now. 

Next, we should now be able to define types for return type of the `eval`
function which looks like this:
```
  eval: func(policies: list<string>,
            data: list<string>,
            policy: string,
            name: string,
            input: string,) -> result<evaluation-result, string>
```
And `evaluation-result` is currently defined like this:
```
  record evaluation-result {
    input: string,
    pattern: string,
    rationale: string,
    output: string,
  }
```
We want to these type to match the types used in the code base. 
`RuntimeValue` look like this:
```rust
pub enum RuntimeValue {
    Null,
    String(Arc<str>),
    Integer(i64),
    Decimal(f64),
    Boolean(bool),
    Object(Object),
    List(Vec<Arc<RuntimeValue>>),
    Octets(#[serde(with = "RuntimeValueBase64")] Vec<u8>),
}
```
So we need to create a type for this enum and there is an `variant` in the wit
specification so lets try using it: 
```
  variant runtime-value {
    null,
    %string(string),
    integer(s64),
    decimal(float64),
    //object(record),
    // This will generate the error "type `rt` depends on itself"
    //%list(list<runtime-value>),
  }
```
Notice that we need to escape identifiers what have the same names as wit
keywords, like string and list above, using `%`.
Next I was not able to specify the list as having elements of this same type.
```console
$ make wit-compile 
cargo b -p seedwing-policy-engine --target=wasm32-wasi --no-default-features --features=""
   Compiling seedwing-policy-engine v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine)
error: failed to parse package: /home/danielbevenius/work/security/seedwing/seedwing-policy/engine/wit
       
       Caused by:
           type `rt` depends on itself
                --> /home/danielbevenius/work/security/seedwing/seedwing-policy/engine/wit/engine.wit:25:16
                 |
              25 |     %list(list<rt>),
                 |                ^-
       
       Stack backtrace:
```
I think that this is pretty common in Rust but Rust is not the only language
that wit caters for remember, so there might be reasons for not allowing this.
I'll raise an [issue](https://github.com/bytecodealliance/wit-bindgen/issues/572)
to ask about this to find out the reason for not allowing this and perhaps find
out if there is a way around it. This is a know 
[issue/limitation](https://github.com/WebAssembly/component-model/issues/56) but
lets see if we can work around this. The idea here is that we define the wit
data type and then transform it.


So we've seen an example of using the wasm component with JavaScript and we
have now added an example which can be run by Python:
```console
$ cd engine/python
$ make bindings 
python3 -m wasmtime.bindgen ../seedwing_policy-engine-component.wasm --out-dir dist
Generating dist/__init__.py
Generating dist/exports/__init__.py
Generating dist/exports/engine.py
Generating dist/imports/__init__.py
Generating dist/imports/environment.py
Generating dist/imports/exit.py
Generating dist/imports/filesystem.py
Generating dist/imports/preopens.py
Generating dist/imports/random.py
Generating dist/imports/streams.py
Generating dist/intrinsics.py
Generating dist/seedwing_policy-engine-component.core0.wasm
Generating dist/seedwing_policy-engine-component.core1.wasm
Generating dist/seedwing_policy-engine-component.core2.wasm
Generating dist/seedwing_policy-engine-component.core3.wasm
Generating dist/types.py
```
And this can the be run using:
```console
$ make run
env WASMTIME_BACKTRACE_DETAILS=1 python3 engine.py

EvaluationResult(input=RuntimeValueString(value='{ "name": "goodboy", "trained": true}'), ty=Pattern(name=PatternName(package=PackagePath(path=['wit']), name='dog'), metadata=PatternMeta(documentation=None, unstable=False, deprecation=None, reporting=Reporting(severity=<Severity.NONE: 0>, explanation=None, authoritative=False)), examples=[], parameters=[], inner=InnerPatternObject(value=ObjectPattern(fields=[Field(name='name', optional=False), Field(name='trained', optional=False)]))), rationale=RationaleNotAnObject(), output='Identity')
```

There is also a Rust example showing how the compnent could be used from Rust.
This might sound a little odd as the Policy Engine itself is written in Rust
but the reason the policy engine exists, or was created, was with secure supply
chain security in mind. Providing the policy engine as a web component module
allows for projects to use it in a secure way. 

_work in progress_

### Questions/Issues

#### Recursive types
One issue is with the types and not having support for [recursive types]
which does not look like it will make it into the MVP (Minimal Viable Product).
So the reply was that recursive types are coming but they will not be available
in the MVP. There are ways to work around this. For example, lets take the
RuntimeValue in seedwing as an example:
```rust
pub enum RuntimeValue {
    Null,
    String(Arc<str>),
    Integer(i64),
    Decimal(f64),
    Boolean(bool),
    Object(Object),
    List(Vec<Arc<RuntimeValue>>),
    Octets(#[serde(with = "RuntimeValueBase64")] Vec<u8>),
}
```
And the wit type would look like this:
```wit
variant runtime-value {
    null,
    %string(string),
    integer(s64),
    decimal(float64),
    //object(object),
    %list(list<runtime-value>),
}
```
What we can do is change the type of the `list` member to:
```with
    %list(list<u32>),
```
And the number will indicate which type of values that the list contains.
So 0 would be `null`, 1 would be `string`, 2 would be `integer`, 3 would be
`decimal`, and 4 would be `list`.




#### How should we deal with reqwest
Like we discussed above reqwest will use wasm-bindgen if that target
arch is `wasm32`. This is the case for both `wasm32-unknown-unknown` and
`wasm32-wasi`. And like we mentioned in our case where we want to use
wit-bindings. One thing that might be worth investigating is using [wasi-http].

I also found [reqwest-wasi] which might be interesting to look into futher. One
thing to note is that the github repository for [reqwest-wasi] is
https://github.com/WasmEdge/reqwest and this is a fork of reqwest made by
WasmEdge.
They also have fork of hypter-tls which migth allow us to use HTTPS which
[reqwest-wasi] currently does not support.

[reqwest-wasi]: https://crates.io/crates/reqwest_wasi


### Future works (suggestions)
For Seedwing I think it could make sense to work on the types and provide a
base component that supports base functionality of the policy engine. Other
features which currently require thirdparty dependencies which might not compile
to wasm/wasi could be provided later. And it might also make sense to create
separate modules/components for them. I need to look into this a little more
but the component model does support dynamic linking of modules.

#### wasmtime-go Component Model support
This [comment](https://github.com/bytecodealliance/wasmtime-go/issues/170#issuecomment-1441976158)
in issue #170 mentions that WebAssembly Component Model support is currently not
implemented for wasmtime-go. Could we help out adding this support?  
Having this would enable the policy engine to be called from Go which simlar to
the JavaScript, Python, and Rust examples above.

### WebAssembly Component generation
[issue 16](https://github.com/seedwing-io/seedwing-policy/issues/16) is about
generating offline compoent modules that contain all the information needed to
evaluate a rule, that is the pattern, the pattern name, and perhaps other data
and policies.

For this we can create a new `world` which imports the policy engine. The 
implementation of this world could then statically contains the policy and call
the imported engine to evaluate the pattern.
Currently, the policy is added at compile time using [include_bytes](https://github.com/danbev/seedwing-policy/blob/88aed53d15f9f6fb78f769610c14b9d60d7cbfec/engine/static-component/src/lib.rs#L17).
This works but we are required to compile the core wasm module first, then make
a WebAssembly component out of it and then compose it with the engine component.
The idea is that it would be possible to have a button in the playground, or
a command in the cli, to ask for such a module to be generated and the returned
.wasm component module would be runnable in any wasm runtime that supports the
webassembly component model.



[wit-bindgen]: https://github.com/danbev/learning-wasi/blob/master/notes/wit-bindgen.md
[frontend]: ./frontend.md
[branch]: https://github.com/danbev/seedwing-policy/tree/wit-bindgen
[wit-bindgen-example]: https://github.com/danbev/learning-wasi/tree/master/wit-bindgen-example
[creating-components-wasi]: https://github.com/bytecodealliance/wit-bindgen#creating-components-wasi
[core]: https://doc.rust-lang.org/core/
[mem]: https://doc.rust-lang.org/core/mem/index.html
[generate]: https://github.com/bytecodealliance/wit-bindgen/blob/8bd0fb32ed68a32e1661c630725d886470fdb632/crates/rust-macro/src/lib.rs#L10
[emit]: https://github.com/bytecodealliance/wit-bindgen/blob/8bd0fb32ed68a32e1661c630725d886470fdb632/crates/rust/src/lib.rs#L1365
[PR]: https://github.com/bytecodealliance/wit-bindgen/pull/568
[errors.rs]: https://github.com/seanmonstar/reqwest/blob/eeca649a3d70c353043b2e42684c6d74f4ba5cae/src/error.rs#L218
[cfd guards]: https://github.com/danbev/seedwing-policy/commit/c893660a99f87d3e326a7509348dac65d44c0ad2
[wasi-http]: https://github.com/WebAssembly/wasi-http
[issue]: https://github.com/bytecodealliance/jco/issues/69
[wasmedge]: https://github.com/WasmEdge/WasmEdge/issues/1877
[recursive types]: https://github.com/WebAssembly/component-model/issues/56
