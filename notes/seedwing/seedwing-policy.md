## Seedwing Policy Engine Internals
Is a policy engine, like OPA for example. The policy language in Sweedwing is
called Dogma, and in OPA it is Rego.

# Table of Contents
1. [CompilationUnit](#compilationunit)
2. [TypeDefn](#typedefn)
3. [Lowerer](#lowerer)

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
So a compilation unit represents a `.dog` file I think, which has a source
which is the name of the source and not the actual source.

### CompilationUnit
This section tries to show how parsing is done. A Rust library named [Chumsky]
is used and I found it useful to try out some standalone [chumsky examples] to
gain a basic understanding of it.

```console
$ rust-gdb --args target/debug/seedwing-policy-server
Reading symbols from target/debug/seedwing-policy-server...
(gdb) br seedwing_policy_engine::lang::parser::PolicyParser::parse
Breakpoint 1 at 0x2c2be3: file seedwing-policy-engine/src/lang/parser/mod.rs, line 276.
(gdb) r
```console
Breakpoint 1, seedwing_policy_engine::lang::parser::PolicyParser::parse<alloc::boxed::Box<dyn core::iter::traits::iterator::Iterator<Item=(char, core::ops::range::Range<usize>)>, alloc::alloc::Global>, seedwing_policy_engine::lang::parser::SourceLocation, alloc::string::String> (self=0x7fffffff6170, source=..., stream="\n\n\n/// Matches OID of 85.4.3, common name.\npattern common<pattern> = {\n  oid: \"85.4.3\",\n  value: pattern,\n}\n\n/// Matches OID of 85.4.10, organization name.\npattern organization<pattern> = {\n  oid: \"85"...) at seedwing-policy-engine/src/lang/parser/mod.rs:276
276	        let tokens = lexer().parse(stream)?;
```
And we can find out how we got here:
```console
(gdb) bt 4
#0  seedwing_policy_engine::lang::parser::PolicyParser::parse<alloc::boxed::Box<dyn core::iter::traits::iterator::Iterator<Item=(char, core::ops::range::Range<usize>)>, alloc::alloc::Global>, seedwing_policy_engine::lang::parser::SourceLocation, alloc::string::String> (self=0x7fffffff6170, source=..., stream="\n\n\n/// Matches OID of 85.4.3, common name.\npattern common<pattern> = {\n  oid: \"85.4.3\",\n  value: pattern,\n}\n\n/// Matches OID of 85.4.10, organization name.\npattern organization<pattern> = {\n  oid: \"85"...) at seedwing-policy-engine/src/lang/parser/mod.rs:276
#1  0x00005555558bff76 in seedwing_policy_engine::lang::hir::World::lower (self=0x7fffffffa4f0) at seedwing-policy-engine/src/lang/hir/mod.rs:424
#2  0x00005555557d50cb in seedwing_policy_engine::lang::builder::{impl#1}::finish::{async_fn#0} () at seedwing-policy-engine/src/lang/builder.rs:36
#3  0x000055555573cff5 in seedwing_policy_server::main::{async_block#0} () at seedwing-policy-server/src/main.rs:87
```
Using `up` we an understand how we got here:
```console
(gdb) up 3
#3  0x000055555573cff5 in seedwing_policy_server::main::{async_block#0} () at seedwing-policy-server/src/main.rs:87
87	    }
(gdb) l
82	    if let Some(directories) = matches.get_many::<String>("data") {
83	        for each in directories {
84	            log::info!("loading data from {:?}", each);
85	            builder.data(DirectoryDataSource::new(each.into()));
86	        }
87	    }
88	
89	    let result = builder.finish().await;
```
This line number reported by gdb is not correct unless I'm missing something
but we know by doing `down` that `builder.finish()` was called.
```console
(gdb) l
31	    {
32	        self.hir.build(sources)
33	    }
34	
35	    pub async fn finish(&mut self) -> Result<runtime::World, Vec<BuildError>> {
36	        let mir = self.hir.lower()?;
37	        let runtime = mir.lower()?;
38	        Ok(runtime)
39	    }
```
And the lowest stack fram is for `self.hir.lower`:
```console
(gdb) down
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
(gdb) f
#1  0x00005555558bff76 in seedwing_policy_engine::lang::hir::World::lower (self=0x7fffffffa4f0) at seedwing-policy-engine/src/lang/hir/mod.rs:424
424	                let unit = PolicyParser::default().parse(source.to_owned(), stream);
```
Like mentioned in the [TypeDefn](#typeDefn) section this will be parsing a
single dogma source:
```console
(gdb) p source
$1 = seedwing_policy_engine::lang::parser::SourceLocation {name: "x509::oid"}
```
And stepping into `parse`:
```console
(gdb) s
(gdb) l
271	        Self: Sized,
272	        Iter: Iterator<Item = (ParserInput, <ParserError as Error<ParserInput>>::Span)> + 'a,
273	        Src: Into<SourceLocation> + Clone,
274	        S: Into<Stream<'a, ParserInput, <ParserError as Error<ParserInput>>::Span, Iter>>,
275	    {
276	        let tokens = lexer().parse(stream)?;
277	        let tokens = remove_comments(&tokens);
278	        let (compilation_unit, errors) = compilation_unit(source).parse_recovery_verbose(
279	            Stream::from_iter(tokens.len()..tokens.len() + 1, tokens.iter().cloned()),
280	        );
(gdb) s
(gdb) l
338	pub fn lexer(
339	) -> impl Parser<ParserInput, Vec<(ParserInput, SourceSpan)>, Error = ParserError> + Clone {
340	    any().map_with_span(|l, span| (l, span)).repeated()
341	}
```
The Chumsky `any` parser will accept anything, and map_with_span will attach a
span, and repeated means that this will be repeated so every thing will be
parsed.
```console
(gdb) n
(gdb) n
(gdb) n
(gdb) p tokens.len
$4 = 228
```
I'm not listing the tokens vector but this can be done.
```console
(gdb) f
277	        let tokens = remove_comments(&tokens);
```
The above is just removing comments from the source. After that we have:
```console
(gdb) l
273	        Src: Into<SourceLocation> + Clone,
274	        S: Into<Stream<'a, ParserInput, <ParserError as Error<ParserInput>>::Span, Iter>>,
275	    {
276	        let tokens = lexer().parse(stream)?;
277	        let tokens = remove_comments(&tokens);
278	        let (compilation_unit, errors) = compilation_unit(source).parse_recovery_verbose(
279	            Stream::from_iter(tokens.len()..tokens.len() + 1, tokens.iter().cloned()),
280	        );
```
So let's step into `compilation_unit`.
```rust
                                                      [input]      [output]
pub fn compilation_unit<S>(source: S,) -> impl Parser<ParserInput, CompilationUnit, Error = ParserError> + Clone where S: Into<SourceLocation> + Clone, {
    use_statement()
        .padded()
        .repeated()
        .then(type_definition().padded().repeated())
        .then_ignore(end())
        .map(move |(use_statements, types)| {
            let mut unit = CompilationUnit::new(source.clone().into());

            for e in use_statements {
                unit.add_use(e)
            }

            for e in types {
                unit.add_type(e)
            }

            unit
        })
}
```
Chumsky is a parser combinator and the engine has parsers defined that handle
the various tokens in the Dogma language.  `use_statement()` returns a Chumsky
parser:
```rust
pub fn use_statement() -> impl Parser<ParserInput, Located<Use>, Error = ParserError> + Clone {
    just("use")
        .padded()
        .ignored()
        .then(type_name())
        .then(as_clause().or_not())
        // .then( just(";").padded().ignored() )
        .map_with_span(|(((_, type_path), as_clause)), span| {
            Located::new(Use::new(type_path, as_clause), span)
        })
}
```
`just` specifies that only that sequence of characters are accepted by the
parser returned from this function. `padding` says that it is alright to
whitespace characters before and/or after the `use` sequence.

The `then` parser will cause this parser to yield a tuple of the first parser,
`use_statement()`, and the second which is `type_definition()`. For example,
a .dog file can specify zero or more `use` statement(s), followed by one or more
type definitions.

The tuple produced is passed to `.map` where we can see that a new
CompilationUnit is created, and all the use statement (if there are any) are
added to the compilation unit. Likewise, the types are also added, and
finally the unit is returned.

Now, the type of these `use` statements are
`seedwing_policy_engine::lang::hir::TypeDefn` which will be in the next section.

### TypeDefn
`types` is a field in a CompilationUnit and I'm not sure about what they are, we
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
It is in the `self.packages` vector which can be inspected using the following
command. This command might look strange but please see [rust-gdb] for details.
I've also formatted the output to make it a little more readable which I think
helps visualize things:
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
see the content of these sources above. What we described here is the
`seedwing_policy_engine::package::Package`:
```
#[derive(Clone)]
pub struct Package {
    path: PackagePath,
    functions: HashMap<String, Arc<dyn Function>>,
    sources: Vec<PackageSource>,
}
```

So how did this entry make it into the packages vector in the first place?

To understand this we need to have a look at
`seedwing_policy_engine::lang::hir::World::new`:
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

So we have now seen what CompoilationUnit's and that TypeDefn are a part of
them. Now closer look at where these CompilationUnit's are used.

### Lowerer
After the package sources have been parsed, the CompilationUnit's are added to
the `seedwing_policy_engine::lang::hir::World` instance which is done in
`World::lower`.
The final thing to happen in `World::lower` is that `Lowerer::lower` is called,
passing in the CompilationUnit's, and the Packages:
```console
(gdb) with listsize 0 -- l seedwing_policy_engine::lang::hir::Lowerer::lower
413	    pub fn lower(&mut self) -> Result<mir::World, Vec<BuildError>> {
414	        self.add_package(crate::core::data::package(self.data_sources.clone()));
415	
416	        let mut core_units = Vec::new();
417	
418	        let mut errors = Vec::new();
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
429	                    Err(err) => {
430	                        for e in err {
431	                            errors.push((source.clone(), e).into())
432	                        }
433	                    }
434	                }
435	            }
436	        }
437	
438	        for unit in core_units {
439	            self.add_compilation_unit(unit);
440	        }
441	
442	        if !errors.is_empty() {
443	            return Err(errors);
444	        }
445	
446	        Lowerer::new(&mut self.units, &mut self.packages).lower()
447	    }
448	}
```
And we can see that Lowerer is declared as:
```console
449	
450	struct Lowerer<'b> {
451	    units: &'b mut Vec<CompilationUnit>,
452	    packages: &'b mut Vec<Package>,
453	}
```
There is a lot going on in `Lowerer::lower` so lets try to take it in steps:
```console
460	    pub fn lower(mut self) -> Result<mir::World, Vec<BuildError>> {
461	        // First, perform internal per-unit linkage and type qualification
462	        let mut world = mir::World::new();
463	        let mut errors = Vec::new();
```

Now, there is another `World` in the module `seedwing_policy_engine::lang::mir`:
```console
(gdb) with listsize 15 -- l seedwing_policy_engine::lang::mir::World::new
196	#[derive(Debug)]
197	pub struct World {
198	    type_slots: Vec<Arc<TypeHandle>>,
199	    types: HashMap<TypeName, usize>,
200	}
```
Now, `TypeHandle` is not something that we have come accross before.
Lets set a breakpoint in seedwing_policy_engine::lang::mir::World::new and
take a closer look:
```console
(gdb) br seedwing_policy_engine::lang::mir::World::new
Breakpoint 1 at 0x567660: file seedwing-policy-engine/src/lang/mir/mod.rs, line 212.
(gdb) r
(gdb) r
Starting program: /home/danielbevenius/work/security/seedwing/seedwing-policy/target/debug/seedwing-policy-server 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading x509::oid
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading x509
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading cyclonedx::v1_4
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading cyclonedx::hash
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading spdx::license
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading iso::swid
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading kafka::opa
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading openvex
[2023-02-10T07:25:19Z INFO  seedwing_policy_engine::lang::hir] loading maven

Breakpoint 1, seedwing_policy_engine::lang::mir::World::new () at seedwing-policy-engine/src/lang/mir/mod.rs:212
212	            type_slots: vec![],
(gdb) l
211	        let mut this = Self {
212	            type_slots: vec![],
213	            types: Default::default(),
214	        };
215	
216	        this.define_primordial("integer", PrimordialType::Integer);
```
We can see that initially `type_slots` is set to an empty vector, and types is
set to the default value for a HashMap:
```console
(gdb) p this
$1 = seedwing_policy_engine::lang::mir::World {type_slots: Vec(size=0), types: HashMap(size=0)}
```
The `type_slots` vector contains TypeHandle's, and the `types` hashmap contains
the TypeName as the key, and the value is an index into the `type_slots` vector.

A TypeHandle looks like this:
```console
(gdb) ptype seedwing_policy_engine::lang::mir::TypeHandle
type = struct seedwing_policy_engine::lang::mir::TypeHandle {
  name: core::option::Option<seedwing_policy_engine::runtime::TypeName>,
  documentation: core::option::Option<alloc::string::String>,
  ty: core::cell::RefCell<core::option::Option<alloc::sync::Arc<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type>>>>,
  parameters: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<alloc::string::String>, alloc::alloc::Global>,
}
```
Lets start with the `name` field and see what `TypeName` looks like:
```console
(gdb) ptype seedwing_policy_engine::runtime::TypeName
type = struct seedwing_policy_engine::runtime::TypeName {
  package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>,
  name: alloc::string::String,
```
```console
(gdb) ptype seedwing_policy_engine::runtime::PackagePath
type = struct seedwing_policy_engine::runtime::PackagePath {
  is_absolute: bool,
  path: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global>,
}
```
Notice that a PackagePath has a vector of PackageName element and PackageName
looks like this:
```
(gdb) ptype seedwing_policy_engine::runtime::PackageName
type = struct seedwing_policy_engine::runtime::PackageName (
  alloc::string::String,
)
```
The `documentation` field in `TypeHandle` is just an optional String.
The `ty` field is a RefCell containing an optional atomic reference counted
element of type seedwing_policy_engine::lang::mir::Type` which is an enum:
```console
(gdb) ptype seedwing_policy_engine::lang::mir::Type
type = enum seedwing_policy_engine::lang::mir::Type {
  Anything,
  Primordial(seedwing_policy_engine::lang::PrimordialType),
  Ref(seedwing_policy_engine::lang::SyntacticSugar, usize, alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::mir::TypeHandle>, alloc::alloc::Global>),
  Deref(alloc::sync::Arc<seedwing_policy_engine::lang::mir::TypeHandle>),
  Argument(alloc::string::String),
  Const(seedwing_policy_engine::lang::lir::ValueType),
  Object(seedwing_policy_engine::lang::mir::ObjectType),
  Expr(alloc::sync::Arc<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Expr>>),
  List(alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::mir::TypeHandle>, alloc::alloc::Global>),
  Nothing,
}
```
Next let take a look at `define_primordial` function:
```console
(gdb) f
#1  0x0000555555abb6ef in seedwing_policy_engine::lang::mir::World::new () at seedwing-policy-engine/src/lang/mir/mod.rs:216
216	        this.define_primordial("integer", PrimordialType::Integer);
(gdb) s
gdb) with listsize 30 -- l define_primordial
224	    fn define_primordial(&mut self, name: &str, ty: PrimordialType) {
225	        let name = TypeName::new(None, name.into());
226	
227	        let ty = Arc::new(TypeHandle::new_with(
228	            Some(name.clone()),
229	            Located::new(mir::Type::Primordial(ty), 0..0),
230	        ));
231	
232	        self.type_slots.push(ty);
233	        self.types.insert(name, self.type_slots.len() - 1);
234	    }
```
And `PrimordialType` is en enum with the following variants:
```console
(gdb) ptype seedwing_policy_engine::lang::PrimordialType
type = enum seedwing_policy_engine::lang::PrimordialType {
  Integer,
  Decimal,
  Boolean,
  String,
  Function(seedwing_policy_engine::lang::SyntacticSugar, seedwing_policy_engine::runtime::TypeName, alloc::sync::Arc<dyn seedwing_policy_engine::core::Function>),
}
```
Whenever I don't know the package name of a type I use `info types type_name`:
```console
(gdb) info types TypeName
```
And then I can inspect that output and figure out the package name. In this case
TypeName is:
```console
(gdb) ptype seedwing_policy_engine::runtime::TypeName
type = struct seedwing_policy_engine::runtime::TypeName {
  package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>,
  name: alloc::string::String,
}
```
And `PackagePath` we covered previously.
Next, in `define_primordial` we are creating a new Arc with a new TypeHandle:
```console
(gdb) l
223	
224	    fn define_primordial(&mut self, name: &str, ty: PrimordialType) {
225	        let name = TypeName::new(None, name.into());
226	
227	        let ty = Arc::new(TypeHandle::new_with(
228	            Some(name.clone()),
229	            Located::new(mir::Type::Primordial(ty), 0..0),
230	        ));
231	
232	        self.type_slots.push(ty);
```
The most interesting call here is `TypeHandle::new_with` which will create a 
new TypeHandle with the `seedwing_policy_engine::lang::mir::Type` which we
might recall is an enum and in this case the Primordial variant is used:
```console
(gdb) ptype seedwing_policy_engine::lang::mir::Type::Primordial 
type = struct seedwing_policy_engine::lang::mir::Type::Primordial (
  seedwing_policy_engine::lang::PrimordialType,
)
```
So Primordial is a tuple struct with one member:
```console
(gdb) p ty.inner 
$6 = seedwing_policy_engine::lang::mir::Type::Primordial(seedwing_policy_engine::lang::PrimordialType::Integer)
(gdb) p ty.inner.0
$7 = seedwing_policy_engine::lang::PrimordialType::Integer
```
Back in `define_primordial` we will then add `ty` to the `type_slots` vector:
```
(gdb) f
#0  seedwing_policy_engine::lang::mir::World::define_primordial (self=0x7fffffff4ce0, name="integer", ty=...) at seedwing-policy-engine/src/lang/mir/mod.rs:232
232	        self.type_slots.push(ty);
```
Just printing `ty` will provide a lot of data which can be hard read. One way
around is to "look" into the Arc and at the data that it points to:
```console
(gdb) p (*ty.ptr.pointer).data
$21 = seedwing_policy_engine::lang::mir::TypeHandle {name: core::option::Option<seedwing_policy_engine::runtime::TypeName>::Some(seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::None, name: "integer"}), documentation: core::option::Option<alloc::string::String>::None, ty: RefCell(borrow=0) = {value = core::option::Option<alloc::sync::Arc<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type>>>::Some(Arc(strong=1, weak=0) = {value = seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type> {inner: seedwing_policy_engine::lang::mir::Type::Primordial(seedwing_policy_engine::lang::PrimordialType::Integer), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}, strong = 1, weak = 0}), borrow = 0}, parameters: Vec(size=0)}
```
And we can always use `ptype` if we need to remind ourselves of what a struct
looks like:
```console
(gdb) ptype seedwing_policy_engine::lang::mir::TypeHandle
type = struct seedwing_policy_engine::lang::mir::TypeHandle {
  name: core::option::Option<seedwing_policy_engine::runtime::TypeName>,
  documentation: core::option::Option<alloc::string::String>,
  ty: core::cell::RefCell<core::option::Option<alloc::sync::Arc<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type>>>>,
  parameters: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<alloc::string::String>, alloc::alloc::Global>,
}
```
And if we wanted to inspect the value of ty in ty:
```console
(gdb) p ((*ty.ptr.pointer).data.ty.value.value.0.ptr.pointer).data
$39 = seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type> {inner: seedwing_policy_engine::lang::mir::Type::Primordial(seedwing_policy_engine::lang::PrimordialType::Integer), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}
```
Next step is to insert the `name` into the `types` hashmap:
```console
(gdb) n
233	        self.types.insert(name, self.type_slots.len() - 1)
```
So after this `types` will contains the TypeName for "Integer" as it's key and
the value will be 0, which means that the TypeHandle for this TypeName can be
found a position 0 of the `type_slots` vector:
```console
(gdb) p *(self.type_slots.buf.ptr.pointer.pointer + 0)
$46 = Arc(strong=1, weak=0) = {value = seedwing_policy_engine::lang::mir::TypeHandle {name: core::option::Option<seedwing_policy_engine::runtime::TypeName>::Some(seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::None, name: "integer"}), documentation: core::option::Option<alloc::string::String>::None, ty: RefCell(borrow=0) = {value = core::option::Option<alloc::sync::Arc<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type>>>::Some(Arc(strong=1, weak=0) = {value = seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::mir::Type> {inner: seedwing_policy_engine::lang::mir::Type::Primordial(seedwing_policy_engine::lang::PrimordialType::Integer), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}, strong = 1, weak = 0}), borrow = 0}, parameters: Vec(size=0)}, strong = 1, weak = 0}
```
After that we are back in seedwing_policy_engine::lang::mir::World::new
```console
(gdb) with listsize 39 -- l seedwing_policy_engine::lang::mir::World::new
184	    ($obj: expr, $name: literal, $primordial: expr) => {
185	        let name = TypeName::new(None, $name.into());
186	        $obj.insert(
187	            name.clone(),
188	            Arc::new(TypeHandle::new_with(
189	                Some(name),
190	                Located::new(mir::Type::Primordial($primordial), 0..0),
191	            )),
192	        );
193	    };
194	}
195	
196	#[derive(Debug)]
197	pub struct World {
198	    type_slots: Vec<Arc<TypeHandle>>,
199	    types: HashMap<TypeName, usize>,
200	}
201	
202	impl World {
203	    pub(crate) fn new() -> Self {
204	        //let mut initial_types = HashMap::new();
205	
206	        //primordial_type!(initial_types, "integer", PrimordialType::Integer);
207	        //primordial_type!(initial_types, "string", PrimordialType::String);
208	        //primordial_type!(initial_types, "boolean", PrimordialType::Boolean);
209	        //primordial_type!(initial_types, "decimal", PrimordialType::Decimal);
210	
211	        let mut this = Self {
212	            type_slots: vec![],
213	            types: Default::default(),
214	        };
215	
216	        this.define_primordial("integer", PrimordialType::Integer);
217	        this.define_primordial("string", PrimordialType::String);
218	        this.define_primordial("boolean", PrimordialType::Boolean);
219	        this.define_primordial("decimal", PrimordialType::Decimal);
220	
221	        this
222	    }
```
And we can see that the same thing will be done for `string`, `boolean`, and
`decimal` types.
That brings up back into `Lowerer::lower`.
```console
(gdb) f
#0  seedwing_policy_engine::lang::hir::Lowerer::lower (self=...) at seedwing-policy-engine/src/lang/hir/mod.rs:463
463	        let mut errors = Vec::new();
```
Next we are now going to iterate over all the CompilationUnits.
```console
(gdb) n
465	        for mut unit in self.units.iter_mut() {
(gdb) n
466	            let unit_path = PackagePath::from(unit.source());
(gdb) p unit.source 
$48 = seedwing_policy_engine::lang::parser::SourceLocation {name: "x509::oid"}
```
Recall what a CompilationUnit looks like:
```console
(gdb) ptype seedwing_policy_engine::lang::parser::CompilationUnit
type = struct seedwing_policy_engine::lang::parser::CompilationUnit {
  source: seedwing_policy_engine::lang::parser::SourceLocation,
  uses: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::parser::Use>, alloc::alloc::Global>,
  types: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::TypeDefn>, alloc::alloc::Global>,
}
```
We are now going to iterate over all the `use` statements that are in the first
CompilationUnit
```console
468	            let mut visible_types = unit
469	                .uses()
470	                .iter()
471	                .map(|e| (e.as_name().inner(), Some(e.type_name())))
472	                .chain(unit.types().iter().map(|e| {
473	                    (
474	                        e.name().inner(),
475	                        Some(Located::new(
476	                            TypeName::new(None, e.name().inner()),
477	                            e.location(),
478	                        )),
479	                    )
480	                }))
481	                .collect::<HashMap<String, Option<Located<TypeName>>>>();
```
Recall that map returns a new iterator that processes the the modified items.
So for every `use` we will map the item to a tuple containing the `name` and
the Some(TypeName). Then the types of the unit are also iterated over and the
items are also mapped to a tuple and finally all the those tuples are used to
create a HashMap which is then returned from collect.

Lets take a look at `visible_types` at this stage:
```console
(gdb) pipe p visible_types | egrep -o '\["[a-z]*"\]'
["common"]
["organization"]
```
We will be using this again so we might want to create a custom gdb command for
it:
```console
(gdb) define visible_types
Type commands for definition of "visible_types".
End with a line saying just "end".
>pipe p visible_types | egrep -o '\["[a-z]*"\]'
>end
```

Recall that these are types, `common` and `organization` object types which were
defined in oid.dog using:
```
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
So thoese become types which can be used in rule later.

Next, we have the following:
```console
483	            //visible_types.insert("int".into(), None);
484	            for primordial in world.known_world() {
485	                visible_types.insert(primordial.name(), None);
486	            }
```
Notice that this is adding the primordial types to the hash visiable_types
HashMap. Currently, we have the following keys in this HashMap:
```console
(gdb) visible_types
["common"]
["organization"]
["integer"]
["decimal"]
["string"]
["boolean"]
```
What we are doing is adding the types which will be available to evaulate the
rules.

<a id="interation-compilation-unit"></a> 
Next, we have an iteration over the CompilationUnit::types:
```console
(gdb) f
#0  seedwing_policy_engine::lang::hir::Lowerer::lower (self=...) at seedwing-policy-engine/src/lang/hir/mod.rs:488
488	            for defn in unit.types() {
(gdb) l 488,+8
488	            for defn in unit.types() {
489	                visible_types.insert(
490	                    defn.name().inner(),
491	                    Some(Located::new(
492	                        unit_path.type_name(defn.name().inner()),
493	                        defn.location(),
494	                    )),
495	                );
496	            }
```
Hmm, this looks odd to me...it looks like we are adding the types again. If we
look pack to the code which iterated over the `uses` we also added the `types`:
```rust
gdb) l 468
463	        let mut errors = Vec::new();
464	
465	        for mut unit in self.units.iter_mut() {
466	            let unit_path = PackagePath::from(unit.source());
467	
468	            let mut visible_types = unit
469	                .uses()
470	                .iter()
471	                .map(|e| (e.as_name().inner(), Some(e.type_name())))
472	                .chain(unit.types().iter().map(|e| {
(gdb) 
473	                    (
474	                        e.name().inner(),
475	                        Some(Located::new(
476	                            TypeName::new(None, e.name().inner()),
477	                            e.location(),
478	                        )),
479	                    )
480	                }))
481	                .collect::<HashMap<String, Option<Located<TypeName>>>>();
482	
```
I looks like the second iteration could be removed if the insert in the `chain`
above used the `unit_path` instead. I'll open a pull request and see if this
correct.


__work in progress__

```rust
464	
465	        for mut unit in self.units.iter_mut() {
466	            let unit_path = PackagePath::from(unit.source());
467	
468	            let mut visible_types = unit
469	                .uses()
470	                .iter()
471	                .map(|e| (e.as_name().inner(), Some(e.type_name())))
472	                .chain(unit.types().iter().map(|e| {
473	                    (
474	                        e.name().inner(),
475	                        Some(Located::new(
476	                            TypeName::new(None, e.name().inner()),
477	                            e.location(),
478	                        )),
479	                    )
480	                }))
481	                .collect::<HashMap<String, Option<Located<TypeName>>>>();
482	
483	            //visible_types.insert("int".into(), None);
484	            for primordial in world.known_world() {
485	                visible_types.insert(primordial.name(), None);
486	            }
487	
488	            for defn in unit.types() {
489	                visible_types.insert(
490	                    defn.name().inner(),
491	                    Some(Located::new(
492	                        unit_path.type_name(defn.name().inner()),
493	                        defn.location(),
494	                    )),
495	                );
496	            }
497	
498	            for defn in unit.types() {
499	                let referenced_types = defn.referenced_types();
500	
501	                for ty in &referenced_types {
502	                    if !ty.is_qualified() && !visible_types.contains_key(&ty.name()) {
503	                        errors.push(BuildError::TypeNotFound(
504	                            unit.source().clone(),
505	                            ty.location().span(),
506	                            ty.clone().as_type_str(),
507	                        ))
508	                    }
509	                }
510	            }
511	
512	            for defn in unit.types_mut() {
513	                defn.qualify_types(&visible_types)
514	            }
515	        }
516	
517	        // next, perform inter-unit linking.
518	
519	        let mut known_world = world.known_world();
520	
521	        //world.push(TypeName::new(None, "int".into()));
522	
523	        //world.push("int".into());
524	
525	        for package in self.packages.iter() {
526	            let package_path = package.path();
527	
528	            known_world.extend_from_slice(
529	                &package
530	                    .function_names()
531	                    .iter()
532	                    .map(|e| package_path.type_name(e.clone()))
533	                    .collect::<Vec<TypeName>>(),
534	            );
535	        }
536	
537	        for unit in self.units.iter() {
538	            let unit_path = PackagePath::from(unit.source());
539	
540	            let unit_types = unit
541	                .types()
542	                .iter()
543	                .map(|e| unit_path.type_name(e.name().inner()))
544	                .collect::<Vec<TypeName>>();
545	
546	            known_world.extend_from_slice(&unit_types);
547	        }
548	
549	        if !errors.is_empty() {
550	            return Err(errors);
551	        }
552	
553	        for unit in self.units.iter() {
554	            for defn in unit.types() {
555	                // these should be fully-qualified now
556	                let referenced = defn.referenced_types();
557	
558	                for each in referenced {
559	                    if !known_world.contains(&each.clone().inner()) {
560	                        errors.push(BuildError::TypeNotFound(
561	                            unit.source().clone(),
562	                            each.location().span(),
563	                            each.clone().as_type_str(),
564	                        ))
565	                    }
566	                }
567	            }
568	        }
569	
570	        for unit in self.units.iter() {
571	            let unit_path = PackagePath::from(unit.source());
572	
573	            for ty in unit.types() {
574	                let name = unit_path.type_name(ty.name().inner());
575	                world.declare(name, ty.documentation.clone(), ty.parameters());
576	            }
577	        }
578	
579	        for package in self.packages.iter() {
580	            let path = package.path();
581	            for (fn_name, func) in package.functions() {
582	                let path = path.type_name(fn_name);
583	                world.declare(
584	                    path,
585	                    func.documentation(),
586	                    func.parameters()
587	                        .iter()
588	                        .cloned()
589	                        .map(|p| Located::new(p, 0..0))
590	                        .collect(),
591	                );
592	            }
593	        }
594	
595	        if !errors.is_empty() {
596	            return Err(errors);
597	        }
598	
599	        for package in self.packages.iter() {
600	            let path = package.path();
601	            for (fn_name, func) in package.functions() {
602	                let path = path.type_name(fn_name);
603	                world.define_function(path, func);
604	            }
605	        }
606	
607	        for unit in self.units.iter() {
608	            let unit_path = PackagePath::from(unit.source());
609	
610	            for (path, ty) in unit.types().iter().map(|e| {
611	                (
612	                    Located::new(unit_path.type_name(e.name().inner()), e.location()),
613	                    e.ty(),
614	                )
615	            }) {
616	                world.define(path.inner(), ty);
617	            }
618	        }
619	
620	        if errors.is_empty() {
621	            Ok(world)
622	        } else {
623	            Err(errors)
624	        }
625	    }
626	}
```
And these are then passed to `Lowerer::new`
```rust
struct Lowerer<'b> {
    units: &'b mut Vec<CompilationUnit>,
    packages: &'b mut Vec<Package>,
}
```

[chumsky]: https://crates.io/crates/chumsky/0.9.0
[chumsky examples]: https://github.com/danbev/learning-rust/tree/master/chumsky#chumsky
[rust-gdb]: https://github.com/danbev/learning-rust/blob/master/notes/gdb.md
