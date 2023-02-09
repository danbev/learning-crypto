## Seedwing Policy Engine
Is a policy engine, like OPA for example. The policy language in Sweedwing is
called Dogma, and in OPA it is Rego.

### policy-server walkthrough
Lets start a debugging session and break in the policy-servers main function.
```console
$ rust-gdb --args target/debug/seedwing-policy-server
Reading symbols from target/debug/seedwing-policy-server...
(gdb) br seedwing-policy-server/src/main.rs:55
Breakpoint 1 at 0x1e7c26: file seedwing-policy-server/src/main.rs, line 31.
(gdb) r
```
I'm skipping over some of the "setup" code, like things related to clap and
logging.
```rust
    let mut builder = PolicyBuilder::new();
    let mut sources = Vec::new();
    if let Some(directories) = matches.get_many::<String>("dir") {
        for dir in directories {
            let dir = PathBuf::from(dir);
            if !dir.exists() {
                log::error!("Unable to open directory: {}", dir.to_string_lossy());
                exit(-3);
            }
            sources.push(Directory::new(dir));
        }
    }
```

So PolicyBuilder is imported using the following statement:
```rust
use seedwing_policy_engine::lang::builder::Builder as PolicyBuilder;
```
And we can find the source for it in
`seedwing-policy-engine/src/lang/builder.rs`.

If we now step-into we will land in the contructor function `new` which calls
`hir::World::new()` (seedwing-policy-engine/src/lang/hir/mod.rs):
```rust
impl World {
    pub fn new() -> Self {
        let mut world = Self {
            units: Default::default(),
            packages: Default::default(),
            source_cache: Default::default(),
            data_sources: Vec::default(),
        };
        world.add_package(crate::core::lang::package());
        ...
     }
}
```
Now, the `World` struct is declared like this:
```console
pub struct World {
    units: Vec<CompilationUnit>,
    packages: Vec<Package>,
    source_cache: SourceCache,
    data_sources: Vec<Arc<dyn DataSource>>,
}
```
Let's start with the vector of CompilationUnit's. CompilationUnit can be found
in seedwing-policy-engine/src/lang/parser/mod.rs:
```rust
#[derive(Debug)]
pub struct CompilationUnit {
    source: SourceLocation,
    uses: Vec<Located<Use>>,
    types: Vec<Located<TypeDefn>>,
}
```
So a compilation unit represents a `.dog` file I think, which has  a source
which is the name of the source and not the actual source.

`uses` is when we have `use` statements in a pattern source.  For example:
```
use list::all
```

