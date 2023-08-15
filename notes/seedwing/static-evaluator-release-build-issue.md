## Release build issue with static-evaluator
This is an issue that I ran into when trying to compile a release build of
a webassembly component that used wit-bindgen.

### Error
```console
$ cargo b --release
   Compiling static-evaluator v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine/static-component/static-evaluator)
error[E0412]: cannot find type `SyntacticSugar` in this scope
  --> src/lib.rs:9:1
   |
9  | / wit_bindgen::generate!({
10 | |     path: "../../wit/",
11 | |     world: "static-evaluator",
12 | |     macro_export,
13 | | });
   | |  ^
   | |  |
   | |__not found in this scope
   |    in this macro invocation
   |
  ::: /home/danielbevenius/.cargo/registry/src/index.crates.io-6f17d22bba15001f/wit-bindgen-rust-macro-0.9.0/src/lib.rs:11:1
   |
11 |   pub fn generate(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
   |   -------------------------------------------------------------------------- in this expansion of `wit_bindgen::generate!`
   |
   = help: consider importing this enum through its public re-export:
           crate::SyntacticSugar

warning: unused import: `crate::types::SyntacticSugar`
 --> src/lib.rs:6:5
  |
6 | use crate::types::SyntacticSugar;
  |     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  |
  = note: `#[warn(unused_imports)]` on by default

For more information about this error, try `rustc --explain E0412`.
warning: `static-evaluator` (lib) generated 1 warning
error: could not compile `static-evaluator` (lib) due to previous error; 1 warning emitted
```

The wit looks like this:
```wit
package seedwing:policy

world static-evaluator {
  import engine
  import static-config
  export run: func() -> string
}

interface static-types {
  record config {
    policy: string,
    policy-name: string,
  }
}

interface static-config {
  use static-types.{config}
  policy-config: func() -> config
}

world static-world {
  export static-config
}


world engine-world {
  export engine
}

interface engine {

  use types.{data-type, evaluation-result-context, runtime-value}
  version: func() -> string
  eval: func(policies: list<string>,
             data: list<tuple<string, data-type>>,
             policy: string,
             name: string,
             input: runtime-value) -> result<evaluation-result-context, string>
}
```
Now, the enum SyntacticSugar is defined in a separate file engine-types.wit
and is used by the policy engine component. The policy engine component can be
compiled as a release build without this error.

So why does the policy engine component compile without this error but the
static-evaluator component fails?  

We can try compiling using by specifying the `opt-level` manually and see if we
can work backwards:

Using `opt-level=0` works which is expected as it would be the same as doing
a debug build:
```console
$ env RUSTFLAGS="-C opt-level=0" cargo build --release
```
But setting `opt-level=1` fails with the above error:
```console
$ env RUSTFLAGS="-C opt-level=1" cargo build --release
```

Lets see what happens if we enable debug-asssertions for a release build:
```console
$ RUSTFLAGS="-C debug-assertions=true opt-level=3" cargo build --release --target=wasm32-wasi
   Compiling bitflags v2.3.3
   Compiling ryu v1.0.14
   Compiling itoa v1.0.8
   Compiling serde v1.0.171
   Compiling wit-bindgen v0.9.0
   Compiling serde_json v1.0.102
   Compiling static-evaluator v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine/static-component/static-evaluator)
    Finished release [optimized] target(s) in 5.38s
```
So what worked. And just to verify we can specify it as false as well:
```console
$ RUSTFLAGS="-C debug-assertions=false" cargo build --release --target=wasm32-wasi
   Compiling bitflags v2.3.3
   Compiling ryu v1.0.14
   Compiling itoa v1.0.8
   Compiling serde v1.0.171
   Compiling wit-bindgen v0.9.0
   Compiling serde_json v1.0.102
   Compiling static-evaluator v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine/static-component/static-evaluator)
error[E0412]: cannot find type `SyntacticSugar` in this scope
  --> src/lib.rs:8:1
   |
8  | / wit_bindgen::generate!({
9  | |     path: "../../wit/",
10 | |     world: "static-evaluator",
11 | | });
   | |__^ not found in this scope
   |
   = help: consider importing this enum:
           crate::types::SyntacticSugar
   = note: this error originates in the macro `wit_bindgen::generate` (in Nightly builds, run with -Z macro-backtrace for more info)

For more information about this error, try `rustc --explain E0412`.
error: could not compile `static-evaluator` (lib) due to previous error
```
_wip_
