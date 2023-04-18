## Playground refinement issue

The pattern used looks like this:
```
pattern dog = {
    name: string,
    trained: boolean(true)
}
```
The refinment is the `boolean(true)` part.

This results in a json output like this:
```console
{
  "name": {
    "pattern": "playground::dog"
  },
  "input": {
    "name": "goodboy",
    "trained": true
  },
  "output": {
    "name": "goodboy",
    "trained": true
  },
  "severity": "none",

  "reason": "Because all fields were satisfied",
  "rationale": [
    {
      "bindings": {
        "terms": [
          true
        ]
      },
      "input": true,
      "output": true,
      "severity": "none",
      "reason": "The input satisfies the function",
      "rationale": [
        {
          "name": {
            "pattern": "boolean"
          },
          "bindings": {
            "terms": [
              true
            ]
          },
          "input": true,
          "output": true,
          "severity": "none",
          "reason": "The primordial type defined in the pattern is satisfied"
        },
        {
          "name": {
            "pattern": "lang::refine"
          },
          "bindings": {
            "terms": [
              true
            ],
            "refinement": true
          },
          "input": true,
          "output": true,
          "severity": "none",
          "reason": "The input satisfies the function",
          "rationale": [
            {
              "input": true,
              "output": true,
              "severity": "none",
              "reason": "The input matches the expected constant value expected in the pattern"
            }
          ]
        }
      ]
    },
    // name field not shown
  ]
}
```

If we did not have the refinement the output would be:
```
{
  "name": {
    "pattern": "playground::dog"
  },
  "input": {
    "name": "goodboy",
    "trained": true
  },
  "output": {
    "name": "goodboy",
    "trained": true
  },
  "severity": "none",
  "reason": "Because all fields were satisfied",

  "rationale": [
    {
      "name": {
        "field": "trained"
      },
      "input": true,
      "output": true,
      "severity": "none",
      "reason": "The primordial type defined in the pattern is satisfied",
      "rationale": [
        {
          "name": {
            "pattern": "boolean"
          },
          "input": true,
          "output": true,
          "severity": "none",
          "reason": "The primordial type defined in the pattern is satisfied"
        }
      ]
    }
  ]
}
```
So when we have a refinement we seem to loose the name of the field:
```
  "rationale": [
    {
      "name": {
        "field": "trained"
      },
```

The pattern in question is the following:
```
    pattern dog = {
       name: string,
       trained: boolean(true),
    }
```

The parsing of this type is done by the following function:
```rust
pub fn type_definition(
) -> impl Parser<ParserInput, Located<PatternDefn>, Error = ParserError> + Clone {
    metadata()
        .then(
            just("pattern")
                .padded()
                .then(simple_type_name())
                .then(type_parameters().or_not())
                .then_with(move |((_, ty_name), params)| {
                    inner_type_definition(&params)
                        .or_not()
                        .map(move |ty| (ty_name.clone(), params.clone(), ty))
                })
                .map(|(ty_name, params, ty)| {
                    let ty = ty.unwrap_or({
                        let loc = ty_name.location();
                        Located::new(Pattern::Nothing, loc)
                    });

                    let loc = ty_name.span().start()..ty.span().end();
                    Located::new(
                        PatternDefn::new(ty_name, ty, params.unwrap_or_default()),
                        loc,
                    )
                }),
        )
        .map(|(metadata, mut defn)| {
            defn.set_metadata(metadata.into_inner().into());
            defn
        })
}
```
In our case we don't have any metadata annotations on our type(pattern) so we
skip that function/parser.
We do have a name, which is `dog` that is parsed by the `simple_type_name`
parser. This if followed by optional type parameters, but our type does
not use any parameters, which could have been something like:
```
  pattern dog<something> = {
    ...
  }
```
The result from `just("pattern")`, which is just "pattern", the result from
`simple_type_name()`, and the results from `type_parameters`, the `then_with`
function will be called. Note that "pattern" is just ignored using the `_`.

Next we have `inner_type_definition`:
```rust
pub fn inner_type_definition(
    params: &Option<Vec<Located<String>>>,
) -> impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone {
    just("=")
        .padded()
        .ignored()
        .then({
            let visible_parameters: Vec<String> = match params {
                Some(params) => params.iter().map(|e| e.inner()).collect(),
                None => Vec::new(),
            };
            type_expr(visible_parameters)
        })
        .map(|(_, x)| x)
}
```
And we can see that we are parsing the `=` part of the pattern:
```
    pattern dog = {
       name: string,
       trained: boolean(true),
    }
```
Now, the following `then` looked a little strange to me the first time but this
is just a block which returns a parser on the last row.
In our case we did not have any parameters so `params` will be None and
`visible_parameters` will be an empty vector.

Next, `type_expr` will be called passing in the empty params vector:
```rust
pub fn type_expr(
    visible_parameters: Vec<String>,
) -> impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone {
    recursive(|expr| logical_or(expr, visible_parameters.clone()))
}
```
Now `recursive` is a Chumsky function that allows the current parser to be
part, or included, the the patterns it parses.

