Micro Parser Combinators
========================

_mpc_ is a lightweight but powerful Parser Combinator library for C.

Using _mpc_ might be of interest to you if you are...

* Building a new programming language
* Building a new data format
* Parsing an existing data format
* Embedding a Domain Specific Language

Features
--------

* Type-Generic Parser Combinators
* Error Message Support
* Regular Expression Support
* Abstract Syntax Tree Support
* Easy to Integrate (One Source File in ANSI C)


Alternatives
------------

The current main alternative is a branch of (https://github.com/wbhart/Cesium3/tree/combinators)[Cesium3].

The main advantages of _mpc_ over this are:

* Works for Generic Types
* Doesn't rely on Boehm-Demers-Weiser Garbage Collection
* Doesn't use `setjmp` and `longjmp` for errors
* Doesn't pollute namespace


View From the Top
-----------------

In this example I specify a grammar for a basic maths language and parse a string from it.

The output is an instance of the included `mpc_ast_t` type.

```c
#include "mpc.h"

mpc_ast_t* parse_maths(const char* input) {

  mpc_parser_t* Expr  = mpc_new("expression");
  mpc_parser_t* Prod  = mpc_new("product");
  mpc_parser_t* Value = mpc_new("value");
  mpc_parser_t* Maths = mpc_new("maths");

  mpc_define(Expr,  mpca_grammar(" <product> (('+' | '-') <product>)* ", Prod));
  mpc_define(Prod,  mpca_grammar(" <value>   (('*' | '/')   <value>)* ", Value));
  mpc_define(Value, mpca_grammar(" /[0-9]+/ | '(' <expression> ')' ", Expr));
  mpc_define(Maths, mpca_total(Expr));
  
  mpc_result_t r;  
  if (!mpc_parse("parse_maths", input, Maths, &r)) {
    mpc_err_print(r.error);
    abort();
  }
  
  mpc_cleanup(4, Expr, Prod, Value, Maths);
  
  return r.output;
}
```

The output for an expression like `(4 * 2 * 11 + 2) + 5` would look something like this

```xml
<root>
    <value>
        <char> '('
        <expression>
            <product>
                <value> '4'
                <char> '*'
                <value> '2'
                <char> '*'
                <value> '11'
            <char> '+'
            <value> '2'
        <char> ')'
    <char> '+'
    <value> '5'
```

View From the Bottom
--------------------

Parser Combinators are structures that encode how to parse a particular language. They can be combined using a number of intuitive operators to create new parsers of ever increasing complexity. With these, complex grammars and languages can be processed easily.

The trick behind Parser Combinators is the observation that by structuring the library in a particular way, one can make building parser combinators, look like writing a grammar itself. Therefore instead of describing _how to parse a language_, a user must only specify _the language itself_, and the computer will work out how to parse it ... as if by magic!

Parsers
-------

The Parser Combinator type in _mpc_ is `mpc_parser_t`. This encodes a function that attempts to parse some string and, if successful, returns a pointer to some data, or otherwise returns some error. A parser can be run using `mpc_parse`.

```c
bool mpc_parse(const char* filename, const char* s, mpc_parser_t* p, mpc_result_t* r);
```

This function returns `true` on success and `false` on failure. It takes as input some parser `p`, string `s`, and some `filename`. It outputs into `r` the result of the parse which is either a pointer to some data object, or an error. The type `mpc_result_t` is a union type defined as follows.

```c
typedef union {
  mpc_err_t* error;
  mpc_val_t* output;
} mpc_result_t;
```

where `mpc_val_t` is synonymous with `void*` and simply represents some pointer to data - the exact type of which is dependant on the parser.


Basic Parsers
-------------

All the following functions return parsers. All of those parsers return strings with the character(s) matched. They have the following functionality.

* `mpc_parser_t* mpc_any(void);` - Matches any character
* `mpc_parser_t* mpc_char(char c);` - Matches a character `c`
* `mpc_parser_t* mpc_range(char s, char e);` - Matches any character in the range `s` to `e`
* `mpc_parser_t* mpc_oneof(const char* s);` - Matches any character in provided string
* `mpc_parser_t* mpc_noneof(const char* s);` - Matches any character not in provided string
* `mpc_parser_t* mpc_satisfy(bool(*f)(char));` - Matches any character satisfying function `f`
* `mpc_parser_t* mpc_string(const char* s);` - Matches string `s`

Several other functions exist that return parsers with special functionality.

* `mpc_parser_t* mpc_pass(void);` - Always is successful and returns `NULL`
* `mpc_parser_t* mpc_fail(void);` - Always fails
* `mpc_parser_t* mpc_lift(mpc_lift_t f);` - Always succeeds and returns the result of function `f`
* `mpc_parser_t* mpc_lift_val(mpc_val_t* x);` - Always succeeds and returns `x`

Combinators
-----------

Combinators are functions that take one or several parsers and return a new one. These combinators are type agnostic - meaning they can be used no matter what type the input parsers are meant to return. In languages such as Haskell ensuring you don't ferry one type of data into a parser requiring a different type of data is done by the compiler. But in C we don't have that luxury, so it is at the discretion of the programmer to ensure that he deals correctly with the output types of different parsers.

A second annoyance in C is that of manual memory management. Some parsers might get half-way and then fail, meaning they need to clean up any partial data that has been collected in the parse. In Haskell this is handled by the Garbage Collector but in C these functions take _destructors_ - functions which clean up and partial data of a given type that has been collected.

Here are some common combinators and how to use then.

```c
mpc_parser_t* mpc_expect(mpc_parser_t* a, const char* expected);
```

Returns a parser that attempts `a` an on failure reports that `expected` was expected.

This is useful for improving the readability of error messages. For example:

* `mpc_or(2, mpc_char('0'), mpc_char('1'))`

might report `expected '0' or '1' at 'x'` while

* `mpc_expect(mpc_or(2, mpc_char('0'), mpc_char('1')), "binary digit")`

will report `expected binary digit at 'x'`.

```c
mpc_parser_t* mpc_apply(mpc_parser_t* a, mpc_apply_t f);
mpc_parser_t* mpc_apply_to(mpc_parser_t* a, mpc_apply_to_t f, void* x);
```

Applies function `f` to the result of parser `a`.
Applies function `f`, taking extra input `x`, to the result of parser `a`.

```c
mpc_parser_t* mpc_not(mpc_parser_t* a, mpc_dtor_t da);
mpc_parser_t* mpc_not_else(mpc_parser_t* a, mpc_dtor_t da, mpc_lift_t lf);
```

Returns a parser with the following behaviour:
  * If parser `a` succeeds, the output parser fails.
  * If parser `a` fails, the output parser succeeds and returns `NULL` or the result of lift function `lf`.

Destructor `da` is used to destroy the result of `a`.

```c
mpc_parser_t* mpc_maybe(mpc_parser_t* a);
mpc_parser_t* mpc_maybe_else(mpc_parser_t* a, mpc_lift_t lf);
```

Attempts to parser `a`. If this fails then succeeds and returns `NULL` or the result of `lf`.

```c
mpc_parser_t* mpc_many(mpc_parser_t* a, mpc_fold_t f);
mpc_parser_t* mpc_many_else(mpc_parser_t* a, mpc_fold_t f, mpc_lift_t lf);
```

Attempts to parse zero or more `a`. If zero instances are found then succeeds and returns `NULL` or the result of `lf`.

If more than zero instances are found results of `a` are combined using fold function `f`. See the _Function Types_ section for more details.

```c
mpc_parser_t* mpc_many1(mpc_parser_t* a, mpc_fold_t f);
```

Attempts to parse one or more `a`. Results are combined with fold function `f`.

```c
mpc_parser_t* mpc_count(mpc_parser_t* a, mpc_dtor_t da, mpc_fold_t f, int n);
mpc_parser_t* mpc_count_else(mpc_parser_t* a, mpc_dtor_t da, mpc_fold_t f, int n, mpc_lift_t lf);
```

Attempts to parse exactly `n` of `a`. If it fails the result output by the fold function `f` is destructed with `da` and either it returns `NULL` or the result of lift function `lf`.

Results of `a` are combined using fold function `f`.

```c
mpc_parser_t* mpc_else(mpc_parser_t* a, mpc_parser_t* b);
```

Attempts to parse `a` and if fails attempts to parse `b`. If both fail, returns an error.

```c
mpc_parser_t* mpc_also(mpc_parser_t* a, mpc_parser_t* b, mpc_dtor_t da, mpc_fold_t f);
mpc_parser_t* mpc_bind(mpc_parser_t* a, mpc_parser_t* b, mpc_dtor_t da, mpc_fold_t f);
```

Attempts to parse `a` and then attempts to parse `b`. If `b` fails it destructs the result of `a` using `da`. If both succeed it returns the result of `a` and `b` combined using the fold function `f`.

```c
mpc_parser_t* mpc_or(int n, ...);
```

Attempts to parse `n` parsers in sequence, returning the first one that succeeds. If all fail, returns an error.

For example: `mpc_or(3, mpc_char('a'), mpc_char('b'), mpc_char('c'))` would attempt to match either an `'a'` or a `'b'` or a `'c'`.

```c
mpc_parser_t* mpc_and(int n, mpc_afold_t f, ...);
```

Attempts to parse `n` parsers in sequence, returning the fold of them using fold function `f`. Parsers must be specified in series, followed by all the destructors for the types they return minus the last. These are used in case of partial success.

For example: `mpc_and(3, mpcf_astrfold, mpc_char('a'), mpc_char('b'), mpc_char('c'), free, free),` would attempt to match `'a'` followed by `'b'` followed by `'c'` and if successful would concatenate them using `mpcf_astrfold`.


Function Types
--------------

The combinator functions take a number of special function types as function pointers. Here is a short explanation of those types are how they are expected to behave.

```c
typedef void(*mpc_dtor_t)(mpc_val_t*);
```

Destructor function. Given some pointer to a data value it will ensure the memory it points to is freed correctly.

```c
typedef mpc_val_t*(*mpc_apply_t)(mpc_val_t*);
typedef mpc_val_t*(*mpc_apply_to_t)(mpc_val_t*,void*);
```

Application function. This takes in some pointer to data and outputs some new or modified pointer to data, ensuring to free and old data no longer required. The `apply_to` variation takes in an extra pointer to some data such as state of the system.

```c
typedef mpc_val_t*(*mpc_fold_t)(mpc_val_t*,mpc_val_t*);
```

Fold function. This takes two pointers to data and must output some new combined pointer to data, ensuring to free and old data no longer required. When used with the `many`, `many1` and `count` functions this initially takes in `NULL` for it's first argument and following that takes in for it's first argument whatever was previously returned by the function itself. In this way users have a chance to build some initial data structure before populating it with whatever is passed as the second argument.

```c
typedef mpc_val_t*(*mpc_afold_t)(int,mpc_val_t**);
```

AFold Function. Similar to the above but it is passed in a list of pointers to data values which must all be folded together and output as a new single data value.

```c
typedef mpc_val_t*(*mpc_lift_t)(void);
```

Lift Function. This is a simple function that returns some data value when called. It can be used to create _empty_ versions of data types when certain combinators have no known default value to return.

Example
-------

Using the above we can already create a parser that matches a C identifier with relative .

First we build a fold function that will concatenate two strings together.

```c
mpc_val_t* parse_fold_string(mpc_val_t* x, mpc_val_t* y) {
  
  if (x == NULL) { return y; }
  if (y == NULL) { return x; }
  
  char* x = realloc(x, strlen(x) + strlen(y) + 1);
  strcat(x, y);
  
  free(y);
  return x;
  
}
```

Then we can actually specify the parser.

```
char* parse_ident(char* input) {
  
  mpc_parser_t* alpha = mpc_oneof("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
  mpc_parser_t* digit = mpc_oneof("0123456789");
  mpc_parser_t* underscore = mpc_char('_');
  
  mpc_parser_t* ident0 = mpc_else(alpha, underscore);
  mpc_parser_t* ident1 = mpc_many(mpc_or(3, alpha, digit, underscore), parse_fold_string);
  mpc_parser_t* ident = mpc_also(ident0, ident1, free, parse_fold_string);
  
  mpc_result_t r;  
  if (!mpc_parse("parse_ident", input, ident, &r)) {
    mpc_err_print(r.error);
    abort();
  }
  
  mpc_delete(ident);
  
  return r.output;
}
```

Self Reference
--------------

Building parsers in the above way can have issues with self reference and left handed recursion.

To overcome this we separate the construction of parsers into two different steps. First we allocate them...

```c
mpc_parser_t* mpc_new(const char* name);
```

This will construct a parser a parser called `name` which can then be referenced by others including itself when defined. Any parse created using `mpc_new` is said to be _retained_. This means it will behave slightly differently to a normal parser. For example when deleting a parser that includes a _retained_ parser, the _retained_ parser it will not be deleted. To delete a retained parser `mpc_delete` must be used on it directly.

A _retained_ parser can then be defined using...

```c
mpc_parser_t* mpc_define(mpc_parser_t* p, mpc_parser_t* a);
```

This assigns the contents of parser `a` into `p`, and frees and memory involved in constructing `a`. Now parsers can reference each other and themselves without trouble.

```c
mpc_parser_t* mpc_undefine(mpc_parser_t* p);
```

But now parsers that reference each other must all be undefined before they are deleted. It is important to do any defining before deletion. The reason for this is that to delete a parser it must look at each sub-parser that is included in it. If any of these have already been deleted it will segfault.


```c
void mpc_cleanup(int n, ...);
```

To ease the task of undefining and then deleting parsers `mpc_cleanup` can be used. It takes `n` parsers as input and undefines them all, before deleting them all.

Common Parsers
---------------

A number of common parsers have been included.

* `mpc_parser_t* mpc_eoi(void);` - Matches only the end of input
* `mpc_parser_t* mpc_soi(void);` - Matches only the start of input

* `mpc_parser_t* mpc_space(void);` - Matches some whitespace character
* `mpc_parser_t* mpc_spaces(void);` - Matches zero or more whitespace characters
* `mpc_parser_t* mpc_whitespace(void);` - Matches zero or more whitespace characters and frees the result

* `mpc_parser_t* mpc_newline(void);` - Matches `'\n'`
* `mpc_parser_t* mpc_tab(void);` - Matches `'\t'`
* `mpc_parser_t* mpc_escape(void);` - Matches a backslash followed by any character

* `mpc_parser_t* mpc_digit(void);` - Matches any character in the range `'0'` - `'9'`
* `mpc_parser_t* mpc_hexdigit(void);` - Matches any character in the range `'0'` - `'9'` as well as `'A'` - `'F'` or `'a'` - `'f'`
* `mpc_parser_t* mpc_octdigit(void);` - Matches any character in the range `'0'` - `'7'`
* `mpc_parser_t* mpc_digits(void);` - Matches one or more digit
* `mpc_parser_t* mpc_hexdigits(void);` - Matches one or more hexdigit
* `mpc_parser_t* mpc_octdigits(void);` - Matches one or more octdigit

* `mpc_parser_t* mpc_lower(void);` - Matches and lower case character
* `mpc_parser_t* mpc_upper(void);` - Matches any upper case character
* `mpc_parser_t* mpc_alpha(void);` - Matches and alphabet character
* `mpc_parser_t* mpc_underscore(void);` - Matches `'_'`
* `mpc_parser_t* mpc_alphanum(void);` - Matches any alphabet character, underscore or digit

* `mpc_parser_t* mpc_int(void);` - Matches digits and converts to `int*`
* `mpc_parser_t* mpc_hex(void);` - Matches hexdigits and converts to `int*`
* `mpc_parser_t* mpc_oct(void);` - Matches octdigits and converts to `int*`
* `mpc_parser_t* mpc_number(void);` - Matches `mpc_int`, `mpc_hex` or `mpc_oct`

* `mpc_parser_t* mpc_real(void);` - Matches some floating point number
* `mpc_parser_t* mpc_float(void);` - Matches some floating point number and converts to `float*`

* `mpc_parser_t* mpc_semi(void);` - Matches `';'`
* `mpc_parser_t* mpc_comma(void);` - Matches `','`
* `mpc_parser_t* mpc_colon(void);` - Matches `':'`
* `mpc_parser_t* mpc_dot(void);` - Matches `'.'`

* `mpc_parser_t* mpc_char_lit(void);` - Matches some character literal
* `mpc_parser_t* mpc_string_lit(void);` - Matches some string literal
* `mpc_parser_t* mpc_regex_lit(void);` - Matches some regex literal

* `mpc_parser_t* mpc_ident(void);` - Matches a C identifier


Useful Parsers
--------------

* `mpc_parser_t* mpc_start(mpc_parser_t* a);`
* `mpc_parser_t* mpc_end(mpc_parser_t* a, mpc_dtor_t da);`
* `mpc_parser_t* mpc_enclose(mpc_parser_t* a, mpc_dtor_t da);`

* `mpc_parser_t* mpc_skip_many(mpc_parser_t* a, mpc_fold_t f);`
* `mpc_parser_t* mpc_skip_many1(mpc_parser_t* a, mpc_fold_t f);`

* `mpc_parser_t* mpc_strip(mpc_parser_t* a);`
* `mpc_parser_t* mpc_tok(mpc_parser_t* a);` 
* `mpc_parser_t* mpc_sym(const char* s);`
* `mpc_parser_t* mpc_total(mpc_parser_t* a, mpc_dtor_t da);`

* `mpc_parser_t* mpc_between(mpc_parser_t* a, mpc_dtor_t ad, const char* o, const char* c);`
* `mpc_parser_t* mpc_parens(mpc_parser_t* a, mpc_dtor_t ad);`
* `mpc_parser_t* mpc_braces(mpc_parser_t* a, mpc_dtor_t ad);`
* `mpc_parser_t* mpc_brackets(mpc_parser_t* a, mpc_dtor_t ad);`
* `mpc_parser_t* mpc_squares(mpc_parser_t* a, mpc_dtor_t ad);`

* `mpc_parser_t* mpc_tok_between(mpc_parser_t* a, mpc_dtor_t ad, const char* o, const char* c);`
* `mpc_parser_t* mpc_tok_parens(mpc_parser_t* a, mpc_dtor_t ad);`
* `mpc_parser_t* mpc_tok_braces(mpc_parser_t* a, mpc_dtor_t ad);`
* `mpc_parser_t* mpc_tok_brackets(mpc_parser_t* a, mpc_dtor_t ad);`
* `mpc_parser_t* mpc_tok_squares(mpc_parser_t* a, mpc_dtor_t ad);`


Fold Functions
--------------

A number of common fold functions a user might want are included. They reside under the `mpcf_*` namespace.

* `void mpcf_dtor_null(mpc_val_t* x);` - Empty destructor. Does nothing
* `mpc_val_t* mpcf_lift_null(void);` - Returns `NULL`
* `mpc_val_t* mpcf_lift_emptystr(void);` - Returns newly allocated empty string

* `mpc_val_t* mpcf_free(mpc_val_t* x);` - Frees `x` and returns `NULL`
* `mpc_val_t* mpcf_int(mpc_val_t* x);` - Converts a decimal string `x` to an `int*`
* `mpc_val_t* mpcf_hex(mpc_val_t* x);` - Converts a hex string `x` to an `int*`
* `mpc_val_t* mpcf_oct(mpc_val_t* x);` - Converts a oct string `x` to an `int*`
* `mpc_val_t* mpcf_float(mpc_val_t* x);` - Converts a string `x` to a `float*`
* `mpc_val_t* mpcf_escape(mpc_val_t* x);` - Converts a string `x` to an escaped version
* `mpc_val_t* mpcf_unescape(mpc_val_t* x);` - Converts a string `x` to an unescaped version

* `mpc_val_t* mpcf_fst(mpc_val_t* x, mpc_val_t* y);` - Returns `x`
* `mpc_val_t* mpcf_snd(mpc_val_t* x, mpc_val_t* y);` - Returns `y`

* `mpc_val_t* mpcf_fst_free(mpc_val_t* x, mpc_val_t* y);` - Returns `x` and frees `y`
* `mpc_val_t* mpcf_snd_free(mpc_val_t* x, mpc_val_t* y);` - Returns `y` and frees `x`

* `mpc_val_t* mpcf_freefold(mpc_val_t* t, mpc_val_t* x);` - Returns `NULL` and frees `x`
* `mpc_val_t* mpcf_strfold(mpc_val_t* t, mpc_val_t* x);` - Concatenates `t` and `x` and returns result 

* `mpc_val_t* mpcf_afst(int n, mpc_val_t** xs);` - Returns first argument
* `mpc_val_t* mpcf_asnd(int n, mpc_val_t** xs);` - Returns second argument
* `mpc_val_t* mpcf_atrd(int n, mpc_val_t** xs);` - Returns third argument

* `mpc_val_t* mpcf_astrfold(int n, mpc_val_t** xs);` - Concatenates and returns all input strings
* `mpc_val_t* mpcf_between_free(int n, mpc_val_t** xs);` - Frees first and third argument and returns second
* `mpc_val_t* mpcf_maths(int n, mpc_val_t** xs);` - Examines second argument as string to see which operator it is, then operators on first and third argument as if they are `int*`.


Another Example
---------------

Here is another example to show of the stuff learnt so far.

Passing around all these function pointers might seem clumsy, but having parsers be type-generic is important as it lets users define their own syntax tree types as well as perform specific house-keeping or processing in the parsing phase. For example we can specify a simple maths grammar that computes the result of the expression as it goes.

We start with a fold function that will fold `int*` types based on some `char*` operator.

```c
mpc_val_t* mpcf_maths(int n, mpc_val_t** xs) {
  
  int** vs = (int**)xs;
    
  if (strcmp(xs[1], "*") == 0) { *vs[0] *= *vs[2]; }
  if (strcmp(xs[1], "/") == 0) { *vs[0] /= *vs[2]; }
  if (strcmp(xs[1], "%") == 0) { *vs[0] %= *vs[2]; }
  if (strcmp(xs[1], "+") == 0) { *vs[0] += *vs[2]; }
  if (strcmp(xs[1], "-") == 0) { *vs[0] -= *vs[2]; }
  
  free(xs[1]); free(xs[2]);
  
  return xs[0];
}
```

And then we use this to specify how the grammar folds.

```c
int parse_maths(char* input) {

  mpc_parser_t* Expr   = mpc_new("expr");
  mpc_parser_t* Factor = mpc_new("factor");
  mpc_parser_t* Term   = mpc_new("term");
  mpc_parser_t* Maths  = mpc_new("maths");

  mpc_define(Expr, mpc_else(
    mpc_and(3, mpcf_maths, Factor, mpc_oneof("*/"), Factor, free, free),
    Factor
  ));
  
  mpc_define(Factor, mpc_else(
    mpc_and(3, mpcf_maths, Term, mpc_oneof("+-"), Term, free, free),
    Term
  ));
  
  mpc_define(Term, mpc_else(mpc_int(), mpc_parens(Expr, free)));
  mpc_define(Maths, mpc_enclose(Expr, free));
  
  mpc_result_t r;
  if (!mpc_parse("parse_maths", input, Maths, &r)) {
    mpc_err_print(r.error);
    abort();
  }

  int result = *r.output;
  free(r.output);
  
  return result;
}
```

Supplied with something like `(4*2)+5` this will output `13`.


Regular Expressions
-------------------

Even with all that has been explained above, specifying parts of text can be a tedious task requiring many lines of code. So _mpc_ provides a simple regular expression matcher.

```c
mpc_parser_t* mpc_re(const char* re);
```

This returns a parser that will attempt to match the given regular expression pattern, and return the matched string on success. It does not have support for groups and match objects, but should be sufficient for simple tasks.

A cute thing about this is that it uses previous parts of the library to parse the user input string - and because _mpc_ is type generic, the parser spits out a `mpc_parser_t` directly! It even uses many of the combinator functions as fold functions! This is a great case study in learning how to use _mpc_, so those curious are encouraged to find it in the source code.


Abstract Syntax Tree
--------------------

For those that really do not care what data they get out a basic abstract syntax tree type `mpc_ast_t` has been included. Along with this are included some combinator functions which work specifically on this type. They reside under `mpca_*` and you will notice they do not require fold functions or destructors to be specified.

Doing things via this method means that all the data processing must take place after the parsing - but to many this will be preferable. It also allows for one more trick...

If all the fold and destructor functions are implicit then the user can simply specify the grammar in some nice way and the system can try to build an AST for them from this alone.

```c
mpc_parser_t* mpca_grammar(const char* grammar, ...);
```

This can be used to do exactly that. It takes in some grammar, as well as a list of named parsers - and outputs a parser that does exactly what is specified.