### Typedefn
`types` is a filed in a CompilationUnit and I'm not sure about what they are, we
can see that it is a vector of `Typedefn`:
```rust
#[derive(Clone, Debug)]
pub struct TypeDefn {
    name: Located<String>,
    ty: Located<Type>,
    parameters: Vec<Located<String>>,
    documentation: Option<String>,
}
```
Let's stick a break point in TypeDefn::new and see what we can figure out:
```console
$ rust-gdb --args target/debug/seedwing-policy-server
Reading symbols from target/debug/seedwing-policy-server...
(gdb) br seedwing_policy_engine::lang::hir::TypeDefn::new 
Breakpoint 1 at 0x36a71b: file seedwing-policy-engine/src/lang/hir/mod.rs, line 88.
(gdb) r

[2023-02-09T05:12:34Z INFO  seedwing_policy_engine::lang::hir] loading x509::oid

Breakpoint 1, seedwing_policy_engine::lang::hir::TypeDefn::new (name=..., ty=..., parameters=Vec(size=1) = {...}) at seedwing-policy-engine/src/lang/hir/mod.rs:88
88	            name,
```
And how did we get here:
```console
(gdb) bt
#0  seedwing_policy_engine::lang::hir::TypeDefn::new (name=..., ty=..., parameters=Vec(size=1) = {...})
    at seedwing-policy-engine/src/lang/hir/mod.rs:88
#1  0x00005555558900ed in seedwing_policy_engine::lang::parser::ty::type_definition::{closure#1} ()
    at seedwing-policy-engine/src/lang/parser/ty.rs:156
#2  0x00005555559bec76 in chumsky::combinator::{impl#17}::parse_inner::
...
#31 0x0000555555816f91 in seedwing_policy_engine::lang::parser::PolicyParser::parse<alloc::boxed::Box<dyn core::iter::traits::iterator::Iterator<Item=(char, core::ops::range::Range<usize>)>, alloc::alloc::Global>, seedwing_policy_engine::lang::parser::SourceLocation, alloc::string::String> (self=0x7fffffff6170, source=..., 
    stream="@v\300\002PU\000\000\230\r1\261i\230Z\375ID of 85.4.3, common name.\npattern common<pattern> = {\n  oid: \"85.4.3\",\n  value: pattern,\n}\n\n/// Matches OID of 85.4.10, organization name.\npattern organization<pattern> = {\n  oid: \"85"...) at seedwing-policy-engine/src/lang/parser/mod.rs:278
#32 0x00005555558bff76 in seedwing_policy_engine::lang::hir::World::lower (self=0x7fffffffa4f0)
    at seedwing-policy-engine/src/lang/hir/mod.rs:424
#33 0x00005555557d50cb in seedwing_policy_engine::lang::builder::{impl#1}::finish::{async_fn#0} ()
    at seedwing-policy-engine/src/lang/builder.rs:36
#34 0x000055555573cff5 in seedwing_policy_server::main::{async_block#0} () at seedwing-policy-server/src/main.rs:87
```
Lets take a look at where we started parsing in the engine itself, before
chumsky code that is:
```console
(gdb) up 32
#32 0x00005555558bff76 in seedwing_policy_engine::lang::hir::World::lower (self=0x7fffffffa4f0)
    at seedwing-policy-engine/src/lang/hir/mod.rs:424
424	                let unit = PolicyParser::default().parse(source.to_owned(), stream);
(gdb) l
419	
420	        for pkg in &self.packages {
421	            for (source, stream) in pkg.source_iter() {
422	                log::info!("loading {}", source);
423	                self.source_cache.add(source.clone(), stream.clone().into());
424	                let unit = PolicyParser::default().parse(source.to_owned(), stream);
425	                match unit {
426	                    Ok(unit) => {
427	                        core_units.push(unit);
428	                    }
```
We can inspect `source` using:
```console
(gdb) p source.name
$3 = seedwing_policy_engine::lang::parser::SourceLocation {name: "x509::oid"}
```
So where did this come source from?  
It is in the `self.packages` vector:
```console
(gdb) p *(self.packages.buf.ptr.pointer.pointer + 6)
$46 = seedwing_policy_engine::package::Package {
  path: seedwing_policy_engine::runtime::PackagePath {
    is_absolute: true,
    path: Vec(size=1) = {
      seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName> {
        inner: seedwing_policy_engine::runtime::PackageName ("x509"),
        location: seedwing_policy_engine::lang::parser::Location {
          span: core::ops::range::Range<usize> {
            start: 0, end: 0
          }
        }
      }
    }
  },
  functions: HashMap(size=2) = {
    ["DER"] = Arc(strong=1, weak=0) = {
      value = dyn seedwing_policy_engine::core::Function,
      strong = 1,
      weak = 0},
    ["PEM"] = Arc(strong=1, weak=0) = {
      value = dyn seedwing_policy_engine::core::Function,
      strong = 1,
      weak = 0}
    },
    sources: Vec(size=2) = {
      seedwing_policy_engine::package::PackageSource {
        name: "oid",
        content: "


                 /// Matches OID of 85.4.3, common name.
                 pattern common<pattern> = {
                   oid: \"85.4.3\",
                   value: pattern,
                 }

                 /// Matches OID of 85.4.10, organization name.
                 pattern organization<pattern> = {
                   oid: \"85.4.10\",
                   value: pattern,
                 }" 
       },
       seedwing_policy_engine::package::PackageSource {
         name: "",
         content: "pattern certificate = base64::base64-url(x509::DER)"
       }
      }
    }
```
From this we can see that the `PackageName` is `x509`, and this package has
two functions, `DER`, and `PEM`. So with a package name and a function name we
might guess that these functions can be called using `x509::DER`. `x509::PEM`.
I also has two sources, one is named 'oid' and the other is unnamed. We can
see the content of these sources above. So how did this entry make it into
the packages vector in the first place?

