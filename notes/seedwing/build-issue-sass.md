### saas build issue
I ran into this issue when building the frontend of seedwing policy engine, but
I've also run into it on other projects that use trunk.

The error generated is the following:
```console
$ cargo t
   Compiling seedwing-policy-engine v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine)
   Compiling seedwing-policy-server-embedded-frontend v0.1.0-alpha.9 (/home/danielbevenius/work/security/seedwing/seedwing-policy/server/embedded-frontend)
error: failed to run custom build command for `seedwing-policy-server-embedded-frontend v0.1.0-alpha.9 (/home/danielbevenius/work/security/seedwing/seedwing-policy/server/embedded-frontend)`

Caused by:
  process didn't exit successfully: `/home/danielbevenius/work/security/seedwing/seedwing-policy/target/debug/build/seedwing-policy-server-embedded-frontend-c72a1b8ad9bd6cac/build-script-build` (exit status: 101)
  --- stdout
  cargo:rerun-if-changed=build.rs
  cargo:rerun-if-changed=../../frontend/Cargo.toml
  cargo:rerun-if-changed=../../frontend/Cargo.lock
  cargo:rerun-if-changed=../../frontend/Trunk.toml
  cargo:rerun-if-changed=../../frontend/package-lock.json
  cargo:rerun-if-changed=../../frontend/package.json
  cargo:rerun-if-changed=../../frontend/src
  cargo:rerun-if-changed=../../frontend/textmate
  cargo:rerun-if-changed=../../frontend/assets

  --- stderr
  thread 'main' panicked at 'Failed to run 'trunk':
  2023-04-11T05:30:11.194406Z  INFO 📦 starting build
  2023-04-11T05:30:11.197862Z  INFO spawning asset pipelines
  2023-04-11T05:30:11.396244Z  INFO copying directory path="assets/images"
  2023-04-11T05:30:11.396269Z  INFO copying directory path="node_modules/@fortawesome/fontawesome-free/webfonts"
  2023-04-11T05:30:11.396275Z  INFO copying directory path="node_modules/@patternfly/patternfly/assets"
  2023-04-11T05:30:11.396292Z  INFO copying file path="assets/codicon.ttf"
  2023-04-11T05:30:11.396304Z  INFO building seedwing-policy-frontend
  2023-04-11T05:30:11.396962Z  INFO finished copying directory path="assets/images"
  2023-04-11T05:30:11.397124Z  INFO finished copying file path="assets/codicon.ttf"
  2023-04-11T05:30:11.404349Z  INFO finished copying directory path="node_modules/@fortawesome/fontawesome-free/webfonts"
  2023-04-11T05:30:11.429747Z  INFO finished copying directory path="node_modules/@patternfly/patternfly/assets"
  2023-04-11T05:30:11.634942Z  INFO using system installed binary app=sass version=Ruby Sass 3.7.4
  2023-04-11T05:30:11.634965Z  INFO compiling sass/scss path="assets/style.scss"
  2023-04-11T05:30:11.849992Z ERROR ❌ error
  error from HTML pipeline

  Caused by:
      0: error from asset pipeline
      1: sass call returned a bad status

     Compiling seedwing-policy-engine v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/engine)
  OptionParser::InvalidOption: invalid option: --no-source-map
  Did you mean?  sourcemap
    Use --trace for backtrace.
  Error: error from HTML pipeline

  Caused by:
      0: error from asset pipeline
      1: sass call returned a bad status
     Compiling seedwing-policy-frontend v0.1.0 (/home/danielbevenius/work/security/seedwing/seedwing-policy/frontend)
      Finished dev [unoptimized + debuginfo] target(s) in 8.56s
  ', server/embedded-frontend/build.rs:51:9
  note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace
warning: build failed, waiting for other jobs to finish...
```

The cause of this for me was that I have a sass in rvm:
```console
$ which sass
/home/danielbevenius/.rvm/gems/ruby-3.0.0/bin/sass

$ whereis sass
sass: /home/danielbevenius/.rvm/gems/ruby-3.0.0/bin/sass /home/danielbevenius/.nvm/versions/node/v18.14.0/bin/sass
```

Fixing the PATH environment variables to include the Node.js sass will allow
the trunk build to proceed:
```console
$ env PATH=/home/danielbevenius/.nvm/versions/node/v18.14.0/bin:$PATH cargo t
```
