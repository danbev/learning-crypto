## Seedwing Policy Engine Internals
Is a policy engine, like OPA for example. The policy language in Sweedwing is
called Dogma, and in OPA it is Rego.

# Table of Contents
1. [policy-server walkthrough](#policy-server-walkthrough)
2. [CompilationUnit](#compilationunit)
3. [PolicyParser](#policyparser)
4. [TypeDefn](#typedefn)
5. [hir::Lowerer::lower](#hirlowererlower)
6. [Compiling](compiling)
7. [Evaluate](evaulate)
7. [Bindings](bindings)

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
Let's start with the vector of CompilationUnit's.


### CompilationUnit
CompilationUnit can be found in seedwing-policy-engine/src/lang/parser/mod.rs:
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

Like mentioned in the previous section when a new PolicyBuilder is created
it will call `hir::World::new`:
```rust
impl Builder {
    pub fn new() -> Self {
        Self {
            hir: hir::World::new(),
        }
    }
```
And `hir::World::new` creates a new seedwing_policy_engine::lang::hir::World:
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
        world.add_package(crate::core::list::package());
        world.add_package(crate::core::string::package());
        world.add_package(crate::core::base64::package());
        world.add_package(crate::core::json::package());
        #[cfg(feature = "sigstore")]
        world.add_package(crate::core::sigstore::package());
        world.add_package(crate::core::x509::package());
        world.add_package(crate::core::cyclonedx::package());
        world.add_package(crate::core::jsf::package());
        world.add_package(crate::core::spdx::package());
        world.add_package(crate::core::iso::package());
        world.add_package(crate::core::kafka::package());
        world.add_package(crate::core::pem::package());
        world.add_package(crate::core::net::package());
        world.add_package(crate::core::openvex::package());
        world.add_package(crate::core::osv::package());
        world.add_package(crate::core::uri::package());
        world.add_package(crate::core::timestamp::package());

        #[cfg(feature = "debug")]
        world.add_package(crate::core::debug::package());

        world.add_package(crate::core::maven::package());

        world
    }
```
Notice that initially `units`, which is a vector of CompilationUnits (if you
for get a type you can always use
`ptype seedwing_policy_engine::lang::hir::World`), is empty. The
`world.add_package` functions that follow will populate this vector with
"core" CompilationUnits. These can be found in:
```console
(gdb) shell ls seedwing-policy-engine/src/core/
base64	   data   iso  json   lang  maven   net      osv  sigstore  string     uri  x509
cyclonedx  debug  jsf  kafka  list  mod.rs  openvex  pem  spdx	    timestamp  vex
```

First, lets look at the `crate::core::lang::package()` call.
```rust
        world.add_package(crate::core::lang::package());
```
```console
(gdb) l seedwing_policy_engine::core::lang::package 
16	mod not;
17	mod or;
18	mod refine;
19	mod traverse;
20	
21	pub fn package() -> Package {
22	    let mut pkg = Package::new(PackagePath::from_parts(vec!["lang"]));
23	    pkg.register_function("and".into(), And);
24	    pkg.register_function("or".into(), Or);
25	    pkg.register_function("refine".into(), Refine);
(gdb) 
26	    pkg.register_function("traverse".into(), Traverse);
27	    pkg.register_function("chain".into(), Chain);
28	    pkg.register_function("not".into(), Not);
29	    pkg.register_function("map".into(), Map);
30	    pkg
31	}
```
So what does `Package` look like?
We can use `info types` to see what types named `Package` exist:
```console
(gdb) info types Package
...
seedwing_policy_engine::package::Package
...
```
Looking at the above output we can then use `ptype` see what it looks like:
```console
(gdb) ptype seedwing_policy_engine::package::Package
type = struct seedwing_policy_engine::package::Package {
  path: seedwing_policy_engine::runtime::PackagePath,
  functions: std::collections::hash::map::HashMap<alloc::string::String, alloc::sync::Arc<dyn seedwing_policy_engine::core::Function>, std::collections::hash::map::RandomState>,
  sources: alloc::vec::Vec<seedwing_policy_engine::package::PackageSource, alloc::alloc::Global>,
}
```
*Side-note*:
Tab-completion works pretty well in gdb so it can be used to efficiently write
commands. One thing I've found though is that gdb crashed often if I don't
provide two `e` when table complating the seedwing package.

So we can see that Package has a path:
```console
(gdb) ptype seedwing_policy_engine::runtime::PackagePath
type = struct seedwing_policy_engine::runtime::PackagePath {
  is_absolute: bool,
  path: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global>,
}
```
So a `PackagePath` can have a vector of path's which we will see more about later.
We can take a look at the Package `pkg` created on the first line:
```console
(gdb) p pkg
$14 = seedwing_policy_engine::package::Package {path: seedwing_policy_engine::runtime::PackagePath {is_absolute: true, path: Vec(size=1) = {seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName> {inner: seedwing_policy_engine::runtime::PackageName ("lang"), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}}}, functions: HashMap(size=0), sources: Vec(size=0)}
```
We can see that it has a `path` vector of size 1:
```console
(gdb) p pkg.path.path.len
$16 = 1
```
And we can dig down into the vector to inspect the element:
```console
(gdb) p pkg.path.path.buf.ptr.pointer.pointer.inner
$15 = seedwing_policy_engine::runtime::PackageName ("lang")
```
And we can see the functions HashMap is initially empty:
```console
(gdb) p pkg.functions 
$19 = HashMap(size=0)
```
And sources is initially an empty vector:
```console 
(gdb) p pkg.sources 
$20 = Vec(size=0)
```
Alright, so lets take a look at `pgk.register_function`:
```console
(gdb) f
#0  seedwing_policy_engine::core::lang::package () at seedwing-policy-engine/src/core/lang/mod.rs:23
23	    pkg.register_function("and".into(), And);
```
Let take a look at `register_function`:
```console
(gdb) l seedwing_policy_engine::package::Package::register_function
29	
30	    pub fn path(&self) -> PackagePath {
31	        self.path.clone()
32	    }
33	
34	    pub fn register_function<F: Function + 'static>(&mut self, name: String, func: F) {
35	        self.functions.insert(name, Arc::new(func));
36	    }
```
Like before we can use `info types Function` to figure out where `Function`
comes from (which module). And the use ptype to inspect further:
```console
(gdb) ptype seedwing_policy_engine::core::Function
type = namespace seedwing_policy_engine::core::Function
```
Well that does not give as very much so lets try to list the source of it
instead:
```
(gdb) l seedwing-policy-engine/src/core/mod.rs:87
82	            supporting: vec![],
83	        }
84	    }
85	}
86	
87	pub trait Function: Sync + Send + Debug {
88	    /// A number between 0 and u8::MAX indicating the evaluation order.
89	    ///
90	    /// 0 means the function is likely to be fast, 255 means likely to be slow.
91	    fn order(&self) -> u8;
(gdb) l
92	
93	    fn documentation(&self) -> Option<String> {
94	        None
95	    }
96	
97	    fn parameters(&self) -> Vec<String> {
98	        Default::default()
99	    }
100	
101	    fn call<'v>(
(gdb) 
102	        &'v self,
103	        input: Arc<RuntimeValue>,
104	        ctx: &'v mut EvalContext,
105	        bindings: &'v Bindings,
106	        world: &'v World,
107	    ) -> Pin<Box<dyn Future<Output = Result<FunctionEvaluationResult, RuntimeError>> + 'v>>;
108	}
```
So we can see now that Function is a Trait. It takes an input of type
Arc<RuntimeValue> which we can figure out is an enum using
`info types RuntimeValue` and then `ptype`, but much like the Function trait
and enum will also just so a type and not the variants. But we can still
list the sources using tab completion and the `l -`, and `l +`:
```console
(gdb) l seedwing_policy_engine::value::RuntimeValue::as_json
(gdb) l -
241	#[derive(Serialize, Debug, Clone)]
242	pub enum RuntimeValue {
243	    Null,
244	    String(String),
245	    Integer(i64),
246	    Decimal(f64),
(gdb) l
247	    Boolean(bool),
248	    Object(Object),
249	    List(#[serde(skip)] Vec<Arc<RuntimeValue>>),
250	    Octets(Vec<u8>),
251	}
```
It will also be passed an `EvalContext`:
```console
(gdb) ptype  seedwing_policy_engine::lang::lir::EvalContext
type = struct seedwing_policy_engine::lang::lir::EvalContext {
  trace: seedwing_policy_engine::lang::lir::EvalTrace,
}
```
And a `Bindings`, and then a `World` and note that this is not the same
World that we saw earlier:
```console
(gdb) ptype seedwing_policy_engine::runtime::World
type = struct seedwing_policy_engine::runtime::World {
  types: std::collections::hash::map::HashMap<seedwing_policy_engine::runtime::TypeName, usize, std::collections::hash::map::RandomState>,
  type_slots: alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::lir::Type>, alloc::alloc::Global>,
  trace: seedwing_policy_engine::lang::lir::EvalTrace,
}
```
This is a little confusing so lets try to sort this out. We have the following
`World` structs in the engine:
```console
(gdb) info types World
...
seedwing_policy_engine::lang::mir::World;
seedwing_policy_engine::lang::hir::World;
seedwing_policy_engine::runtime::World;
```
So we have one World in `runtime`:
```console
(gdb) ptype seedwing_policy_engine::runtime::World
type = struct seedwing_policy_engine::runtime::World {
  types: std::collections::hash::map::HashMap<seedwing_policy_engine::runtime::TypeName, usize, std::collections::hash::map::RandomState>,
  type_slots: alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::lir::Type>, alloc::alloc::Global>,
  trace: seedwing_policy_engine::lang::lir::EvalTrace,
}
```
And one in `lang::hir`:
```console
(gdb) ptype seedwing_policy_engine::lang::hir::World
type = struct seedwing_policy_engine::lang::hir::World {
  units: alloc::vec::Vec<seedwing_policy_engine::lang::parser::CompilationUnit, alloc::alloc::Global>,
  packages: alloc::vec::Vec<seedwing_policy_engine::package::Package, alloc::alloc::Global>,
  source_cache: seedwing_policy_engine::runtime::cache::SourceCache,
  data_sources: alloc::vec::Vec<alloc::sync::Arc<dyn seedwing_policy_engine::data::DataSource>, alloc::alloc::Global>,
}
```
and one in `lang:mir`:
```console
(gdb) ptype seedwing_policy_engine::lang::mir::World
type = struct seedwing_policy_engine::lang::mir::World {
  type_slots: alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::mir::TypeHandle>, alloc::alloc::Global>,
  types: std::collections::hash::map::HashMap<seedwing_policy_engine::runtime::TypeName, usize, std::collections::hash::map::RandomState>,
}
```



So I diverged a little but I was talking about the `register_function` for this:
```console
23	    pkg.register_function("and".into(), And);
(gdb) s
35	        self.functions.insert(name, Arc::new(func));
```
`And` is struct, which we can take a closer look at:
```console
(gdb) l seedwing_policy_engine::core::lang::and::{impl#0}::call
(gdb) l -
14	#[derive(Debug)]
15	pub struct And;
16	
17	impl Function for And {
18	    fn order(&self) -> u8 {
19	        128
20	    }
21	    fn parameters(&self) -> Vec<String> {
22	        vec![TERMS.into()]
23	    }
```
So we can see that `And` implements the `Function` trait. I'm going to defer
the `call` function implementation until looking closer at the evaulate process
of the engine.
Alright, so we are adding that function to the packages functions hashmap.
After this we will be back in `seedwing_policy_engine::core::lang::package`
which will add the following functions:
```console
(gdb) l seedwing_policy_engine::core::lang::package:21,+10
21	pub fn package() -> Package {
22	    let mut pkg = Package::new(PackagePath::from_parts(vec!["lang"]));
23	    pkg.register_function("and".into(), And);
24	    pkg.register_function("or".into(), Or);
25	    pkg.register_function("refine".into(), Refine);
26	    pkg.register_function("traverse".into(), Traverse);
27	    pkg.register_function("chain".into(), Chain);
28	    pkg.register_function("not".into(), Not);
29	    pkg.register_function("map".into(), Map);
30	    pkg
31	}
```
And the do the same for these functions, so lets return:
```console
(gdb) finish
(gdb) f
#0  0x00005555558df485 in seedwing_policy_engine::lang::hir::World::new () at seedwing-policy-engine/src/lang/hir/mod.rs:338
338	        world.add_package(crate::core::lang::package());
```
So we will now pass that Package returned to `world.add_package` (and the is
the World in seedwing_policy_engine::lang::hir::World):
```
(gdb) l seedwing_policy_engine::lang::hir::World::add_package:400,411
409	    pub fn add_package(&mut self, package: Package) {
410	        self.packages.push(package);
411	    }
```
And the above is adding the package to the `packages` vector, and self here is
World. After this we will be back in
seedwing_policy_engine::lang::hir::World::new:
```console
(gdb) l 339,+25
339	        world.add_package(crate::core::list::package());
340	        world.add_package(crate::core::string::package());
341	        world.add_package(crate::core::base64::package());
342	        world.add_package(crate::core::json::package());
343	        #[cfg(feature = "sigstore")]
344	        world.add_package(crate::core::sigstore::package());
345	        world.add_package(crate::core::x509::package());
346	        world.add_package(crate::core::cyclonedx::package());
347	        world.add_package(crate::core::jsf::package());
348	        world.add_package(crate::core::spdx::package());
349	        world.add_package(crate::core::iso::package());
350	        world.add_package(crate::core::kafka::package());
351	        world.add_package(crate::core::pem::package());
352	        world.add_package(crate::core::net::package());
353	        world.add_package(crate::core::openvex::package());
354	        world.add_package(crate::core::osv::package());
355	        world.add_package(crate::core::uri::package());
356	        world.add_package(crate::core::timestamp::package());
357	
358	        #[cfg(feature = "debug")]
359	        world.add_package(crate::core::debug::package());
360	
361	        world.add_package(crate::core::maven::package());
362	
363	        world
364	    }
```
And we are adding those packages in the same way. So before we return from this
function `world` looks like this:
```console
(gdb) p world.data_sources.len
$8 = 0

(gdb) p world.units.len 
$5 = 0

(gdb) p world.source_cache.cache
$6 = HashMap(size=0)

(gdb) p world.packages.len
$7 = 18
```
So the only memeber that is populated at this stage is `packages`.
```console
(gdb) n
(gdb) 
(gdb) l
19	impl Builder {
20	    pub fn new() -> Self {
21	        Self {
22	            hir: hir::World::new(),
23	        }
24	    }
25	
```
And after returning from this we have returned from `PolicyBuilder::new()`.
So we are now back in seedwing_policy_server::main.
```console
(gdb) l 57,+18
57	    let mut builder = PolicyBuilder::new();
58	    let mut sources = Vec::new();
59	    if let Some(directories) = matches.get_many::<String>("dir") {
60	        for dir in directories {
61	            let dir = PathBuf::from(dir);
62	            if !dir.exists() {
63	                log::error!("Unable to open directory: {}", dir.to_string_lossy());
64	                exit(-3);
65	            }
66	            sources.push(Directory::new(dir));
67	        }
68	    }
69	
70	    //log::info!("loading policies from {}", dir);
71	    for source in sources.iter() {
72	        if let Err(result) = builder.build(source.iter()) {
73	            errors.extend_from_slice(&result);
74	        }
75	    }
```
This will first gather the directories specified on the command line, if any,
and store them in vector `sources`.  This vector will then be iterated over
and for each item `builder.build` will be called:
```console
(gdb) l seedwing_policy_engine::lang::builder::Builder::new
15	        Self::new()
16	    }
17	}
18	
19	impl Builder {
20	    pub fn new() -> Self {
21	        Self {
22	            hir: hir::World::new(),
23	        }
24	    }
(gdb) l
25	
26	    pub fn build<S, SrcIter>(&mut self, sources: SrcIter) -> Result<(), Vec<BuildError>>
27	    where
28	        Self: Sized,
29	        S: Into<String>,
30	        SrcIter: Iterator<Item = (SourceLocation, S)>,
31	    {
32	        self.hir.build(sources)
33	    }
34	
```
And this just delegates to `seedwing_policy_engine::lang::hir::World::build`:
```console
(gdb) l seedwing_policy_engine::lang::hir::World::build:370,399
370	    pub fn build<S, SrcIter>(&mut self, sources: SrcIter) -> Result<(), Vec<BuildError>>
371	    where
372	        Self: Sized,
373	        S: Into<String>,
374	        SrcIter: Iterator<Item = (SourceLocation, S)>,
375	    {
376	        let mut errors = Vec::new();
377	        for (source, stream) in sources {
378	            log::info!("loading policies from {}", source);
379	
380	            let input = stream.into();
381	
382	            self.source_cache.add(source.clone(), input.clone().into());
383	            let unit = PolicyParser::default().parse(source.clone(), input);
384	            match unit {
385	                Ok(unit) => self.add_compilation_unit(unit),
386	                Err(err) => {
387	                    for e in err {
388	                        errors.push((source.clone(), e).into())
389	                    }
390	                }
391	            }
392	        }
393	
394	        if errors.is_empty() {
395	            Ok(())
396	        } else {
397	            Err(errors)
398	        }
399	    }
```
We can see that the sources are iterated over and get parsed by
[PolicyParser](#policyparser)

### PolicyParser
PolicyParser is used to parse source policies. It might help to get familiar
with [Chumsky examples] to fully understand this section.

So lets see a concrete examples by start a debugging session:
```console
$ rust-gdb --args target/debug/seedwing-policy-server
Reading symbols from target/debug/seedwing-policy-server...

(gdb) br seedwing_policy_engine::lang::parser::PolicyParser::parse
Breakpoint 1 at 0x2c2be3: file seedwing-policy-engine/src/lang/parser/mod.rs, line 276.

(gdb) r
Breakpoint 1, seedwing_policy_engine::lang::parser::PolicyParser::parse<alloc::boxed::Box<dyn core::iter::traits::iterator::Iterator<Item=(char, core::ops::range::Range<usize>)>, alloc::alloc::Global>, seedwing_policy_engine::lang::parser::SourceLocation, alloc::string::String> (self=0x7fffffff6170, source=..., stream="\n\n\n/// Matches OID of 85.4.3, common name.\npattern common<pattern> = {\n  oid: \"85.4.3\",\n  value: pattern,\n}\n\n/// Matches OID of 85.4.10, organization name.\npattern organization<pattern> = {\n  oid: \"85"...) at seedwing-policy-engine/src/lang/parser/mod.rs:276
276	        let tokens = lexer().parse(stream)?;
```
So lets start by listing the `parse` function:
```console
(gdb) l seedwing_policy_engine::lang::parser::PolicyParser::parse:263,291
264	    pub fn parse<'a, Iter, Src, S>(
265	        &self,
266	        source: Src,
267	        stream: S,
268	    ) -> Result<CompilationUnit, Vec<ParserError>>
269	    where
270	        Self: Sized,
271	        Iter: Iterator<Item = (ParserInput, <ParserError as Error<ParserInput>>::Span)> + 'a,
272	        Src: Into<SourceLocation> + Clone,
273	        S: Into<Stream<'a, ParserInput, <ParserError as Error<ParserInput>>::Span, Iter>>,
274	    {
275	        let tokens = lexer().parse(stream)?;
276	        let tokens = remove_comments(&tokens);
277	        let (compilation_unit, errors) = compilation_unit(source).parse_recovery_verbose(
278	            Stream::from_iter(tokens.len()..tokens.len() + 1, tokens.iter().cloned()),
279	        );
280	
281	        if !errors.is_empty() {
282	            Err(errors)
283	        } else if let Some(compilation_unit) = compilation_unit {
284	            Ok(compilation_unit)
285	        } else {
286	            Err(vec![ParserError::custom(
287	                0..0,
288	                "Unable to parse; no further details available",
289	            )])
290	        }
291	    }
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
I'm not listing the tokens vector, as it is quite long, but this can be done.
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
So let's take a look at `compilation_unit`.
```rust
(gdb) l seedwing_policy_engine::lang::parser::compilation_unit:366,390
366	pub fn compilation_unit<S>(
367	    source: S,
368	) -> impl Parser<ParserInput, CompilationUnit, Error = ParserError> + Clone
369	where
370	    S: Into<SourceLocation> + Clone,
371	{
372	    use_statement()
373	        .padded()
374	        .repeated()
375	        .then(type_definition().padded().repeated())
376	        .then_ignore(end())
377	        .map(move |(use_statements, types)| {
378	            let mut unit = CompilationUnit::new(source.clone().into());
379	
380	            for e in use_statements {
381	                unit.add_use(e)
382	            }
383	
384	            for e in types {
385	                unit.add_type(e)
386	            }
387	
388	            unit
389	        })
390	}
```
[Chumsky] is a parser combinator and the seedwing_policy_engine has parsers
defined that handle the various tokens in the Dogma language.  `use_statement()`
returns a Chumsky parser:
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
parser returned from this function. `padding` says that it is alright to have
whitespace characters before and/or after the `use` sequence.

The `then` parser will cause this parser to yield a tuple of the first parser,
`use_statement()`, and the second which is `type_definition()`. For example,
a .dog file can specify zero or more `use` statement(s), followed by one or more
type definitions.

The tuple produced is passed to `.map` where we can see that a new
CompilationUnit is created in `complation_unit`, and all the use statement
(if there are any) are added to the compilation unit. Likewise, the types are
also added, and finally the unit is returned.

Now, the type of these `use` statements are
`seedwing_policy_engine::lang::hir::TypeDefn` which will be in the next section.

### TypeDefn
`types` is a field in a CompilationUnit and we can see that it is a vector of
`Typedefn`:
```console
(gdb) ptype seedwing_policy_engine::lang::hir::TypeDefn
type = struct seedwing_policy_engine::lang::hir::TypeDefn {
  name: seedwing_policy_engine::lang::parser::Located<alloc::string::String>,
  ty: seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::lang::hir::Type>,
  parameters: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<alloc::string::String>, alloc::alloc::Global>,
  documentation: core::option::Option<alloc::string::String>,
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
them. To recap a little, we could create a
`seedwing_policy_engine::lang::builder::Builder`, then add sources by complating
`build`, and/or add `data` sources, and then call `finish`.
```console
(gdb) l seedwing-policy-engine/src/lang/builder.rs:8
3	use crate::lang::parser::SourceLocation;
4	use crate::runtime;
5	use crate::runtime::cache::SourceCache;
6	use crate::runtime::BuildError;
7	
8	#[derive(Clone)]
9	pub struct Builder {
10	    hir: hir::World,
11	}
12	
```
And then we would add sources by calling `build`, and/or add `data` sources, and
then call `finish`:
```console
(gdb) l seedwing_policy_engine::lang::builder::Builder::finish:35,39
35	    pub async fn finish(&mut self) -> Result<runtime::World, Vec<BuildError>> {
36	        let mir = self.hir.lower()?;
37	        let runtime = mir.lower()?;
38	        Ok(runtime)
39	    }
```
`self` in this case is `seedwing_policy_engine::lang::hir::World` so its `lower`
function will be called:
```console
(gdb) l seedwing_policy_engine::lang::hir::World::lower:413,448
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
428	                        self.add_compilation_unit(unit);
429	                    }
430	                    Err(err) => {
431	                        for e in err {
432	                            errors.push((source.clone(), e).into())
433	                        }
434	                    }
435	                }
436	            }
437	        }
438	
439	        for unit in core_units {
440	            self.add_compilation_unit(unit);
441	        }
442	
443	        if !errors.is_empty() {
444	            return Err(errors);
445	        }
446	
447	        Lowerer::new(&mut self.units, &mut self.packages).lower()
448	    }
```
We have seen most of this previously, like `add_package`, and
`PolicyParser::parse` so lets turn our attention to
[hir::Lowerer::lower](#hirlowererlower).

### hir::Lowerer::lower
After the package sources have been parsed, the `CompilationUnit`'s are added to
the `seedwing_policy_engine::lang::hir::World` instance which is done in
`World::lower`.

The final thing to happen in `World::lower` is that `Lowerer::lower` is called,
passing in the `CompilationUnit`'s, and the `Packages`:
```console
(gdb) l seedwing_policy_engine::lang::hir::Lowerer::lower:461,626
461	    pub fn lower(self) -> Result<mir::World, Vec<BuildError>> {
462	        // First, perform internal per-unit linkage and type qualification
463	        let mut world = mir::World::new();
464	        let mut errors = Vec::new();
465	
466	        for unit in self.units.iter_mut() {
467	            let unit_path = PackagePath::from(unit.source());
468	
469	            let mut visible_types = unit
470	                .uses()
471	                .iter()
472	                .map(|e| (e.as_name().inner(), Some(e.type_name())))
473	                .chain(unit.types().iter().map(|e| {
474	                    (
475	                        e.name().inner(),
476	                        Some(Located::new(
477	                            TypeName::new(None, e.name().inner()),
478	                            e.location(),
479	                        )),
480	                    )
481	                }))
482	                .collect::<HashMap<String, Option<Located<TypeName>>>>();
483	
484	            //visible_types.insert("int".into(), None);
485	            for primordial in world.known_world() {
486	                visible_types.insert(primordial.name(), None);
487	            }
488	
489	            for defn in unit.types() {
490	                visible_types.insert(
491	                    defn.name().inner(),
492	                    Some(Located::new(
493	                        unit_path.type_name(defn.name().inner()),
494	                        defn.location(),
495	                    )),
496	                );
497	            }
498	
499	            for defn in unit.types() {
500	                let referenced_types = defn.referenced_types();
501	
502	                for ty in &referenced_types {
503	                    if !ty.is_qualified() && !visible_types.contains_key(&ty.name()) {
504	                        errors.push(BuildError::TypeNotFound(
505	                            unit.source().clone(),
506	                            ty.location().span(),
507	                            ty.clone().as_type_str(),
508	                        ))
509	                    }
510	                }
511	            }
512	
513	            for defn in unit.types_mut() {
514	                defn.qualify_types(&visible_types)
515	            }
516	        }
517	
518	        // next, perform inter-unit linking.
519	
520	        let mut known_world = world.known_world();
521	
522	        //world.push(TypeName::new(None, "int".into()));
523	
524	        //world.push("int".into());
525	
526	        for package in self.packages.iter() {
527	            let package_path = package.path();
528	
529	            known_world.extend_from_slice(
530	                &package
531	                    .function_names()
532	                    .iter()
533	                    .map(|e| package_path.type_name(e.clone()))
534	                    .collect::<Vec<TypeName>>(),
535	            );
536	        }
537	
538	        for unit in self.units.iter() {
539	            let unit_path = PackagePath::from(unit.source());
540	
541	            let unit_types = unit
542	                .types()
543	                .iter()
544	                .map(|e| unit_path.type_name(e.name().inner()))
545	                .collect::<Vec<TypeName>>();
546	
547	            known_world.extend_from_slice(&unit_types);
548	        }
549	
550	        if !errors.is_empty() {
551	            return Err(errors);
552	        }
553	
554	        for unit in self.units.iter() {
555	            for defn in unit.types() {
556	                // these should be fully-qualified now
557	                let referenced = defn.referenced_types();
558	
559	                for each in referenced {
560	                    if !known_world.contains(&each.clone().inner()) {
561	                        errors.push(BuildError::TypeNotFound(
562	                            unit.source().clone(),
563	                            each.location().span(),
564	                            each.clone().as_type_str(),
565	                        ))
566	                    }
567	                }
568	            }
569	        }
570	
571	        for unit in self.units.iter() {
572	            let unit_path = PackagePath::from(unit.source());
573	
574	            for ty in unit.types() {
575	                let name = unit_path.type_name(ty.name().inner());
576	                world.declare(name, ty.documentation.clone(), ty.parameters());
577	            }
578	        }
579	
580	        for package in self.packages.iter() {
581	            let path = package.path();
582	            for (fn_name, func) in package.functions() {
583	                let path = path.type_name(fn_name);
584	                world.declare(
585	                    path,
586	                    func.documentation(),
587	                    func.parameters()
588	                        .iter()
589	                        .cloned()
590	                        .map(|p| Located::new(p, 0..0))
591	                        .collect(),
592	                );
593	            }
594	        }
595	
596	        if !errors.is_empty() {
597	            return Err(errors);
598	        }
599	
600	        for package in self.packages.iter() {
601	            let path = package.path();
602	            for (fn_name, func) in package.functions() {
603	                let path = path.type_name(fn_name);
604	                world.define_function(path, func);
605	            }
606	        }
607	
608	        for unit in self.units.iter() {
609	            let unit_path = PackagePath::from(unit.source());
610	
611	            for (path, ty) in unit.types().iter().map(|e| {
612	                (
613	                    Located::new(unit_path.type_name(e.name().inner()), e.location()),
614	                    e.ty(),
615	                )
616	            }) {
617	                world.define(path.inner(), ty);
618	            }
619	        }
620	
621	        if errors.is_empty() {
622	            Ok(world)
623	        } else {
624	            Err(errors)
625	        }
626	    }
(gdb) 
```
Notice that this function returnes an instance of
`seedwing_policy_engine::lang::hir::World`.
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
Notice that this is adding the primordial types to the hash `visible_types`
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
Hmm, this looks odd to me...it looks like we are adding the types again to the
`visible_types` hashmap. If we look back at the code which iterated over the
`uses`, we also added the `types`:
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
So here we first add the `uses`, followed by the `types` to the `visible_types`
HashMap (the HashMap gets created with those tuples produced by the map
functions). The hashmap uses Strings as its keys.

Lets set a break point on each of these and print out the string that will be
used as the key in the hashmap.

First lets print the length of the unit.types, and unit.uses vectors:
```console
(gdb) dprintf 468, "unit.types.len=%d, unit.uses.len=%d\n", unit.types.len, unit.uses.len
```
And lets also print out the path of the package (just the first two levels):
```console
(gdb) br 468
Breakpoint 19 at 0x5555558ec785: file seedwing-policy-engine/src/lang/hir/mod.rs, line 468.
(gdb) commands
Type commands for breakpoint(s) 19, one per line.
End with a line saying just "end".
>p (*(unit_path.path.buf.ptr.pointer.pointer+0)).inner.__0
>p (*(unit_path.path.buf.ptr.pointer.pointer+1)).inner.__0
>continue
>end
```
Lets set a breakpoint in the `uses` iteration:
```
(gdb) br 471
Note: breakpoint 4 also set at pc 0x5555558d9ece.
Breakpoint 7 at 0x5555558d9ece: file seedwing-policy-engine/src/lang/hir/mod.rs, line 471.
(gdb) command
Type commands for breakpoint(s) 7, one per line.
End with a line saying just "end".
>p e.as_name.inner.inner
>end
```

And then one for the `chain` iterator adapter:
```
(gdb) br 474
Breakpoint 4 at 0x5555558da02e: file seedwing-policy-engine/src/lang/hir/mod.rs, line 474.
(gdb) commands
Type commands for breakpoint(s) 4, one per line.
End with a line saying just "end".
>p (*(e)).inner.name.inner
>end
```
And then finally one in the second iteration of `unit.types`:
```console
(gdb) br 490
Breakpoint 5 at 0x5555558ecc27: file seedwing-policy-engine/src/lang/hir/mod.rs, line 490.
(gdb) commands
Type commands for breakpoint(s) 5, one per line.
End with a line saying just "end".
> p defn.inner.name.inner
>end
```
We can save these breakpoints just incase gdb crashed:
```console
(gdb) save breakpoints use-breakpoints
```
These can then be "loaded' by using `(gdb) source use-breakpoints`

Alright, lets we restart the session, and we will now stop at each breakpoint
and also print out the string that will be used as keys:
```console
(gdb) r
Start it from the beginning? (y or n) y
Starting program: /home/danielbevenius/work/security/seedwing/seedwing-policy/target/debug/seedwing-policy-server 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[2023-02-14T12:00:38Z INFO  seedwing_policy_engine::lang::hir] loading x509::oid
[2023-02-14T12:00:38Z INFO  seedwing_policy_engine::lang::hir] loading x509
[2023-02-14T12:00:38Z INFO  seedwing_policy_engine::lang::hir] loading cyclonedx::v1_4
[2023-02-14T12:00:38Z INFO  seedwing_policy_engine::lang::hir] loading cyclonedx::hash
[2023-02-14T12:00:38Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-14T12:00:38Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-14T12:00:39Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-14T12:00:39Z INFO  seedwing_policy_engine::lang::hir] loading spdx::license
[2023-02-14T12:00:39Z INFO  seedwing_policy_engine::lang::hir] loading iso::swid
[2023-02-14T12:00:39Z INFO  seedwing_policy_engine::lang::hir] loading kafka::opa
[2023-02-14T12:00:39Z INFO  seedwing_policy_engine::lang::hir] loading openvex
[2023-02-14T12:00:39Z INFO  seedwing_policy_engine::lang::hir] loading maven
unit.types.len=2, unit.uses.len=0