To understand this we need to have a look at
```console
(gdb) l seedwing_policy_engine::lang::hir::World::new 
316	impl Default for World {
317	    fn default() -> Self {
318	        Self::new()
319	    }
320	}
321	
322	impl Clone for World {
323	    fn clone(&self) -> Self {
324	        let mut h = World::new();
325	        h.packages = self.packages.clone();
326	        h
327	    }
328	}
329	
330	impl World {
331	    pub fn new() -> Self {
332	        let mut world = Self {
333	            units: Default::default(),
334	            packages: Default::default(),
335	            source_cache: Default::default(),
336	            data_sources: Vec::default(),
337	        };
338	        world.add_package(crate::core::lang::package());
339	        world.add_package(crate::core::list::package());
340	        world.add_package(crate::core::string::package());
341	        world.add_package(crate::core::base64::package());
342	        world.add_package(crate::core::json::package());
343	        #[cfg(feature = "sigstore")]
344	        world.add_package(crate::core::sigstore::package());
345	        world.add_package(crate::core::x509::package());
```
Notice, the call `world.add_package(crate::core::x509::package());`. This is
the same as our package name above `x509`. So lets take a look at that package
function:
```console
(gdb) set listsize 15
(gdb) l seedwing_policy_engine::core::x509::package 
13	use std::sync::Arc;
14	use x509_parser::parse_x509_certificate;
15	use x509_parser::pem::Pem;
16	use x509_parser::x509::X509Version;
17	
18	pub mod convert;
19	
20	pub fn package() -> Package {
21	    let mut pkg = Package::new(PackagePath::from_parts(vec!["x509"]));
22	    pkg.register_function("pem".into(), PEM);
23	    pkg.register_function("der".into(), DER);
24	    pkg.register_source("oid".into(), include_str!("oid.dog"));
25	    pkg.register_source("".into(), include_str!("certificate.dog"));
26	    pkg
27	}
```
We can see that this function registers the two functions and adds the two
sources. If we take a look at `oid.dog` we find:
```console
$ cat seedwing-policy-engine/src/core/x509/oid.dog 



/// Matches OID of 85.4.3, common name.
pattern common<pattern> = {
  oid: "85.4.3",
  value: pattern,
}

/// Matches OID of 85.4.10, organization name.
pattern organization<pattern> = {
  oid: "85.4.10",
  value: pattern,
}
```
And notice that this matches the contents field above. Likewise the unanmed
source looks like this:
```console
$ cat seedwing-policy-engine/src/core/x509/certificate.dog 
pattern certificate = base64::base64-url(x509::der)
```
Alright, so back to our question which was trying to figure out what a
TypeDefn is. Let dump back to the lowest stack frame, which will take use
to the constructor of TypeDefn:
```console
(gdb) down 100
(gdb) l
81	    parameters: Vec<Located<String>>,
82	    documentation: Option<String>,
83	}
84	
85	impl TypeDefn {
86	    pub fn new(name: Located<String>, ty: Located<Type>, parameters: Vec<Located<String>>) -> Self {
87	        Self {
88	            name,
89	            ty,
90	            parameters,
91	            documentation: None,
92	        }
93	    }
```
Let's take a look at the name argument:
```console
(gdb) p name
$73 = seedwing_policy_engine::lang::parser::Located<alloc::string::String> {
        inner: "common",
        location: seedwing_policy_engine::lang::parser::Location {
          span: core::ops::range::Range<usize> {
            start: 51,
            end: 57
          }
        }
      }
```
Looking at the above output we can see that the name is `common`and it has a
location of the file it originated in, which we can see from the `cat` command
above. The `start` posistion is where the name starts and notice that there are
three empty lines in the source file which accounts for 3 characters, then we
have the commens which is 39, so 43 plus the number of character up to the starts
of `common`. 
Next, we have the `ty` or type:
```console
gdb) p ty
$77 = seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Type> {
        inner: seedwing_policy_engine::lang::hir::Type::Object(
          seedwing_policy_engine::lang::hir::ObjectType {
            fields: Vec(size=2) = {
              seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Field> {
                inner: seedwing_policy_engine::lang::hir::Field {
                  name: seedwing_policy_engine::lang::parser::Located<alloc::string::String> {
                    inner: "oid", location: seedwing_policy_engine::lang::parser::Location {
                      span: core::ops::range::Range<usize> {
                        start: 73,
                        end: 76
                      }
                    }
                  },
                  ty: seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Type> {
                    inner: seedwing_policy_engine::lang::hir::Type::Const(seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::lir::ValueType> {
                       inner: seedwing_policy_engine::lang::lir::ValueType::String("85.4.3"),
                       location: seedwing_policy_engine::lang::parser::Location {
                         span: core::ops::range::Range<usize> {
                           start: 78,
                           end: 86
                         }
                       }
                     },
                     location: seedwing_policy_engine::lang::parser::Location {
                       span: core::ops::range::Range<usize> {
                         start: 78,
                         end: 86
                       }
                     }
                   },
                   optional: false
                 },
                 location: seedwing_policy_engine::lang::parser::Location {
                   span: core::ops::range::Range<usize> {
                     start: 73,
                     end: 86
                   }
                 }
              }, 
              seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Field> {
                inner: seedwing_policy_engine::lang::hir::Field {
                  name: seedwing_policy_engine::lang::parser::Located<alloc::string::String> {
                    inner: "value",
                    location: seedwing_policy_engine::lang::parser::Location {
                      span: core::ops::range::Range<usize> {
                        start: 90,
                        end: 95
                      }
                     }
                  },
                  ty: seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Type> {
                    inner: seedwing_policy_engine::lang::hir::Type::Parameter(seedwing_policy_engine::lang::parser::Located<alloc::string::String> {
                      inner: "pattern",
                      location: seedwing_policy_engine::lang::parser::Location {
                         span: core::ops::range::Range<usize> {
                           start: 97,
                           end: 104
                         }
                       }
                     }),
                     location: seedwing_policy_engine::lang::parser::Location {
                       span: core::ops::range::Range<usize> {
                         start: 97,
                         end: 104
                       }
                     }
                   },
                   optional: false
                 },
                 location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 90, end: 104}}}}}), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 69, end: 109}}}
```
This is a seedwing_policy_engine::lang::hir::ObjectType which describes a types
that is defined like this:
```
pattern common<pattern> = {
  oid: "85.4.3",
  value: pattern,
}
```
```console
(gdb) l seedwing_policy_engine::lang::hir::ObjectType::new
237	#[derive(Clone, Debug)]
238	pub struct ObjectType {
239	    fields: Vec<Located<Field>>,
240	}
```
In this case we have a vector of two
`seedwing_policy_engine::lang::hir::Field`'s:
```console
(gdb) l seedwing_policy_engine::lang::hir::Field::new
277	pub struct Field {
278	    name: Located<String>,
279	    ty: Located<Type>,
280	    optional: bool,
281	}
```
This leaves us with the final argument which is `parameters`:
```console
(gdb) p parameters
$78 = Vec(size=1) = {
  seedwing_policy_engine::lang::parser::Located<alloc::string::String> {inner: "pattern", location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 58, end: 65}}}}
```

