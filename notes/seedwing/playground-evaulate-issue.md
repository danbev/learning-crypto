## playground evaluate issue ("Option::unwrap() on None value")
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
Pressing the Evaluate button results in the following error:
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

(gdb) r
Starting program: /home/danielbevenius/work/security/seedwing/seedwing-policy/target/debug/seedwing-policy-server 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".

Breakpoint 1, seedwing_policy_server::main::{async_block#0} () at seedwing-policy-server/src/main.rs:55
55	    let mut builder = PolicyBuilder::new();
```
So PolicyBuilder is imported using the following statement:
```rust
use seedwing_policy_engine::lang::builder::Builder as PolicyBuilder;
```
And we can find the source for it in seedwing-policy-engine/src/lang/builder.rs.
If we now step we will land in the contructor function `new` which creates a
new hir::World::new().
```rust
pub struct World {
    units: Vec<CompilationUnit>,
    packages: Vec<Package>,
    source_cache: SourceCache,
    data_sources: Vec<Arc<dyn DataSource>>,
}

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
        ...
     }
}
```
We can see that initially the World instance will be initialized with default
values, and then populated by adding packages.

After that we are back in seedwing_policy_server/src/main.rs:
```
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
The `dir` command line argument specifies a directory to load policies from:
```rust
        .arg(Arg::new("dir").value_name("policy directory").num_args(1..))
```
In this case we did not specify and such directory so we will skip the above
block:
```console
(gdb) p sources
$2 = Vec(size=0)
```
Next we have:
```rust
    if let Some(directories) = matches.get_many::<String>("data") {
        for each in directories {
            log::info!("loading data from {:?}", each);
            builder.data(DirectoryDataSource::new(each.into()));
        }
    }
```
And we did not specify and `data` command line option either:
```rust
        .arg(
            Arg::new("data")
                .long("data")
                .short('d')
                .value_name("data directory")
                .num_args(0..),
        )
```
But note that if we had added a data directory the builder would have been  
updated, this will become important later.

Next in main.rs we have:
```rust
    let result = builder.finish().await;
```
And this call will land in:
```console
(gdb) info source
Current source file is seedwing-policy-engine/src/lang/builder.rs
```
`finish` looks like this:
```rust
    pub async fn finish(&mut self) -> Result<runtime::World, Vec<BuildError>> {
        let mir = self.hir.lower()?;
        let runtime = mir.lower()?;
        Ok(runtime)
    }
```
Let's stick a breakpoint in the function to skip over all the async generated
code:
```console
(gdb) br builder.rs:36
```
I've had gdb crash on my when trying to use tab completion in gdb, so I actually
restarted the debugging session and added the breakpoint.

Now, recall that lower was one of the parent frames that was shown in the
backtrace above so lets step into it.  This call will land in
seedwing-policy-engine/src/lang/hir/mod.rs:414:
```rust
    pub fn lower(&mut self) -> Result<mir::World, Vec<BuildError>> {
        self.add_package(crate::core::data::package(self.data_sources.clone()));
        ...
    }
```
We can check the size of the `data_sources`:
```console
(gdb) p self.data_sources
$1 = Vec(size=0)
```
Lets set a break point in:
```console
(gdb) br seedwing_policy_engine::core::data::package 
Breakpoint 2 at 0x555555a5dfa6: file seedwing-policy-engine/src/core/data/mod.rs, line 11.
(gdb) c
```
One thing to keep in mind is that the server is started on the main thread, but
later when actix is started there will be multiple threads running which means
you might have to select a different thread when adding breakpoints.