We can see that we the closure we pass will call `logical_or`:
```rust
pub fn logical_or(
    expr: impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone,
    visible_parameters: Vec<String>,
) -> impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone {
    logical_and(expr.clone(), visible_parameters.clone())
        .then(
            op("||")
                .then(logical_and(expr, visible_parameters))
                .repeated(),
        )
        .map_with_span(|(first, rest), span| {
            if rest.is_empty() {
                first
            } else {
                Located::new(
                    Pattern::Join(
                        once(first)
                            .chain(rest.iter().map(|e| e.1.clone()))
                            .collect(),
                    ),
                    span,
                )
            }
        })
}
```
We can see that first `logical_and` is called:
```rust
pub fn logical_and(
    expr: impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone,
    visible_parameters: Vec<String>,
) -> impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone {
    ty(expr.clone(), visible_parameters)
        .then(op("&&").then(expr).repeated())
        .map_with_span(|(first, rest), span| {
            if rest.is_empty() {
                first
            } else {
                Located::new(
                    Pattern::Meet(
                        once(first)
                            .chain(rest.iter().map(|e| e.1.clone()))
                            .collect(),
                    ),
                    span,
                )
            }
        })
}
```
And notice that this calls `ty` which is bringing us closer to what I think
we are interested about for this particular issue:
```rust
pub fn ty(
    expr: impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone,
    visible_parameters: Vec<String>,
) -> impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone {
    just("!")
        .padded()
        .or_not()
        .then(just("*").padded().repeated())
        .then(
            parenthesized_expr(expr.clone())
                .or(expr_ty())
                .or(list_ty(expr.clone()))
                .or(const_type())
                .or(type_ref(expr.clone(), visible_parameters))
                .or(object_type(expr.clone()))
                .then(postfix(expr).repeated()),
        )
        .map_with_span(|((not, deref), (primary, postfix)), span| {
            println!("primary: {primary:?}");
        ...
```
Now, recall that what the parser is currently "seeing" is the following:
```
    {
       name: string,
       trained: boolean(true),
    }
```
Now a type can have a `!` prefix which denotes the NOT/inverse of what follows,
and it can have padding and is not required (`or_not`). There may also be a
dereference symbol (`*`). After that we have a parser that handles parenthesis
which we don't have in this case. Then it will try to parse an expression, one
in the format like `${something}`.  Next it will try to parse a list type, one
in the format like `[1, 2, 3]` but that is also not the case here. And the
parser will not be able to parse a const type of or a type ref, but it will be
the start of a object, `{`, so the `object_type` parser will handle it.

```rust
pub fn object_type(
    ty: impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone,
) -> impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone {
    println!("object_type");
    just("{")
        .padded()
        .map_with_span(|_, span| span) // _ is just the "{" string
        .then(
            field_definition(ty)
                .separated_by(just(",").padded().ignored())
                .allow_trailing(),
        )
        .then(just("}").padded().map_with_span(|_, span| span))
        ...
}
```

```rust
pub fn field_definition(
    ty: impl Parser<ParserInput, Located<Pattern>, Error = ParserError> + Clone,
) -> impl Parser<ParserInput, Located<Field>, Error = ParserError> + Clone {
    println!("field_definition");
    metadata()
        .then(
            field_name()
                .then(just("?").or_not())
                .then(just(":").labelled("colon").padded().ignored())
                .then(ty)
                .map(|(((name, optional), _), ty)| {
                    let loc = name.span().start()..ty.span().end();
                    Located::new(Field::new(name, ty, optional.is_some()), loc)
                }),
        )
        .map(|(meta, mut field)| {
            field.set_metadata(meta.into_inner().into());
            field
        })
}
```
Note that the field_name parser will called, followed by an optional `?`, and
then a colon.
After that the parser, `ty` that was passed in will be executed by `then(ty)`.
So at this point the parser has parsed:
```
      parsed this so far
      ↓
  name: string,
``` 
Back in `ty` will go through the same proceess, maybe parse a `!`, and then
maybe parse a `*`, and the proceed to parse `string`. This will be a `type_ref`
in object_type:
```rust
                .or(type_ref(expr.clone(), visible_parameters))
```
Since there is nothing else after the type but a comma the field will be
parsed:
```
    pattern dog = {
       name: string,
       ↓
       trained: boolean(true),
    }
```
This would lead us back to field_definition, and first the field_name will be
parsed. And once agains this is parsed as a (or by) type_ref, and the followed
by a colon, then `ty` will run again with this part of the input:
```
    pattern dog = {
       name: string,
                ↓
       trained: boolean(true),
    }
```
Now, this time through the `ty` parser will parse this as a constant type:
```rust
                .or(const_type())
                .then(postfix(expr).repeated()),
```
Followed this time by a postfix which will pick up the refinement `(true)`. 



One thing to keep in mind is that these function are returning parsers. The
println statements I've included will be run when these "parser factory/builder"
functions are called. When the `parse` function is called the actual parsers
will run.

```
    {
       name: string,
       trained: boolean(true),
    }
```
So the starting `{` will cause the object_type parser to be executed. That
parser will parse a field_definition which will be the `name: string` field.