So hopefully that helps understand what TypeDefn is.


I'm thinking that `data_sources` are data that we want to have loaded into
the engine, which could be json files containing information that can then
be accessed by rules to match against.



### Dogma policy language

#### Types
In the source tree we have the following directories in src/lang:
* hir (High Level Intermediate Representation)
* mir (Mid Level Intermediate Representation)
* lir (Low Level Intermediate Representation)

So lets try to understand what is in each of these modules.

#### lir (Low Level Intermediate Representation)
seedwing-policy-engine/src/lang/lir/mod.rs contains the base building blocks
of the types in the engine.

There is a Type struct which represents a types in the engine, an Expr enum
which has variants for the expressions available.

```rust
#[derive(Debug, Serialize)]
pub struct Type {
    name: Option<TypeName>,
    documentation: Option<String>,
    parameters: Vec<String>,
    inner: InnerType,
}
```
So we cn see that a Type can have an optional name, optional documentation, 
parameters, and an inner type.
TypeName can be found in the src/runtime module:

```rust
#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct TypeName {
    package: Option<PackagePath>,
    name: String,
}
```
Alright, so a TypeName has a name and optionally belongs to a PackagePath.

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct PackagePath {
    is_absolute: bool,
    path: Vec<Located<PackageName>>,
}
And we can see that a PackagePath has a boolean indicating if it is an absolute
or relative path. It also has the path itself which is Vector which can hold
elemenets of `Located<PackageName`.
```rust
/// An internal type used to facilitate error prioritisation. You shouldn't need to interact with this type during
/// normal use of the crate.
pub struct Located<I, E> {
    pub(crate) at: usize,
    pub(crate) error: E,
    pub(crate) phantom: PhantomData<I>,
}
```
And PackageName looks like this:
```rust

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct PackageName(pub(crate) String);
```
The `parameters` member is not clear to me what is represents at the moment.
A Type represents some structure or content of input to the engine.