So let's take a look at the `package` function in
seedwing-policy-engine/src/core/data/mod.rs:
```rust
pub fn package(data_sources: Vec<Arc<dyn DataSource>>) -> Package {
    let mut pkg = Package::new(PackagePath::from_parts(vec!["data"]));
    pkg.register_function("From".into(), From::new(data_sources));
    pkg
}
```
seedwing-policy-engine/src/package/mod.rs:
```rust
#[derive(Clone)]
pub struct Package {
    path: PackagePath,
    functions: HashMap<String, Arc<dyn Function>>,
    sources: Vec<PackageSource>,
}

impl Package {
    ...

    pub fn register_function<F: Function + 'static>(&mut self, name: String, func: F) {
        self.functions.insert(name, Arc::new(func));
    }

}
```
Lets set a break point in this function so that we can inspect the values:
```console
(gdb) br seedwing_policy_engine::package::Package::register_function
(gdb) c
```
Lets first take a look at `name`:
```console
(gdb) p name
$3 = "From"
```
And then `func`:
```console
(gdb) p func
$2 = seedwing_policy_engine::core::data::from::From {data_sources: Arc(strong=1, weak=0) = {value = Vec(size=0), 
    strong = 1, weak = 0}}
```
And recall that `self.functions` is a hashmap with the name as the key and
an atomic reference counted value as the value. Using these values the 
functions hashmap will be updated by inserting.
```console
(gdb) p self.functions
$4 = HashMap(size=1) = {["From"] = Arc(strong=1, weak=0) = {value = dyn seedwing_policy_engine::core::Function, 
    strong = 1, weak = 0}}
```
After returning from that function, we will be back in `package` and the new
Package instance will be returned. This will then get passed to `add_package`:
```console
        self.add_package(crate::core::data::package(self.data_sources.clone()));
```

```console
    pub fn add_package(&mut self, package: Package) {
        self.packages.push(package);
    }
```
The number of packages before adding this package:
```console
(gdb) p self.packages
$5 = Vec(size=18) = {
...
```
And this is the package we are going to add:
```console
(gdb) p package
$6 = seedwing_policy_engine::package::Package
{path: seedwing_policy_engine::runtime::PackagePath {
      is_absolute: true,
      path: Vec(size=1) = {
        seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName> {
          inner: seedwing_policy_engine::runtime::PackageName ("data"),
            location: seedwing_policy_engine::lang::parser::Location {
              span: core::ops::range::Range<usize> {start: 0, end: 0}
            }
        }
      }
},
functions: HashMap(size=1) = {["From"] = Arc(strong=1, weak=0) = {
      value = dyn seedwing_policy_engine::core::Function, strong = 1, weak = 0}}, sources: Vec(size=0)}
```
And after adding we can inspect the vector of packages:
```console
(gdb) p self.packages
$7 = Vec(size=19) = {
...
seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName> {inner: seedwing_policy_engine::runtime::PackageName ("data"), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}}}, functions: HashMap(size=1) = {["From"] = Arc(strong=1, weak=0) = {
        value = dyn seedwing_policy_engine::core::Function, strong = 1, weak = 0}}, sources: Vec(size=0)}}
}
```
After this we will be back in `seedwing_policy_engine::lang::hir::World::lower`:
```rust
        for pkg in &self.packages {
            for (source, stream) in pkg.source_iter() {
                log::info!("loading {}", source);
                self.source_cache.add(source.clone(), stream.clone().into());
                let unit = PolicyParser::default().parse(source.to_owned(), stream);
                match unit {
                    Ok(unit) => {
                        core_units.push(unit);
                    }
                    Err(err) => {
                        for e in err {
                            errors.push((source.clone(), e).into())
                        }
                    }
                }
            }
        }
```
And lets continue until line 447 which is the last line in the function and this
will show the log::info entry above:
```console
(gdb) until 447
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading x509::oid
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading x509
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading cyclonedx::v1_4
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading cyclonedx::hash
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading jsf
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading spdx::license
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading iso::swid
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading kafka::opa
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading vex::openvex
[2023-02-07T07:52:25Z INFO  seedwing_policy_engine::lang::hir] loading maven
seedwing_policy_engine::lang::hir::World::lower (self=0x7fffffffa9c0) at seedwing-policy-engine/src/lang/hir/mod.rs:447
447	        Lowerer::new(&mut self.units, &mut self.packages).lower()
```
`Lowerer::new` can be found in seedwing-policy-engine/src/lang/hir/mod.rs and
it takes the packages as a parameter.
```rust
struct Lowerer<'b> {
    units: &'b mut Vec<CompilationUnit>,
    packages: &'b mut Vec<Package>,
}

impl<'b> Lowerer<'b> {
    pub fn new(units: &'b mut Vec<CompilationUnit>, packages: &'b mut Vec<Package>) -> Self {
        Self { units, packages }
    }
    ...
}
```
So a new instance of Lowerer is created, and then its `lower` member function
is called:
```rust
    pub fn lower(mut self) -> Result<mir::World, Vec<BuildError>> {
        // First, perform internal per-unit linkage and type qualification
        let mut world = mir::World::new();
        let mut errors = Vec::new();
```
Now, don't be confused about the mir::World, as there are World structs in the
following modules:
```
seedwing_policy_engine::runtime::World
seedwing_policy_engine::lang::hir::World
seedwing_policy_engine::lang::mir::World
```
And notice that in this case we are in the middle intermediate representation
(mir) module in seedwing-policy-engine/src/lang/hir/mod.rs

