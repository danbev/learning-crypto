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
not use any parameters, which would have been something like this if it did:
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
In our case we did not have any parameters so `params` will be `None` and
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
Note that the `field_name` parser will be called, followed by an optional `?`,
and then a colon.

After that the parser will call `ty` that was passed in will be executed by
`then(ty)`.  So at this point the parser has parsed:
```
      parsed this so far
      ↓
  name: string,
``` 
Back in `ty, it`will go through the same proceess, maybe parse a `!`, and then
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
This would lead us back to `field_definition`, and first `field_name` will be
parsed, this time to parse the `trained` field name. And once again this is
parsed by `type_ref`, and then followed by a colon, then `ty` will run again
with this part of the input:
```
    pattern dog = {
       name: string,
                ↓
       trained: boolean(true),
    }
```
Now, this time through the `ty` parser will parse this as a type_ref type:
```rust
                .or(type_ref(expr.clone(), visible_parameters))
                .or(object_type(expr.clone()))
                .then(postfix(expr).repeated()),
```
So `type_ref` will parse the `boolean` part of the expression `boolean(true)`:
```
type_ref name: PatternName { package: None, name: "boolean" }
```
So that will have parsed the `boolean` part of the expresssion. We will move
us to:
```
    pattern dog = {
       name: string,
                       ↓
       trained: boolean(true),
    }
```
What I think "SHOULD" happen now is that the `postfix` parser will parse the 
`(true)`. But this does not seem to be the case, when I'm seeing when debugging
is that `const_type` will called. 

This is that the parser look like in `ty`:
```
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
                .debug("efter type_ref")
                .or(object_type(expr.clone()))
                .then(postfix(expr).repeated()),
        )
        .map_with_span(|((not, deref), (primary, postfix)), span| {
```
So `type_ref` has parsed the field correctly, but that should be it, const_type
should not be parsing anything in this case. That is the responsibility of
`postfix`.

Hmm, so `const_type` is called and will call `boolean_literal`:will in 
engine/src/lang/parser/literal.rs (there are two functions named like this and
I first tried updating the other function which obviously did not work).
And if we look at that function we can see that it parses a `just(true)`:
```rust
pub fn boolean_literal(
) -> impl Parser<ParserInput, Located<ValuePattern>, Error = ParserError> + Clone {
    choice((
        just("true").map(|value| {
            println!("boolean_literal......{value:?}");
            ValuePattern::Boolean(true)
        }),
        just("false").map(|_| ValuePattern::Boolean(false)),
    ))
    .map_with_span(Located::new)
}
```
This is interesting, the pattern it matches is `true`, not `(true)` which is
the pattern that we are currently looking at. What I think is happening is that
the current parser is recursive and because it is now handling `(true)` that
is being handled by the "outer" parser. This will then eat the parentheses and
then `true` will be available to the parsers in the list above to handle. This
will not allow `const_type` to handle it, and after that the postfix will be
handled. But in this particulare case I don't thing we want 
parenthesized_expr to handle this and instead let it pass through to postfix
which will add this as a refinement type.

_wip_

So the above parsers will output two tuples, the `~`(not), the `*`(deref),
the primary (which is one of the or's (expr_ty, list_ty, const_type, type_ref,
object_type), and then a postfix (refinement or traversal).

This is then followed by a `map_with_span`:
```rust

        .map_with_span(|((not, deref), (primary, postfix)), span| {
            println!("primary: {primary:?}");
            let mut core = if postfix.is_empty() {
                primary
            } else {
                let mut terms = Vec::new();
                terms.push(primary);

                for each in postfix {
                    match each {
                        Postfix::Refinement(refinement) => {
                            if let Some(refinement) = refinement {
                                println!("refinement: {refinement:?}");
                                terms.push(Located::new(
                                    Pattern::Refinement(Box::new(refinement.clone())),
                                    refinement.location(),
                                ));
                            }
                        }
                        Postfix::Traversal(step) => {
                            terms.push(Located::new(
                                Pattern::Traverse(step.clone()),
                                step.location(),
                            ));
                        }
                    }
                }

                Located::new(Pattern::Chain(terms), span)
            };
```
One thing to keep in mind is that these function are returning parsers. The
println statements I've included will be run when these "parser factory/builder"
functions are called. When the `parse` function is called the actual parsers
will run.