A type also has a member named `inner` which is of type `InnerType`:
```rust
pub enum InnerType {
    Anything,
    Primordial(PrimordialType),
    Bound(Arc<Type>, Bindings),
    Ref(SyntacticSugar, usize, Vec<Arc<Type>>),
    Deref(Arc<Type>),
    Argument(String),
    Const(ValueType),
    Object(ObjectType),
    Expr(Arc<Expr>),
    List(Vec<Arc<Type>>),
    Nothing,
}
```
The `PrimordialType` is defined in `src/lang/mod.rs`
```rust
#[derive(Debug, Clone, Serialize)]
pub enum PrimordialType {
    Integer,
    Decimal,
    Boolean,
    String,
    Function(SyntacticSugar, TypeName, #[serde(skip)] Arc<dyn Function>),
}
```

There is also a type for a `Field`
```
#[derive(Serialize, Debug)]
pub struct Field {
    name: String,
    ty: Arc<Type>,
    optional: bool,
}
```

#### mir (Mid Level Intermediate Representation)
This module also `Type` but is an enum unlike the Type in lir which is a
struct):
```rust
pub enum Type {
    Anything,
    Primordial(PrimordialType),
    Ref(SyntacticSugar, usize, Vec<Arc<TypeHandle>>),
    Deref(Arc<TypeHandle>),
    Argument(String),
    Const(ValueType),
    Object(ObjectType),
    Expr(Arc<Located<Expr>>),
    List(Vec<Arc<TypeHandle>>),
    Nothing,
}
```
This looks very simliar to the lir enum InnerType:
```rust
pub enum InnerType {
    Anything,
    Primordial(PrimordialType),
    Bound(Arc<Type>, Bindings),
    Ref(SyntacticSugar, usize, Vec<Arc<Type>>),
    Deref(Arc<Type>),
    Argument(String),
    Const(ValueType),
    Object(ObjectType),
    Expr(Arc<Expr>),
    List(Vec<Arc<Type>>),
    Nothing,
}
```


In this module we can find types like `TypeHandle`
```rust
#[derive(Default, Debug)]
pub struct TypeHandle {
    name: Option<TypeName>,
    documentation: Option<String>,
    ty: RefCell<Option<Arc<Located<Type>>>>,
    parameters: Vec<Located<String>>,
}
```
This module also defines the `World` struct:
```rust
#[derive(Debug)]
pub struct World {
    type_slots: Vec<Arc<TypeHandle>>,
    types: HashMap<TypeName, usize>,
}
```
An an instance of this struct has a `type_slots` member which is a vector (Vec)
of atomic reference counted (Arc) TypeHandle's. And it also has a hashmap of
TypeName's mapped to their sizes.