So the first thing that happens is that `mir::World::new` is called which 
can be found in eedwing-policy-engine/src/lang/mir/mod.rs
```rust
#[derive(Debug)]
pub struct World {
    type_slots: Vec<Arc<TypeHandle>>,
    types: HashMap<TypeName, usize>,
}
```
This World contains a vector, type_slots, with TypeHandle's. The types hashmap
contains the name of the types as the key and the value it index into the
type_slots vector.

TypeHandle looks like this:
```rust
#[derive(Default, Debug)]
pub struct TypeHandle {
    name: Option<TypeName>,
    documentation: Option<String>,
    ty: RefCell<Option<Arc<Located<Type>>>>,
    parameters: Vec<Located<String>>,
}
```
This first thing that happens in seedwing_policy_engine::lang::mir::World::new
is that a new World is initialized:
```rust
impl World {
    pub(crate) fn new() -> Self {

        let mut this = Self {
            type_slots: vec![],
            types: Default::default(),
        };
        ...
}
```
Next, primordial types (the most basic types) are added to the World instance
using `define_primordial`. PrimordialType can be found in
seedwing-policy-engine/src/lang/mod.rs:
```
#[derive(Debug, Clone, Serialize)]
pub enum PrimordialType {
    Integer,
    Decimal,
    Boolean,
    String,
    Function(SyntacticSugar, TypeName, #[serde(skip)] Arc<dyn Function>),
}
```
Now lets take a look at `define_primordial`:
```rust
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
We can inspect the arguments passed:
```console
(gdb) p name
$1 = seedwing_policy_engine::runtime::TypeName {package: core::option::Option<seedwing_policy_engine::runtime::PackagePath>::None, name: "integer"}
(gdb) p ty
$4 = seedwing_policy_engine::lang::PrimordialType::Integer
```
Next, a new TypeHandle is created using the name and PrimordialType (the
paramter ty) using `TypeHandle::new_with`:
```rust
    pub fn new_with(name: Option<TypeName>, ty: Located<mir::Type>) -> Self {
        Self {
            name,
            documentation: None,
            ty: RefCell::new(Some(Arc::new(ty))),
            parameters: vec![],
        }
    }
```
And then that TypeHandle is added to the World's type_slots vector:
```rust
        self.type_slots.push(ty);
        self.types.insert(name, self.type_slots.len() - 1);
```
And notice that `self.types` contains the name of the type and the `index` into
type_slots. Notice that `type_slots` is a vector, and `types` is a hashmap which
contains a name and the indext into the type_slots vector.

So that will return us back into `seedwing_policy_engine::lang::mir::World::new`
and the other primordial types will be added:
```rust
        this.define_primordial("string", PrimordialType::String);
        this.define_primordial("boolean", PrimordialType::Boolean);
        this.define_primordial("decimal", PrimordialType::Decimal);
        this
   }
```
And that will return nu back in seedwing-policy-engine/src/lang/hir/mod.rs
```rust
        let mut world = mir::World::new();  <--- we returned from this
        let mut errors = Vec::new();

