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

If we inspect the expanded macros for a debug build and a release build we
can look and see what the differences are:
```console
$ cargo +nightly rustc --release -- -Zunstable-options -Z unpretty=expanded > release-expanded  2>&1
$ cargo +nightly rustc -- -Zunstable-options -Z unpretty=expanded > debug-expanded  2>&1
```
To be able to troubleshoot this we need to be able to update the wit-bindgen
code that our project is using. To do this we can use a path dependency in 
Cargo.toml:
```toml
wit-bindgen = { path = "/home/danielbevenius/work/wasm/wit-bindgen/crates/guest-rust", version="0.8.0", default-features = true, features = ['macros'] }
```
Notice that the version that we are using is `0.8.0` so we will need to check
out that version in the wit-bindgen repo:
```console
$ git checkout 0.8.0
$ git ci -b version-0.8.0
```
With that inplace we can start by looking at the places where `debug_assertions``
is used. The first place is in `crates/rust/src/lib.rs`:
```console
$ grep -Rn debug_assertions crates/ src/
crates/guest-rust/src/lib.rs:73:            if cfg!(debug_assertions) {
crates/rust/src/lib.rs:1108:                        #[cfg(not(debug_assertions))]
crates/rust/src/lib.rs:1110:                        #[cfg(debug_assertions)]
crates/rust/src/lib.rs:1127:                        #[cfg(not(debug_assertions))]
crates/rust/src/lib.rs:1129:                        #[cfg(debug_assertions)]
crates/rust/src/lib.rs:1227:                    result.push_str("#[cfg(not(debug_assertions))]");
crates/rust/src/lib.rs:1240:                    result.push_str("#[cfg(debug_assertions)]");
crates/rust/src/lib.rs:1253:                        result.push_str("#[cfg(debug_assertions)]");
crates/rust/src/lib.rs:1255:                        result.push_str("#[cfg(not(debug_assertions))]");
crates/rust/src/lib.rs:1261:                result.push_str("#[cfg(debug_assertions)]");
crates/rust/src/lib.rs:1307:                        result.push_str("#[cfg(debug_assertions)]");
crates/rust/src/lib.rs:1309:                        result.push_str("#[cfg(not(debug_assertions))]");
crates/rust/src/lib.rs:1315:                result.push_str("#[cfg(debug_assertions)]");
crates/rust/src/lib.rs:1346:                        #[cfg(not(debug_assertions))]
crates/rust/src/lib.rs:1348:                        #[cfg(debug_assertions)]
crates/rust/src/lib.rs:1381:                        #[cfg(not(debug_assertions))]
crates/rust/src/lib.rs:1383:                        #[cfg(debug_assertions)]
crates/rust/src/lib.rs:1405:                result.push_str("#[cfg(debug_assertions)]");
crates/rust/src/lib.rs:1421:                result.push_str("#[cfg(not(debug_assertions))]");
crates/rust/src/lib.rs:1501:                    converted.push_str("#[cfg(not(debug_assertions))]");
crates/rust/src/lib.rs:1506:                    converted.push_str("#[cfg(debug_assertions)]");
```
If we take a look at `crates/rust/src/lib.rs` and see where these are used and
looking that them the `emit` function contains a section that handles enums:
```rust
    fn emit(
        &mut self,
        _resolve: &Resolve,
        inst: &Instruction<'_>,
        operands: &mut Vec<String>,
        results: &mut Vec<String>,
    ) {
    ...


            Instruction::VariantLift {
                name, variant, ty, ..
            } => {
                let mut result = String::new();
                result.push_str("{");

                let named_enum = variant.cases.iter().all(|c| c.ty.is_none());
                let blocks = self
                    .blocks
                    .drain(self.blocks.len() - variant.cases.len()..)
                    .collect::<Vec<_>>();
                let op0 = &operands[0];

                if named_enum {
                    // In unchecked mode when this type is a named enum then we know we
                    // defined the type so we can transmute directly into it.
                    result.push_str("#[cfg(not(debug_assertions))]");
                    result.push_str("{");
                    result.push_str("::core::mem::transmute::<_, ");
                    result.push_str(&name.to_upper_camel_case());
                    result.push_str(">(");
                    result.push_str(op0);
                    result.push_str(" as ");
                    result.push_str(int_repr(variant.tag()));
                    result.push_str(")");
                    result.push_str("}");
                }
                if named_enum {
                    result.push_str("#[cfg(debug_assertions)]");
                }
                result.push_str("{");
                result.push_str(&format!("match {op0} {{\n"));
                let name = self.typename_lift(*ty);
                for (i, (case, block)) in variant.cases.iter().zip(blocks).enumerate() {
                    let pat = i.to_string();
                    let block = if case.ty.is_some() {
                        format!("({block})")
                    } else {
                        String::new()
                    };
                    let case = case.name.to_upper_camel_case();
                    if i == variant.cases.len() - 1 {
                        result.push_str("#[cfg(debug_assertions)]");
                        result.push_str(&format!("{pat} => {name}::{case}{block},\n"));
                        result.push_str("#[cfg(not(debug_assertions))]");
                        result.push_str(&format!("_ => {name}::{case}{block},\n"));
                    } else {
                        result.push_str(&format!("{pat} => {name}::{case}{block},\n"));
                    }
                }
                result.push_str("#[cfg(debug_assertions)]");
                result.push_str("_ => panic!(\"invalid enum discriminant\"),\n");
                result.push_str("}");
                result.push_str("}");

                result.push_str("}");
                results.push(result);
            }
}
```
If we try commenting out the `debug_assertions` code:
```console
diff --git a/crates/rust/src/lib.rs b/crates/rust/src/lib.rs
index ae44689..c61b7ff 100644
--- a/crates/rust/src/lib.rs
+++ b/crates/rust/src/lib.rs
@@ -1214,6 +1214,7 @@ impl Bindgen for FunctionBindgen<'_, '_> {
                     .collect::<Vec<_>>();
                 let op0 = &operands[0];
 
+                /*
                 if named_enum {
                     // In unchecked mode when this type is a named enum then we know we
                     // defined the type so we can transmute directly into it.
@@ -1228,9 +1229,10 @@ impl Bindgen for FunctionBindgen<'_, '_> {
                     result.push_str(")");
                     result.push_str("}");
                 }
+                */
 
                 if named_enum {
-                    result.push_str("#[cfg(debug_assertions)]");
+                    //result.push_str("#[cfg(debug_assertions)]");
                 }
                 result.push_str("{");
                 result.push_str(&format!("match {op0} {{\n"));
@@ -1244,15 +1246,15 @@ impl Bindgen for FunctionBindgen<'_, '_> {
                     };
                     let case = case.name.to_upper_camel_case();
                     if i == variant.cases.len() - 1 {
-                        result.push_str("#[cfg(debug_assertions)]");
+                        //result.push_str("#[cfg(debug_assertions)]");
                         result.push_str(&format!("{pat} => {name}::{case}{block},\n"));
-                        result.push_str("#[cfg(not(debug_assertions))]");
+                        //result.push_str("#[cfg(not(debug_assertions))]");
                         result.push_str(&format!("_ => {name}::{case}{block},\n"));
                     } else {
                         result.push_str(&format!("{pat} => {name}::{case}{block},\n"));
                     }
                 }
-                result.push_str("#[cfg(debug_assertions)]");
+                //result.push_str("#[cfg(debug_assertions)]");
                 result.push_str("_ => panic!(\"invalid enum discriminant\"),\n");
                 result.push_str("}");
                 result.push_str("}")
```
We can re-run our release build and it will pass. That is a good sign that we
are on the right track.
So we know that when debug assertions are enabled we things work so we should
look at when debug assertions are disabled. Specifically the following code
looks interesting:
```rust
                if named_enum {
                    // In unchecked mode when this type is a named enum then we know we
                    // defined the type so we can transmute directly into it.
                    result.push_str("#[cfg(not(debug_assertions))]");
                    result.push_str("{");
                    result.push_str("::core::mem::transmute::<_, ");
                    result.push_str(&name.to_upper_camel_case());
                    result.push_str(">(");
                    result.push_str(op0);
                    result.push_str(" as ");
                    result.push_str(int_repr(variant.tag()));
                    result.push_str(")");
                    result.push_str("}");
                }
```
If we revert the changes we made above and then we can see if we can "fix" this
part of the code to work.

The `name` variable in this case is the name of an enum as seen in the wit file,
for example `syntactic-sugar` and calling `to_upper_camel_case` will turn the
name into `SyntacticSugar`. Notice that this is full path to the enum only
the name of the enum. And also notice that this is what gets generated by the
macro, so this is outputted to a file and then the compiler needs to be able
to interpret these paths. Just having `SyntacticSugar` would require an import
of the enum but I don't think that would work in this situation. What might
work is instead using the full path:
```console
diff --git a/crates/rust/src/lib.rs b/crates/rust/src/lib.rs
index ae44689..5d555ee 100644
--- a/crates/rust/src/lib.rs
+++ b/crates/rust/src/lib.rs
@@ -1213,14 +1213,14 @@ impl Bindgen for FunctionBindgen<'_, '_> {
                     .drain(self.blocks.len() - variant.cases.len()..)
                     .collect::<Vec<_>>();
                 let op0 = &operands[0];
-
+                let name = self.typename_lift(*ty);
                 if named_enum {
                     // In unchecked mode when this type is a named enum then we know we
                     // defined the type so we can transmute directly into it.
                     result.push_str("#[cfg(not(debug_assertions))]");
                     result.push_str("{");
                     result.push_str("::core::mem::transmute::<_, ");
-                    result.push_str(&name.to_upper_camel_case());
+                    result.push_str(&name);
                     result.push_str(">(");
                     result.push_str(op0);
                     result.push_str(" as ");
@@ -1234,7 +1234,6 @@ impl Bindgen for FunctionBindgen<'_, '_> {
                 }
                 result.push_str("{");
                 result.push_str(&format!("match {op0} {{\n"));
-                let name = self.typename_lift(*ty);
                 for (i, (case, block)) in variant.cases.iter().zip(blocks).enumerate() {
                     let pat = i.to_string();
                     let block = if case.ty.is_some() {
```
This seems to work, at least for our project. I'll run the tests in wit-bindgen
and create an issue for this to see what the maintainers think.