Lets take a closer look at the constructor function and look at how it populates
a World instance:
```rust

    pub(crate) fn new() -> Self {
        let mut this = Self {
            type_slots: vec![],
            types: Default::default(),
        };

        this.define_primordial("integer", PrimordialType::Integer);
        ...
    }

    fn define_primordial(&mut self, name: &str, ty: PrimordialType) {
        let name = TypeName::new(None, name.into());

        let ty = Arc::new(TypeHandle::new_with(
            Some(name.clone()),
            Located::new(mir::Type::Primordial(ty), 0..0),
        ));

        self.type_slots.push(ty);
        self.types.insert(name, self.type_slots.len() - 1);
    }
```
Now, we might this that we already know what TypeName is, but I did not
recognize the first argument to TypeName::new and need to look it up. When
doing this I found that this is not the TypeName from the lir that I was thining
of but instead the TypeName from the runtime module:
```rust
#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct TypeName {
    package: Option<PackagePath>,
    name: String,
}
```
And this matches the above `TypeName::new` call. But the lir is meant for
lower level code and would not be exposed at this level so we can assume it is
not used here.
That is a PackagePath?

```rust
        let ty = Arc::new(TypeHandle::new_with(
            Some(name.clone()),
            Located::new(mir::Type::Primordial(ty), 0..0),
        ));
```
`Located` might something like span in rust where it indicates where the type is
used?
```rust
pub struct Located<T> {
    inner: T,
    location: Location,
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct Location {
    span: SourceSpan,
}

pub type SourceSpan = std::ops::Range<usize>;
```

### playground evaluate issue "Option::unwrap() on None value"
I ran into this issue the first time I tried out the playground locally.
I used the simplest pattern possible:
```
pattern something = integer
```
I compiles fine but when I try to evaluate it using:
```
7

pattern = something
```
The error below is generated:
```rust
[2023-02-07T06:31:31Z INFO  seedwing_policy_engine::lang::mir] define type vex::openvex::affected
[2023-02-07T06:31:31Z INFO  seedwing_policy_engine::lang::mir] define type maven::coordinates
[2023-02-07T06:31:31Z INFO  seedwing_policy_engine::lang::mir] Compiling 132 patterns
thread 'actix-rt|system:0|arbiter:1' panicked at 'called `Option::unwrap()` on a `None` value', seedwing-policy-engine/src/lang/mir/mod.rs:79:35
stack backtrace:
   0: rust_begin_unwind
             at /rustc/fc594f15669680fa70d255faec3ca3fb507c3405/library/std/src/panicking.rs:575:5
   1: core::panicking::panic_fmt
             at /rustc/fc594f15669680fa70d255faec3ca3fb507c3405/library/core/src/panicking.rs:64:14
   2: core::panicking::panic
             at /rustc/fc594f15669680fa70d255faec3ca3fb507c3405/library/core/src/panicking.rs:111:5
   3: core::option::Option<T>::unwrap
             at /rustc/fc594f15669680fa70d255faec3ca3fb507c3405/library/core/src/option.rs:778:21
   4: seedwing_policy_engine::lang::mir::TypeHandle::ty
             at /home/danielbevenius/work/security/seedwing/seedwing-policy/seedwing-policy-engine/src/lang/mir/mod.rs:79:9
   5: seedwing_policy_engine::runtime::World::add
             at /home/danielbevenius/work/security/seedwing/seedwing-policy/seedwing-policy-engine/src/runtime/mod.rs:443:18
   6: seedwing_policy_engine::lang::mir::World::lower
             at /home/danielbevenius/work/security/seedwing/seedwing-policy/seedwing-policy-engine/src/lang/mir/mod.rs:462:13
   7: seedwing_policy_engine::lang::builder::Builder::finish::{{closure}}
             at /home/danielbevenius/work/security/seedwing/seedwing-policy/seedwing-policy-engine/src/lang/builder.rs:37:23
   8: <seedwing_policy_server::playground::evaluate as actix_web::service::HttpServiceFactory>::register::evaluate::{{closure}}
             at ./src/playground.rs:106:51
   9: actix_web::handler::handler_service::{{closure}}::{{closure}}
             at /home/danielbevenius/.cargo/registry/src/github.com-1ecc6299db9ec823/actix-web-4.2.1/src/handler.rs:106:21
```
After adding some log statements I found that the TypeHandle in this case if
for a Data function.

Lets start a debugging session to understand what is going on:
```console
$ rust-gdb --args ../target/debug/seedwing-policy-server
Reading symbols from ../target/debug/seedwing-policy-server...
(gdb) br main.rs:55
Breakpoint 1 at 0x1d7a42: file seedwing-policy-server/src/main.rs, line 55.
```