```
I'm going to skip over the code that deals with the core compilation units, and
focus on the `data` package/thing:
```console
(gdb) until 580
(gdb) p self.packages
...
seedwing_policy_engine::package::Package {path: seedwing_policy_engine::runtime::PackagePath {is_absolute: true, path: Vec(size=1) = {
        seedwing_policy_engine::lang::parser::Located<seedwing_policy_engine::runtime::PackageName> {inner: seedwing_policy_engine::runtime::PackageName ("data"), location: seedwing_policy_engine::lang::parser::Location {span: core::ops::range::Range<usize> {start: 0, end: 0}}}}}, functions: HashMap(size=1) = {["From"] = Arc(strong=1, weak=0) = {
        value = dyn seedwing_policy_engine::core::Function, strong = 1, weak = 0}}, sources: Vec(size=0)}}
```
This will bring us to:
```rust
        for package in self.packages.iter() {
            let path = package.path();
            for (fn_name, func) in package.functions() {
                let path = path.type_name(fn_name);
                world.declare(
                    path,
                    func.documentation(),
                    func.parameters()
                        .iter()
                        .cloned()
                        .map(|p| Located::new(p, 0..0))
                        .collect(),
                );
            }
        }
```

After this server has started, but before anything else happens everythings
looks like expected.

## Recap
So what is happening is that when the server starts it will create a new
PolicyBuilder and call finish on it:
```rust
    let result = builder.finish().await;
```
This will call `finish`, which will call `hir.lower`.
```rust
    pub async fn finish(&mut self) -> Result<runtime::World, Vec<BuildError>> {
        let mir = self.hir.lower()?;
        let runtime = mir.lower()?;
        Ok(runtime)
    }
```
`hir::lower` will add the `core::data::package`:
```rust
    pub fn lower(&mut self) -> Result<mir::World, Vec<BuildError>> {
        self.add_package(crate::core::data::package(self.data_sources.clone()));
        ...
    }
```
Now, `self.packages` is a vector and after this call an entry in that vector
will have the `data::From` package  in it.

Now, when we call evaluate on the playground, it has a state which contains
the same PolicyBuilder cloned:
```rust
#[derive(Clone)]
pub struct PlaygroundState {
    builder: PolicyBuilder,
    sources: Vec<Directory>,
}
```
And we can see that the PolicyBuilder is cloned:
```rust
                App::new()
                    .app_data(web::Data::new(world.clone()))
                    .app_data(web::Data::new(Documentation(raw_docs)))
                    .app_data(web::Data::new(PlaygroundState::new(
                        builder.clone(),
                        sources.clone(),
                    )))
```

Now, the `evaluate` will also call `builder.finish`:
```rust
#[post("/playground/evaluate/{path:.*}")]
pub async fn evaluate(
    req: HttpRequest,
    state: web::Data<PlaygroundState>,
    path: web::Path<String>,
    mut body: Payload,
) -> HttpResponse {
    ...
    match serde_json::from_slice::<EvaluateRequest>(&content) {
        Ok(body) => {
            match serde_json::from_str::<serde_json::Value>(&body.value) {
                Ok(payload) => {
                    match state.build(body.policy.as_bytes()) {
                        Ok(mut builder) => {

                            match builder.finish().await {

                                Ok(world) => {
                                    let value = RuntimeValue::from(&payload);
```
This will again add the `core::data::package` to the packages, so there will
be two entries the packages vector.

Later when `Lowerer::lower` is called it will iterate over the packages twice,
first calling world.declare, and then in the second iteration call
world.define_function:
```rust
        for package in self.packages.iter() {
            let path = package.path();
            for (fn_name, func) in package.functions() {
                print!("fn_name: {} ", &fn_name);
                let path = path.type_name(fn_name);
                println!("path: {}", &path);
                world.declare(
                    path,
                    func.documentation(),
                    func.parameters()
                        .iter()
                        .cloned()
                        .map(|p| Located::new(p, 0..0))
                        .collect(),
                );
            }
        }

        if !errors.is_empty() {
            return Err(errors);
        }

        for package in self.packages.iter() {
            let path = package.path();
            for (fn_name, func) in package.functions() {
                let path = path.type_name(fn_name);
                world.define_function(path, func);
            }
        }
```
If we have two duplicate packages in the vector of packages...what will happen?
Well, there will be two entries in the type_slots of mir::World, one will be
an entry with a Refcall<None> and the second the one that is updated. But
since the old one is still there it will get added below just that same causing
the error:
```rust
            world.add(ty.name.as_ref().unwrap().clone(), ty.clone());
```