Breakpoint 19, seedwing_policy_engine::lang::hir::Lowerer::lower (self=...) at seedwing-policy-engine/src/lang/hir/mod.rs:468
468	            let mut visible_types = unit
$161 = "x509"
$162 = "oid"
Continuing

Breakpoint 1, seedwing_policy_engine::lang::hir::{impl#10}::lower::{closure#1} (e=0x5555585d45e0) at seedwing-policy-engine/src/lang/hir/mod.rs:474
474	                        e.name().inner(),
$160 = "common"
```
So that is the first of item of the unit.types vector to be added to the hashmap
and it is inserted as "common". Notice that this unit does not have and entries
is its `uses` vector.

Next, we continue:
```console
(gdb) c
Continuing.

Breakpoint 4, seedwing_policy_engine::lang::hir::{impl#10}::lower::{closure#1} (e=0x5555585d46c0) at seedwing-policy-engine/src/lang/hir/mod.rs:474
474	                        e.name().inner(),
$25 = "organization"
```
And this is the second entry that gets inserted into the hashmap, this time
using the key "organization".

There are only 2 entries, `unit.types.len=2` above, in the `types` vector for
the current unit, so the next time we continue, we will break in the second
iteration of unit.types:
```console
(gdb) c
Continuing.

Breakpoint 7, seedwing_policy_engine::lang::hir::Lowerer::lower (self=...) at seedwing-policy-engine/src/lang/hir/mod.rs:490
490	                    defn.name().inner(),
$26 = "common"
```
This will overwrite the entry we added above. And if we continue:
```console
(gdb) c
Continuing.

Breakpoint 7, seedwing_policy_engine::lang::hir::Lowerer::lower (self=...) at seedwing-policy-engine/src/lang/hir/mod.rs:490
490	                    defn.name().inner(),
$29 = "organization"
```
And the same things happens here. Unless I'm missing something it looks like the
second iteration could be removed, if the `insert` in the `chain iterator
adapter` above used the `unit_path`. I'll open a
[pull request](https://github.com/seedwing-io/seedwing-policy/pull/69) and see
if this correct.


TODO: go through the rest of this function but for now I'm going to settle
with that it populates the World instance and returnes it.

After hir::lower() returns mir::lower will be called:
```console
(gdb) l seedwing_policy_engine::lang::builder::Builder::finish:35,39
35	    pub async fn finish(&mut self) -> Result<runtime::World, Vec<BuildError>> {
36	        let mir = self.hir.lower()?;
37	        let runtime = mir.lower()?;
38	        Ok(runtime)
39
```
```console
(gdb) ptype seedwing_policy_engine::lang::mir::World
type = struct seedwing_policy_engine::lang::mir::World {
  type_slots: alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::mir::TypeHandle>, alloc::alloc::Global>,
  types: std::collections::hash::map::HashMap<seedwing_policy_engine::runtime::TypeName, usize, std::collections::hash::map::RandomState>,
}
```

```rust
(gdb) l seedwing_policy_engine::lang::mir::World::lower:462,473
462	    pub fn lower(self) -> Result<runtime::World, Vec<BuildError>> {
463	        let mut world = runtime::World::new();
464	
465	        log::info!("Compiling {} patterns", self.types.len());
466	
467	        for (_slot, ty) in self.type_slots.iter().enumerate() {
468	            world.add(ty.name.as_ref().unwrap().clone(), ty.clone());
469	        }
470	
471	        Ok(world)
472	    }
473	}
```
And this will first create a runtime::World:
```console
(gdb) ptype seedwing_policy_engine::runtime::World
type = struct seedwing_policy_engine::runtime::World {
  types: std::collections::hash::map::HashMap<seedwing_policy_engine::runtime::TypeName, usize, std::collections::hash::map::RandomState>,
  type_slots: alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::lang::lir::Type>, alloc::alloc::Global>,
  trace: seedwing_policy_engine::lang::lir::EvalTrace,
}
```
And the lower function will iterate over all the items in `type_slots` with
`enumerate` (but the index is not currently used), and call add for each item:
```console
(gdb) l seedwing_policy_engine::runtime::World::add:432,439
432	    pub(crate) fn add(&mut self, path: TypeName, handle: Arc<TypeHandle>) {
433	        let ty = handle.ty();
434	        let name = handle.name();
435	        let parameters = handle.parameters().iter().map(|e| e.inner()).collect();
436	        let converted = lir::convert(name, handle.documentation(), parameters, &ty);
437	        self.type_slots.push(converted);
438	        self.types.insert(path, self.type_slots.len() - 1);
439	    }
```
`add` will call `seedwing_policy_engine::lang::lir::convert` which contains
a match block for the mir type, and returns a lir type. This returned type is
then added to this World instances `type_slot` vector, and the `types` hashmap
is updated to have the index. When that is done for all types then
mir::World::lower will return the populated runtime::World instance. And
builder::finish will return that World to the caller.

__work in progress__

### Compiling
The previous sections dealt with the building part of the policy engine and
the result of this is an instance of `seedwing_policy_engine::runtime::World`:
```console
(gdb) f
#0  seedwing_policy_server::main::{async_block#0} () at seedwing-policy-server/src/main.rs:90
90	    let result = builder.finish().await;
```
This is world is then cloned and passed to the PlaygroundState. Lets start the
server and then set a break point in the Playground::compile function:
```console
$ rust-gdb --args target/debug/seedwing-policy-server
Reading symbols from target/debug/seedwing-policy-server...
(gdb) br seedwing-policy-server/src/playground.rs:77
Breakpoint 1 at 0x5555557b023e: file seedwing-policy-server/src/playground.rs, line 77.
(gdb) r
...
[2023-02-13T08:30:34Z INFO  seedwing_policy_engine::lang::mir] Compiling 135 patterns
[2023-02-13T08:30:34Z INFO  seedwing_policy_server] starting up at http://0.0.0.0:8080/
```
We can now access the [playground](http://0.0.0.0:8080/playground/) and
enter a rule:
```
pattern something = integer
```
And the press `Compile` which should allow the breakpoint we set to be hit:
```console
Thread 2 "actix-rt|system" hit Breakpoint 29, seedwing_policy_server::playground::{impl#4}::register::compile::{async_fn#0} () at seedwing-policy-server/src/playground.rs:77
77	    match state.build(&content) {
```
And we can take a look at the compile function using:
```console
(gdb) l seedwing_policy_server::playground::{impl#4}::register::compile::{async_fn#0}:66,81
71	) -> HttpResponse {
72	    let mut content = BytesMut::new();
73	    while let Some(Ok(bit)) = body.next().await {
74	        content.extend_from_slice(&bit);
75	    }
76	
77	    match state.build(&content) {
78	        Ok(_) => HttpResponse::Ok().into(),
79	        Err(e) => HttpResponse::BadRequest().body(e.to_string()),
80	    }
81	}
```
And we can inspect `state.build`:
```console
(gdb) l seedwing_policy_server::playground::PlaygroundState::build:26,52
26	    pub fn build(&self, policy: &[u8]) -> Result<PolicyBuilder, String> {
27	        let mut builder = self.builder.clone();
28	        for source in self.sources.iter() {
29	            if let Err(e) = builder.build(source.iter()) {
30	                log::error!("err {:?}", e);
31	                return Err(e
32	                    .iter()
33	                    .map(|b| b.to_string())
34	                    .collect::<Vec<String>>()
35	                    .join(","));
36	            }
37	        }
38	        match core::str::from_utf8(policy) {
39	            Ok(s) => {
40	                if let Err(e) = builder.build(Ephemeral::new("playground", s).iter()) {
41	                    log::error!("unable to build policy [{:?}]", e);
42	                    return Err(format!("Compilation error: {e:?}"));
43	                }
44	            }
45	            Err(e) => {
46	                log::error!("unable to parse [{:?}]", e);
47	                return Err(format!("Unable to parse POST'd input {e:?}"));
48	            }
49	        }
50	        Ok(builder)
51	    }
52	}
```
Now, this looks familiar to what the server did for source directories that were
specified on the command line. 
```console
(gdb) br seedwing_policy_server::playground::PlaygroundState::build
Breakpoint 30 at 0x5555557af2ff: file seedwing-policy-server/src/playground.rs, line 2
(gdb) c
Continuing.

Thread 2 "actix-rt|system" hit Breakpoint 30, seedwing_policy_server::playground::PlaygroundState::build (self=0x7ffff0017840, policy=&[u8](size=27) = {...}) at seedwing-policy-server/src/playground.rs:27
27	        let mut builder = self.builder.clone();
```
In this case we don't have any sources (TODO: how are these sources added?).
```console
(gdb) p self.sources
$6 = Vec(size=0)
```
So lets set a breakpoint further down:
```console
(gdb) br seedwing-policy-server/src/playground.rs:39
Breakpoint 33 at 0x5555557af7fe: file seedwing-policy-server/src/playground.rs, line 39.
(gdb) c
40	                if let Err(e) = builder.build(Ephemeral::new("playground", s).iter()) {
```
First lets inspect `s` which should be the policy we entered:
```console
(gdb) p s
$8 = "pattern something = integer"
```
Next, we have a type that we have not seen before, `Ephemeral`. so lets take a
look at this
type:
```console
(gdb) ptype seedwing_policy_engine::runtime::sources::Ephemeral
type = struct seedwing_policy_engine::runtime::sources::Ephemeral {
  source: seedwing_policy_engine::lang::parser::SourceLocation,
  content: alloc::string::String,
}
```
So we are created a new Ephemeral and calling `.iter` on it and then passing
that into `seedwing_policy_engine::lang::builder::Builder::build`. And the
rest we have already gone through previously in this document.

### Evaluate
In the previous section we discussed compiling a pattern in the playground. This
section will take a look at evaluating a pattern.

Simliar to the previous section, this section will also look at a function in
playground, this time will be the `evaluate` function:
```console
(gdb) l  seedwing_policy_server::playground::{impl#6}::register::evaluate::{async_fn#0}:89,161
95	) -> HttpResponse {
96	    let mut content = BytesMut::new();
97	    while let Some(Ok(bit)) = body.next().await {
98	        content.extend_from_slice(&bit);
99	    }
100	    match serde_json::from_slice::<EvaluateRequest>(&content) {
101	        Ok(body) => match serde_json::from_str::<serde_json::Value>(&body.value) {
102	            Ok(payload) => match state.build(body.policy.as_bytes()) {
103	                Ok(mut builder) => match builder.finish().await {
104	                    Ok(world) => {
105	                        let value = RuntimeValue::from(&payload);
106	                        let mut full_path = "playground::".to_string();
107	                        full_path += &path.replace('/', "::");
108	
109	                        match world
110	                            .evaluate(
111	                                &*full_path,
112	                                value,
113	                                EvalContext::new(
114	                                    seedwing_policy_engine::lang::lir::EvalTrace::Enabled,
115	                                ),
116	                            )
117	                            .await
118	                        {
119	                            Ok(result) => {
120	                                let rationale = Rationalizer::new(&result);
121	                                let rationale = rationale.rationale();
122	
123	                                if result.satisfied() {
124	                                    HttpResponse::Ok().body(rationale)
125	                                } else {
126	                                    HttpResponse::NotAcceptable().body(rationale)
127	                                }
128	                            }
129	                            Err(err) => {
130	                                log::error!("err {:?}", err);
131	                                HttpResponse::InternalServerError().finish()
132	                            }
133	                        }
134	                    }
135	                    Err(e) => {
136	                        log::error!("err {:?}", e);
137	                        let e = e
138	                            .iter()
139	                            .map(|b| b.to_string())
140	                            .collect::<Vec<String>>()
141	                            .join(",");
142	                        HttpResponse::BadRequest().body(e.to_string())
143	                    }
144	                },
145	                Err(e) => {
146	                    log::error!("unable to build policy [{:?}]", e);
147	                    HttpResponse::NotAcceptable().body(e.to_string())
148	                }
149	            },
150	            Err(e) => {
151	                log::error!("unable to parse [{:?}]", e);
152	                HttpResponse::BadRequest()
153	                    .body(format!("Unable to parse POST'd input {}", req.path()))
154	            }
155	        },
156	        Err(e) => {
157	            log::error!("unable to parse [{:?}]", e);
158	            HttpResponse::BadRequest().body(format!("Unable to parse POST'd input {}", req.path()))
159	        }
160	    }
161	}
```
Lets set a breakpoint in the start of this function:
```console
(gdb) br seedwing-policy-server/src/playground.rs:114
Breakpoint 3 at 0x555555748ac5: file seedwing-policy-server/src/playground.rs, line 115.
```
And lets step into `seedwing_policy_engine::runtime::World`:
```rust
(gdb) l
452	    pub async fn evaluate<P: Into<String>, V: Into<RuntimeValue>>(
453	        &self,
454	        path: P,
455	        value: V,
456	        mut ctx: EvalContext,
457	    ) -> Result<EvaluationResult, RuntimeError> {
458	        let value = Arc::new(value.into());
459	        let path = TypeName::from(path.into());
460	        let slot = self.types.get(&path);
461	        if let Some(slot) = slot {
```
So we have `value` which is a RuntimeValue
```console
(gdb) ptype seedwing_policy_engine::value::RuntimeValue
type = enum seedwing_policy_engine::value::RuntimeValue {
  Null,
  String(alloc::string::String),
  Integer(i64),
  Decimal(f64),
  Boolean(bool),
  Object(seedwing_policy_engine::value::Object),
  List(alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::value::RuntimeValue>, alloc::alloc::Global>),
  Octets(alloc::vec::Vec<u8, alloc::alloc::Global>),
}
```
```console
(gdb) p path
$11 = "playground::people"
(gdb) p value
$12 = seedwing_policy_engine::value::RuntimeValue::String(alloc::string::String {vec: alloc::vec::Vec<u8, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<u8, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<u8> {pointer: core::ptr::non_null::NonNull<u8> {pointer: 0x7fffe80d84b0}, _marker: core::marker::PhantomData<u8>}, cap: 3, alloc: alloc::alloc::Global}, len: 3}})
```

```console
(gdb) l
456	        mut ctx: EvalContext,
457	    ) -> Result<EvaluationResult, RuntimeError> {
458	        let value = Arc::new(value.into());
459	        let path = TypeName::from(path.into());
460	        let slot = self.types.get(&path);
461	        if let Some(slot) = slot {
462	            let ty = self.type_slots[*slot].clone();
463	            let bindings = Bindings::default();
464	            ty.evaluate(value.clone(), &mut ctx, &bindings, self).await
465	        } else {
(gdb) p *slot
$37 = 6
```
So we are using the path of `playground::people` and getting the index (slot)
of that path from the `type_slots` hashmap. The index is then used to lookup
the type from the `types` vector.
And notice that we are creating a new Bindings instance as well.
Next we are calling `evaluate` on the type.
It is not clear to me what type is stored in `playground::people` but lets take
a look and see:
```console
(gdb) p (*ty.ptr.pointer).data
```
Reading the output of the above command is hard. But we can use `ptype` to
see the fields that are available:
```console
(gdb) ptype seedwing_policy_engine::lang::lir::Type
type = struct seedwing_policy_engine::lang::lir::Type {
  name: core::option::Option<seedwing_policy_engine::runtime::TypeName>,
  documentation: core::option::Option<alloc::string::String>,
  parameters: alloc::vec::Vec<alloc::string::String, alloc::alloc::Global>,
  inner: seedwing_policy_engine::lang::lir::InnerType,
}
```
Now, if we want to inspect the `name` field we will get an Option type.
```console
gdb) p (*ty.ptr.pointer).data.name
$15 = core::option::Option<seedwing_policy_engine::runtime::TypeName>::Some(seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::Some(seedwing_policy_engine::runtime::PackagePath {is_absolute: true, path: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>> {pointer: core::ptr::non_null::NonNull<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>> {pointer: 0x7ffff00da3d0}, _marker: core::marker::PhantomData<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>>}, cap: 1, alloc: alloc::alloc::Global}, len: 1}}), name: alloc::string::String {vec: alloc::vec::Vec<u8, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<u8, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<u8> {pointer: core::ptr::non_null::NonNull<u8> {pointer: 0x7ffff00da340}, _marker: core::marker::PhantomData<u8>}, cap: 6, alloc: alloc::alloc::Global}, len: 6}}})
```
And we can inspect that output and find the pointer to a string inside:
```console
(gdb) printf "%s\n", 0x7ffff00da340
people
```
The pattern used is this:
```
pattern people = lang::or<*data::from<"people.yaml">>
```

Next, will will call:
```console
ty.evaluate(value.clone(), &mut ctx, &bindings, self).await
```
And notice the `.await` on the end which means that this function returns
a Future.
```console
199	    pub fn evaluate<'v>(
200	        self: &'v Arc<Self>,
201	        value: Arc<RuntimeValue>,
202	        ctx: &'v EvalContext,
203	        bindings: &'v Bindings,
204	        world: &'v World,
205	    ) -> Pin<Box<dyn Future<Output = Result<EvaluationResult, RuntimeError>> + 'v>> {
206	        let trace = ctx.trace(value.clone(), self.clone());
207	        match &self.inner {
```
Notice that this function returns a Pin<Box<dyn Future<...>. So Pin is used when
we have self-references, where we don't want the data to be moved which will
invalidate the pointers to other fields, and here we are pining on the heap so
Box is used (which allocates on the heap).

So this will match on the inner type and stepping we will land on line:
```console
216	            InnerType::Ref(sugar, slot, arguments) => trace.run(Box::pin(async move {
(gdb) l
211	                    self.clone(),
212	                    Rationale::Anything,
213	                    Output::Identity,
214	                ))
215	            })),
216	            InnerType::Ref(sugar, slot, arguments) => trace.run(Box::pin(async move {
217	                #[allow(clippy::ptr_arg)]
218	                fn build_bindings<'b>(
219	                    value: Arc<RuntimeValue>,
220	                    mut bindings: Bindings,
```
The match arm here returns Box::pin, well atleast after going through the
tracing first.

We first have an inner function definition named `build_bindings`. And after
that we have the creating of the Box:pin:
```console
216	            InnerType::Ref(sugar, slot, arguments) => trace.run(Box::pin(async move {
217	                #[allow(clippy::ptr_arg)]
218	                fn build_bindings<'b>(
219	                    value: Arc<RuntimeValue>,
220	                    mut bindings: Bindings,
(gdb) l
221	                    ctx: &'b EvalContext,
222	                    parameters: Vec<String>,
223	                    arguments: &'b Vec<Arc<Type>>,
224	                    world: &'b World,
225	                ) -> Pin<Box<dyn Future<Output = Result<Bindings, RuntimeError>> + 'b>>
226	                {
227	                    Box::pin(async move {
228	                        for (param, arg) in parameters.iter().zip(arguments.iter()) {
229	                            if let InnerType::Ref(_sugar, slot, unresolved_bindings) = &arg.inner {
230	                                if let Some(resolved_type) = world.get_by_slot(*slot) {
```
And notice that what is Boxed and then Pinned, is an async block which will be
turned into a Future by the compiler. And it uses move as it moves the values
for the future context. So we will have a Future in the Box which is pinned and
this means that the code in the async block is not executed at this point but
is passed to `trace.run` function.
```rust
impl<'ctx> TraceHandle<'ctx> {
    fn run<'v>(
        mut self,
        block: Pin<Box<dyn Future<Output = Result<EvaluationResult, RuntimeError>> + 'v>>,
    ) -> Pin<Box<dyn Future<Output = Result<EvaluationResult, RuntimeError>> + 'v>>
    where
        'ctx: 'v,
    {
        if self.start.is_some() {
            Box::pin(async move {
                if let Some(correlation) = self.context.correlation().await {
                    self.context
                        .start(correlation, self.input.clone(), self.ty.clone())
                        .await;
                    let mut result = block.await;
                    let elapsed = self.start.map(|e| e.elapsed());
                    self.context
                        .complete(correlation, self.ty.clone(), &mut result, elapsed)
                        .await;
                    result
                } else {
                    block.await
                }
            })
        } else {
            block
        }
    }
}
```
Notice the call
```rust
                    let mut result = block.await;
```
Lets set a break point in the actual future code:
```console
(gdb) br seedwing-policy-engine/src/lang/lir/mod.rs:228
```
And if we step we will see that we return from this function and then .await
it.
The first thing in the async block was a inner function named `build_bindings`
which will be called in the following line which is the actual first line of
code to be executed in the async block:
```rust
                if let Some(ty) = world.get_by_slot(*slot) {
                    let bindings = build_bindings(
                        value.clone(),
                        bindings.clone(),
                        ctx,
                        ty.parameters(),
                        arguments,
                        world,
                    )
                    .await;
```

Lets look at the first line and try to understand what this block i doing:
```rust
                        for (param, arg) in parameters.iter().zip(arguments.iter()) {
```
This is taking the parameters, and the arguments and zipping them so that we
get a param and it's argument. Now recall that a parameter is some thing that
a function can accept, and an argument is what is actually passed.  If we looks
at the `From` Function implementation we find that parameters is defined as:
```rust
impl Function for From {
    fn parameters(&self) -> Vec<String> {
        vec![PATH.into()]
    }
```

```console
(gdb) printf "%s\n", (*param).vec.buf.ptr.pointer.pointer 
path�

(gdb) p (*(*arg).ptr.pointer).data.inner
(gdb) printf "%s\n", 0x7ffff00db510
people.yaml
```
So we can see that we have `people.yaml` as the param, and `path` as the arg.
```console
(gdb) p (*(*arg).ptr.pointer).data.inner
$51 = seedwing_policy_engine::lang::lir::InnerType::Const(seedwing_policy_engine::lang::lir::ValueType::String(alloc::string::String {vec: alloc::vec::Vec<u8, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<u8, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<u8> {pointer: co
```
So in this case we have a `ValueType`?
```console
(gdb) f
#0  seedwing_policy_engine::lang::lir::{impl#1}::evaluate::{async_block#1}::build_bindings::{async_block#0} () at seedwing-policy-engine/src/lang/lir/mod.rs:287
287	                                bindings.bind(param.clone(), arg.clone())
(gdb) l
282	                                            InnerType::Nothing,
283	                                        )),
284	                                    )
285	                                }
286	                            } else {
287	                                bindings.bind(param.clone(), arg.clone())
288	                            }
```
So notice that in this case all that is happening is that bindings.bind is
called with the param and arg that we showed above. And bind just inserts those
values into its hashmap. So the hashmap will contain `path` as the key.
And after that an Ok result with the Bindings will be returned by this
function (`build_bindings`).
```console
(gdb) l
302	                        world,
303	                    )
304	                    .await;
305	
306	                    let bindings = bindings.unwrap();
307	                    let result = ty.evaluate(value.clone(), ctx, &bindings, world).await?;
```
The type is the following:
```console
(gdb) p (*(ty.ptr.pointer)).data.name
$70 = core::option::Option<seedwing_policy_engine::runtime::TypeName>::Some(seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::Some(seedwing_policy_engine::runtime::PackagePath {is_absolute: true, path: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>> {pointer: core::ptr::non_null::NonNull<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>> {pointer: 0x7ffff021a960}, _marker: core::marker::PhantomData<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>>}, cap: 1, alloc: alloc::alloc::Global}, len: 1}}), name: alloc::string::String {vec: alloc::vec::Vec<u8, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<u8, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<u8> {pointer: core::ptr::non_null::NonNull<u8> {pointer: 0x7ffff021a9b0}, _marker: core::marker::PhantomData<u8>}, cap: 4, alloc: alloc::alloc::Global}, len: 4}}})

(gdb) printf "%s\n", 0x7ffff021a9b0
from
```
So we are calling `evaluate` of the ty which we can find in
seedwing-policy-engine/src/lang/lir/mod.rs:
```console
199	    pub fn evaluate<'v>(
200	        self: &'v Arc<Self>,
201	        value: Arc<RuntimeValue>,
(gdb) l
202	        ctx: &'v EvalContext,
203	        bindings: &'v Bindings,
204	        world: &'v World,
205	    ) -> Pin<Box<dyn Future<Output = Result<EvaluationResult, RuntimeError>> + 'v>> {
206	        let trace = ctx.trace(value.clone(), self.clone());
207	        match &self.inner {
208	            InnerType::Anything => trace.run(Box::pin(async move {
209	                Ok(EvaluationResult::new(
210	                    value.clone(),
211	                    self.clone(),
```
Notice that we this is the same `evaluate` function as before, but this times
the the Type (self.inner) is different:
```console
gdb) p (*(*self).ptr.pointer).data.inner
$77 = seedwing_policy_engine::lang::lir::InnerType::Primordial(seedwing_policy_engine::lang::PrimordialType::Function(seedwing_policy_engine::lang::SyntacticSugar::None, seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::Some(seedwing_policy_engine::runtime::PackagePath {is_absolute: true, path: alloc::vec::Vec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>> {pointer: core::ptr::non_null::NonNull<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>> {pointer: 0x7ffff021a9d0}, _marker: core::marker::PhantomData<seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName>>}, cap: 1, alloc: alloc::alloc::Global}, len: 1}}), name: alloc::string::String {vec: alloc::vec::Vec<u8, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<u8, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<u8> {pointer: core::ptr::non_null::NonNull<u8> {pointer: 0x7ffff021aac0}, _marker: core::marker::PhantomData<u8>}, cap: 4, alloc: alloc::alloc::Global}, len: 4}}}, alloc::sync::Arc<dyn seedwing_policy_engine::core::Function> {ptr: core::ptr::non_null::NonNull<alloc::sync::ArcInner<dyn seedwing_policy_engine::core::Function>> {pointer: *const alloc::sync::ArcInner<dyn seedwing_policy_engine::core::Function> {pointer: 0x7ffff00309a0, vtable: 0x555558538e50}}, phantom: core::marker::PhantomData<alloc::sync::ArcInner<dyn seedwing_policy_engine::core::Function>>}))
```
So we should match the seedwing_policy_engine::lang::lir::InnerType::Primordial
arm:
```console
(gdb) l
335	                        Rationale::InvalidArgument(name.clone()),
336	                        Output::None,
337	                    ))
338	                }
339	            })),
340	            InnerType::Primordial(inner) => match inner {
                    ... match arms for all other PrimordialType's

414	                PrimordialType::Function(_sugar, _name, func) => trace.run(Box::pin(async move {
415	                    let result = func.call(value.clone(), ctx, bindings, world).await?;
416	                    Ok(EvaluationResult::new(
417	                        value.clone(),
418	                        self.clone(),
419	                        Rationale::Function(
420	                            result.output().is_some(),
421	                            result.rationale().map(Box::new),
422	                            result.supporting(),
423	                        ),
424	                        result.output(),
(gdb) l
425	                    ))
426	                })),
427	            },

(gdb) br 415
Breakpoint 4 at 0x55555587555d: /home/danielbevenius/work/security/seedwing/seedwing-policy/seedwing-policy-engine/src/lang/lir/mod.rs:415. (2 locations)
(gdb) c
Continuing.

Thread 2 "actix-rt|system" hit Breakpoint 4, seedwing_policy_engine::lang::lir::{impl#1}::evaluate::{async_block#9} () at seedwing-policy-engine/src/lang/lir/mod.rs:415
415	                    let result = func.call(value.clone(), ctx, bindings, world).await?;
```
So what is `value` in this case?
```console
(gdb) p (*value.ptr.pointer).data
$84 = seedwing_policy_engine::value::RuntimeValue::String(alloc::string::String {vec: alloc::vec::Vec<u8, alloc::alloc::Global> {buf: alloc::raw_vec::RawVec<u8, alloc::alloc::Global> {ptr: core::ptr::unique::Unique<u8> {pointer: core::ptr::non_null::NonNull<u8> {pointer: 0x7ffff00a15a0}, _marker: core::marker::PhantomData<u8>}, cap: 3, alloc: alloc::alloc::Global}, len: 3}})
(gdb) printf "%s\n", 0x7ffff00a15a0
jim
```
And we also have the bindings which were created as we saw above. The world
instance provides access all the types that can be accessed.




Lets continue with the previous example which defined a rule that looked like
this:
```
pattern something = integer
```
And in the [playground](http://0.0.0.0:8080/playground/) we can enter any number
, lets say `18` and then enter `something` as the pattern name. Then we can
press the `Evaluate` button. This should allow are breakpoint be hit:
```console
Thread 2 "actix-rt|system" hit Breakpoint 1, seedwing_policy_server::playground::{impl#6}::register::evaluate::{async_fn#0} () at seedwing-policy-server/src/playground.rs:100
100	    match serde_json::from_slice::<EvaluateRequest>(&content) {
```
If we step-into a few times:
```console
(gdb) f
#0  seedwing_policy_server::playground::{impl#6}::register::evaluate::{async_fn#0} () at seedwing-policy-server/src/playground.rs:102
102	            Ok(payload) => match state.build(body.policy.as_bytes()) {

(gdb) l
97	    while let Some(Ok(bit)) = body.next().await {
98	        content.extend_from_slice(&bit);
99	    }
100	    match serde_json::from_slice::<EvaluateRequest>(&content) {
101	        Ok(body) => match serde_json::from_str::<serde_json::Value>(&body.value) {
102	            Ok(payload) => match state.build(body.policy.as_bytes()) {
103	                Ok(mut builder) => match builder.finish().await {
104	                    Ok(world) => {
105	                        let value = RuntimeValue::from(&payload);
106	                        let mut full_path = "playground::".to_string();
```
This `state.build` is the same function that we discussed in the Compile section
earlier. I found it interesting that this is actually also compiling the policy
again. We can see this by stepping into the build function and printing s the
source variable `s`:
```console
(gdb) p s
$4 = "pattern something = integer"
```
After that we are about to call `builder.finish` which we have also covered
earlier. So lets looks at the next pattern matching clause:
```
(gdb) l 104,134
104	                    Ok(world) => {
105	                        let value = RuntimeValue::from(&payload);
106	                        let mut full_path = "playground::".to_string();
107	                        full_path += &path.replace('/', "::");
108	
109	                        match world
110	                            .evaluate(
111	                                &*full_path,
112	                                value,
113	                                EvalContext::new(
114	                                    seedwing_policy_engine::lang::lir::EvalTrace::Enabled,
115	                                ),
116	                            )
117	                            .await
118	                        {
119	                            Ok(result) => {
120	                                let rationale = Rationalizer::new(&result);
121	                                let rationale = rationale.rationale();
122	
123	                                if result.satisfied() {
124	                                    HttpResponse::Ok().body(rationale)
125	                                } else {
126	                                    HttpResponse::NotAcceptable().body(rationale)
127	                                }
128	                            }
129	                            Err(err) => {
130	                                log::error!("err {:?}", err);
131	                                HttpResponse::InternalServerError().finish()
132	                            }
133	                        }
134	                    }
```
Lets start by inspecting the `payload` variable:
```console
(gdb) p payload
$6 = serde_json::value::Value::Number(serde_json::number::Number {n: "18"})
```
We might have seen RuntimeValue before as a parameter to some function but notes
really looked at it. We can use `info types RuntimeValue` to figure out the
correct module name.
```console
(gdb) ptype seedwing_policy_engine::value::RuntimeValue
type = enum seedwing_policy_engine::value::RuntimeValue {
  Null,
  String(alloc::string::String),
  Integer(i64),
  Decimal(f64),
  Boolean(bool),
  Object(seedwing_policy_engine::value::Object),
  List(alloc::vec::Vec<alloc::sync::Arc<seedwing_policy_engine::value::RuntimeValue>, alloc::alloc::Global>),
  Octets(alloc::vec::Vec<u8, alloc::alloc::Global>),
}
```
If we step-into this function we will land in:
```console
(gdb) f
#0  seedwing_policy_engine::value::json::{impl#1}::from (value=0x7ffff00550b8) at seedwing-policy-engine/src/value/json.rs:41
41	        match value {
```
The source for this can listed using:
```console
(gdb) l seedwing_policy_engine::value::json::{impl#0}::from:15,15
7	    fn from(value: JsonValue) -> Self {
8	        match value {
9	            JsonValue::Null => RuntimeValue::Null,
10	            JsonValue::Bool(inner) => RuntimeValue::Boolean(inner),
11	            JsonValue::Number(inner) => {
12	                if inner.is_f64() {
13	                    RuntimeValue::Decimal(inner.as_f64().unwrap())
14	                } else if inner.is_i64() {
15	                    RuntimeValue::Integer(inner.as_i64().unwrap())
```
In our case this function will return RuntimeValue::Integer:
```console
(gdb) p value
$1 = seedwing_policy_engine::value::RuntimeValue::Integer(18)
```
So we are almost ready to call evaulate:
```console
(gdb) l 104,+15
104	                    Ok(world) => {
105	                        let value = RuntimeValue::from(&payload);
106	                        let mut full_path = "playground::".to_string();
107	                        full_path += &path.replace('/', "::");
108	
109	                        match world
110	                            .evaluate(
111	                                &*full_path,
112	                                value,
113	                                EvalContext::new(
114	                                    seedwing_policy_engine::lang::lir::EvalTrace::Enabled,
115	                                ),
116	                            )
117	                            .await
118	                        {
119	                            Ok(result) => {

(gdb) p path
$4 = actix_web::types::path::Path<alloc::string::String> ("something")

(gdb) p full_path
$5 = "playground::something"
```
Lets now step into `world.evaluate`:
```console
(gdb) l
445	    pub async fn evaluate<P: Into<String>, V: Into<RuntimeValue>>(
446	        &self,
447	        path: P,
448	        value: V,
449	        mut ctx: EvalContext,
450	    ) -> Result<EvaluationResult, RuntimeError> {
451	        let value = Arc::new(value.into());
452	        let path = TypeName::from(path.into());
453	        let slot = self.types.get(&path);
454	        if let Some(slot) = slot {
```
Notice that local new variable `path` is created using
TypeName::from(path.into()):
```console
(gdb) p path
$9 = seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::Some(seedwing_policy_engine::runtime::PackagePath {is_absolute: true, path: Vec(size=1) = {seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName> {inner: seedwing_policy_engine::runtime::PackageName ("playground"), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}}}), name: "something"}
```
And then we are going to 'lookup' the index into the slot_types vector by
getting the index from the `types` hashmap:
```console
(gdb) l 453,458
453	        let slot = self.types.get(&path);
454	        if let Some(slot) = slot {
455	            let ty = self.type_slots[*slot].clone();
456	            let bindings = Bindings::default();
457	            ty.evaluate(value.clone(), &mut ctx, &bindings, self).await
458	        } else {

(gdb) p *slot
$14 = 4

(gdb) p ty.ptr.pointer.data.inner
$29 = seedwing_policy_engine::lang::lir::InnerType::Ref(seedwing_policy_engine::lang::SyntacticSugar::None, 0, Vec(size=0))
(gdb) p ty.ptr.pointer.data.documentation
$30 = core::option::Option<alloc::string::String>::Some("")
(gdb) p ty.ptr.pointer.data.parameters
$31 = Vec(size=0)

(gdb) p bindings
$16 = seedwing_policy_engine::lang::lir::Bindings {bindings: HashMap(size=0)}
```
After this `ty.evaluate` will be called.
```console
(gdb) l seedwing_policy_engine::lang::lir::Type::evaluate:196,204
196	    pub fn evaluate<'v>(
197	        self: &'v Arc<Self>,
198	        value: Arc<RuntimeValue>,
199	        ctx: &'v mut EvalContext,
200	        bindings: &'v Bindings,
201	        world: &'v World,
202	    ) -> Pin<Box<dyn Future<Output = Result<EvaluationResult, RuntimeError>> + 'v>> {
203	        let trace = ctx.trace();
204	        match &self.inner {
```
And here we are matching on the inner type of this Type. Notice that this
function returns a Future. `self.inner` in our case is:
```console
(gdb) p ty.ptr.pointer.data.inner
$29 = seedwing_policy_engine::lang::lir::InnerType::Ref(seedwing_policy_engine::lang::SyntacticSugar::None, 0, Vec(size=0))
```
so the following match block will be executed:
```console
(gdb) l seedwing-policy-engine/src/lang/lir/mod.rs:216,321
216	            InnerType::Ref(sugar, slot, arguments) => Box::pin(async move {
217	                #[allow(clippy::ptr_arg)]
218	                fn build_bindings<'b>(
219	                    value: Arc<RuntimeValue>,
220	                    mut bindings: Bindings,
221	                    ctx: &'b mut EvalContext,
222	                    parameters: Vec<String>,
223	                    arguments: &'b Vec<Arc<Type>>,
224	                    world: &'b World,
225	                ) -> Pin<Box<dyn Future<Output = Result<Bindings, RuntimeError>> + 'b>>
226	                {
227	                    Box::pin(async move {
228	                        for (param, arg) in parameters.iter().zip(arguments.iter()) {
229	                            if let InnerType::Ref(_sugar, slot, unresolved_bindings) = &arg.inner {
230	                                if let Some(resolved_type) = world.get_by_slot(*slot) {
231	                                    if resolved_type.parameters().is_empty() {
232	                                        bindings.bind(param.clone(), resolved_type.clone())
233	                                    } else {
234	                                        let resolved_bindings = build_bindings(
235	                                            value.clone(),
236	                                            bindings.clone(),
237	                                            ctx,
238	                                            resolved_type.parameters(),
239	                                            unresolved_bindings,
240	                                            world,
241	                                        )
242	                                        .await?;
243	                                        bindings.bind(
244	                                            param.clone(),
245	                                            Arc::new(Type::new(
246	                                                resolved_type.name(),
247	                                                resolved_type.documentation(),
248	                                                resolved_type.parameters(),
249	                                                InnerType::Bound(resolved_type, resolved_bindings),
250	                                            )),
251	                                        )
252	                                    }
253	                                }
254	                            } else if let InnerType::Argument(name) = &arg.inner {
255	                                bindings.bind(param.clone(), bindings.get(name).unwrap());
256	                            } else if let InnerType::Deref(_inner) = &arg.inner {
257	                                let result = arg
258	                                    .evaluate(value.clone(), ctx, &Bindings::default(), world)
259	                                    .await?;
260	
261	                                if result.satisfied() {
262	                                    if let Some(output) = result.output() {
263	                                        bindings.bind(param.clone(), Arc::new(output.into()))
264	                                    } else {
265	                                        bindings.bind(
266	                                            param.clone(),
267	                                            Arc::new(Type::new(
268	                                                None,
269	                                                None,
270	                                                Vec::default(),
271	                                                InnerType::Nothing,
272	                                            )),
273	                                        )
274	                                    }
275	                                } else {
276	                                    bindings.bind(
277	                                        param.clone(),
278	                                        Arc::new(Type::new(
279	                                            None,
280	                                            None,
281	                                            Vec::default(),
282	                                            InnerType::Nothing,
283	                                        )),
284	                                    )
285	                                }
286	                            } else {
287	                                bindings.bind(param.clone(), arg.clone())
288	                            }
289	                        }
290	
291	                        Ok(bindings)
292	                    })
293	                };
294	
295	                if let Some(ty) = world.get_by_slot(*slot) {
296	                    let bindings = build_bindings(
297	                        value.clone(),
298	                        bindings.clone(),
299	                        ctx,
300	                        ty.parameters(),
301	                        arguments,
302	                        world,
303	                    )
304	                    .await?;
305	
306	                    let result = ty.evaluate(value.clone(), ctx, &bindings, world).await?;
307	                    if let SyntacticSugar::Chain = sugar {
308	                        Ok(EvaluationResult::new(
309	                            Some(value.clone()),
310	                            self.clone(),
311	                            result.rationale().clone(),
312	                            result.raw_output().clone(),
313	                            trace.done(),
314	                        ))
315	                    } else {
316	                        Ok(result)
317	                    }
318	                } else {
319	                    Err(RuntimeError::NoSuchTypeSlot(*slot))
320	                }
321	            }),
```
Alright, so it is not clear to me what this function does so lets try to sort
it out.
The first thing to notice is that the


So when this function returns, the Future will be awaited, and poll will call
the future:
```console
457	            ty.evaluate(value.clone(), &mut ctx, &bindings, self).await
```
Lets set a break point in the Future:
```console
(gdb) br seedwing-policy-engine/src/lang/lir/mod.rs:295
Breakpoint 5 at 0x555555a705ad: file seedwing-policy-engine/src/lang/lir/mod.rs, line 295.
```

First, we have function definition, `build_bindings`, followed by:
```console
gdb) l seedwing-policy-engine/src/lang/lir/mod.rs:295,304
295	                if let Some(ty) = world.get_by_slot(*slot) {
296	                    let bindings = build_bindings(
297	                        value.clone(),
298	                        bindings.clone(),
299	                        ctx,
300	                        ty.parameters(),
301	                        arguments,
302	                        world,
303	                    )
304	                    .await?;
```
The `ty` returned will be:
```console
(gdb) p ty.ptr.pointer.data
$36 = seedwing_policy_engine::lang::lir::Type {name: core::option::Option<seedwing_policy_engine::runtime::TypeName>::Some(seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::None, name: "integer"}), documentation: core::option::Option<alloc::string::String>::None, parameters: Vec(size=0), inner: seedwing_policy_engine::lang::lir::InnerType::Primordial(seedwing_policy_engine::lang::PrimordialType::Integer)}
```
At this point the body of `build_bindings` will be executed. In this case there
are no parameters so the bindings will just be returned.
This will then recurse and `ty.evaluate` will be called but this time on then
`PrimordialType::Integer`



```console
(gdb) br seedwing-policy-engine/src/lang/lir/mod.rs:228
```

### Bindings
So where are Bindings first encountered?
A DataSource can be added to a builder and this is done when we specify then
`--data` command line option to the `cli` or the `server`.
```rust
(gdb) l seedwing-policy-server/src/main.rs:88,93
88	    if let Some(directories) = matches.get_many::<String>("data") {
89	        for each in directories {
90	            log::info!("loading data from {:?}", each);
91	            builder.data(DirectoryDataSource::new(each.into()));
92	        }
93	    }
```
Lets set a breakpoint there:
```console
(gdb) br seedwing-policy-server/src/main.rs:88
Breakpoint 2 at 0x5555557b8060: file seedwing-policy-server/src/main.rs, line 88.
```
Recall that the `--data` option takes directory and the option can be specified
multiple times. And for each other the data directories a new
DirectoryDataSource will be created and added to the Builder.
```console
(gdb) printf "%s\n", (*each).vec.buf.ptr.pointer.pointer 
sample-data
```
So after this the data_sources in the hir World instance will have been
populated. The next time this is touches in in World::lower.
```console
(gdb) br seedwing-policy-engine/src/lang/hir/mod.rs:415
(gdb) c
Continuing.

Breakpoint 3, seedwing_policy_engine::lang::hir::World::lower (self=0x7fffffff9be8) at seedwing-policy-engine/src/lang/hir/mod.rs:415
415	        self.add_package(crate::core::data::package(self.data_sources.clone()));
```
And this we have seen before, but previously we did not have any data sources
set so the vector was empty. This time we have an entry. If we step-into
`crate::core::data::package` we find:
```console
(gdb) l
7	
8	use crate::core::data::from::From;
9	
10	pub fn package(data_sources: Vec<Arc<dyn DataSource>>) -> Package {
11	    let mut pkg = Package::new(PackagePath::from_parts(vec!["data"]));
12	    pkg.register_function("from".into(), From::new(data_sources));
13	    pkg
14	}
```
If we look at `From::new` we find:
```console
(gdb) l
23	impl From {
24	    pub fn new(data_sources: Vec<Arc<dyn DataSource>>) -> Self {
25	        Self {
26	            data_sources: Arc::new(data_sources),
27	        }
28	    }
29	}
```
Notice that From::new takes the whole vector and then stores in its
`data_sources` vector field. After this `register_function` will add the
type implementing the trait `Function`.
One thing I noticed was the `parameters` function for From:
```console
gdb) l seedwing_policy_engine::core::data::from::{impl#2}::parameters:45,47
45	    fn parameters(&self) -> Vec<String> {
46	        vec![PATH.into()]
47	    }

(gdb) l 16,16
16	const PATH: &str = "path";
```
I don't understand this at the moment but I've seen this `path` in debug output
and it might be relevant. Oh, wait. Parameters are `parameters` to the function
I think. So when we call the function we will specify a path of the file that
we want to get data from.  So I was thinking that the file was specified as
a normal argument to the function, something like:
```
pattern components = *data::from("rule_data.yml")
```
But that is incorrect, as parameter is specifed like a generic type instead like
this:
```
pattern components = *data::from<"rule_data.yml">
```

If we take a closer look at the `From::call` function we can see that it takes
a reference to a `Bindings` as a parameter:
```console
(gdb) l seedwing_policy_engine::core::data::from::{impl#2}::call:49,56
49	    fn call<'v>(
50	        &'v self,
51	        _input: Arc<RuntimeValue>,
52	        _ctx: &'v EvalContext,
53	        bindings: &'v Bindings,
54	        _world: &'v World,
55	    ) -> Pin<Box<dyn Future<Output = Result<FunctionEvaluationResult, RuntimeError>> + 'v>> {
56	        Box::pin(async move {
(gdb) l
57	            if let Some(val) = bindings.get(PATH) {
58	                if let Some(ValueType::String(path)) = val.try_get_resolved_value() {
59	                    println!("DataSource from....{}", path);
60	                    for ds in &*self.data_sources {
61	                        if let Ok(Some(value)) = ds.get(path.clone()) {
62	                            return Ok(Output::Transform(Arc::new(value)).into());
63	                        }
64	                    }
65	                }
66	            }
```
We can see that `path` is:
```console
(gdb) printf "%s\n", path.vec.buf.ptr.pointer.pointer 
rule_data.yml
```
But we we inspect the length of the data_sources vector it is zero:
```console
(gdb) p self.data_sources.ptr.pointer.data.len
$33 = 0
```
Hmm, so looking into this a little closer it turns out that the playground.rs
will clone the `builder` (which contains the hir::World):
```rust
                App::new()
                    .app_data(web::Data::new(world.clone()))
                    .app_data(web::Data::new(monitor.clone()))
                    .app_data(web::Data::new(statistics.clone()))
                    .app_data(web::Data::new(Documentation(raw_docs)))
                    .app_data(web::Data::new(Examples(raw_examples)))
                    .app_data(web::Data::new(PlaygroundState::new(
                        builder.clone(),
                        sources.clone(),
                    )))
```
The problem is that `World::clone` does not clone the data_sources, so the
vector will be left in its default state (zero elements):
```rust
impl Clone for World {
    fn clone(&self) -> Self {
        let mut h = World::new();
        h.packages = self.packages.clone();
        h
    }
}
```
Updating this to also clone the data_sources worked ([PR](https://github.com/seedwing-io/seedwing-policy/pull/83).


```console
(gdb) ptype seedwing_policy_engine::lang::lir::Bindings
type = struct seedwing_policy_engine::lang::lir::Bindings {
  bindings: std::collections::hash::map::HashMap<alloc::string::String, alloc::sync::Arc<seedwing_policy_engine::lang::lir::Type>, std::collections::hash::map::RandomState>,
}
```
So we can see that Bindings has one member which is a HashMap with Strings as
the keys and Type as the values.
Lets verify that the From function gets defined:
```console
(gdb) br 593
Breakpoint 4 at 0x555555a3e64d: file seedwing-policy-engine/src/lang/hir/mod.rs, line 593.
(gdb) commands
Type commands for breakpoint(s) 4, one per line.
End with a line saying just "end".
>printf "%s\n", fn_name.vec.buf.ptr.pointer.pointer
>end
```
_wip_



[chumsky]: https://crates.io/crates/chumsky/0.9.0
[chumsky examples]: https://github.com/danbev/learning-rust/tree/master/chumsky#chumsky
[rust-gdb]: https://github.com/danbev/learning-rust/blob/master/notes/gdb.md
