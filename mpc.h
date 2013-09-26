/*
** mpc - Micro Parser Combinator library for C
** https://github.com/orangeduck/mpc
** Daniel Holden - contact@daniel-holden.com
** Licensed under BSD3
*/
#ifndef mpc_h
#define mpc_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

/*
** Error Type
*/

struct mpc_err_t;
typedef struct mpc_err_t mpc_err_t;

int mpc_err_line(mpc_err_t* x);
int mpc_err_column(mpc_err_t* x);
char mpc_err_unexpected(mpc_err_t* x);
char** mpc_err_expected(mpc_err_t* x, int* num);
char* mpc_err_filename(mpc_err_t* x);

void mpc_err_delete(mpc_err_t* x);
void mpc_err_print(mpc_err_t* x);
void mpc_err_print_to(mpc_err_t* x, FILE* f);
void mpc_err_msg(mpc_err_t* x, char* out, int* outn, int outmax);

/*
** Parsing
*/

typedef void mpc_val_t;

typedef union {
  mpc_err_t* error;
  mpc_val_t* output;
} mpc_result_t;

struct mpc_parser_t;
typedef struct mpc_parser_t mpc_parser_t;

bool mpc_parse(const char* filename, const char* s, mpc_parser_t* p, mpc_result_t* r);
bool mpc_parse_file(const char* filename, FILE* f, mpc_parser_t* p, mpc_result_t* r);
bool mpc_parse_filename(const char* filename, mpc_parser_t* p, mpc_result_t* r);

/*
** Function Types
*/

typedef void(*mpc_dtor_t)(mpc_val_t*);
typedef mpc_val_t*(*mpc_apply_t)(mpc_val_t*);
typedef mpc_val_t*(*mpc_apply_to_t)(mpc_val_t*,void*);
typedef mpc_val_t*(*mpc_fold_t)(mpc_val_t*,mpc_val_t*);
typedef mpc_val_t*(*mpc_afold_t)(int,mpc_val_t**);
typedef mpc_val_t*(*mpc_lift_t)(void);

/*
** Building a Parser
*/

void mpc_delete(mpc_parser_t* p);
mpc_parser_t* mpc_new(const char* name);

mpc_parser_t* mpc_define(mpc_parser_t* p, mpc_parser_t* a);
mpc_parser_t* mpc_undefine(mpc_parser_t* p);

void mpc_cleanup(int n, ...);
void mpc_cleanup_va(int n, va_list va);

/*
** Basic Parsers
*/

mpc_parser_t* mpc_pass(void);
mpc_parser_t* mpc_fail(void);
mpc_parser_t* mpc_lift(mpc_lift_t f);
mpc_parser_t* mpc_lift_val(mpc_val_t* x);

mpc_parser_t* mpc_any(void);
mpc_parser_t* mpc_char(char c);
mpc_parser_t* mpc_range(char s, char e);
mpc_parser_t* mpc_oneof(const char* s);
mpc_parser_t* mpc_noneof(const char* s);
mpc_parser_t* mpc_satisfy(bool(*f)(char));
mpc_parser_t* mpc_string(const char* s);

/*
** Core Parsers
*/

mpc_parser_t* mpc_expect(mpc_parser_t* a, const char* expected);
mpc_parser_t* mpc_apply(mpc_parser_t* a, mpc_apply_t f);
mpc_parser_t* mpc_apply_to(mpc_parser_t* a, mpc_apply_to_t f, void* x);
mpc_parser_t* mpc_not(mpc_parser_t* a, mpc_dtor_t da);
mpc_parser_t* mpc_not_else(mpc_parser_t* a, mpc_dtor_t da, mpc_lift_t lf);
mpc_parser_t* mpc_maybe(mpc_parser_t* a);
mpc_parser_t* mpc_maybe_else(mpc_parser_t* a, mpc_lift_t lf);
mpc_parser_t* mpc_many(mpc_parser_t* a, mpc_fold_t f);
mpc_parser_t* mpc_many_else(mpc_parser_t* a, mpc_fold_t f, mpc_lift_t lf);
mpc_parser_t* mpc_many1(mpc_parser_t* a, mpc_fold_t f);
mpc_parser_t* mpc_count(mpc_parser_t* a, mpc_dtor_t da, mpc_fold_t f, int n);
mpc_parser_t* mpc_count_else(mpc_parser_t* a, mpc_dtor_t da, mpc_fold_t f, int n, mpc_lift_t lf);
mpc_parser_t* mpc_else(mpc_parser_t* a, mpc_parser_t* b);
mpc_parser_t* mpc_also(mpc_parser_t* a, mpc_parser_t* b, mpc_dtor_t da, mpc_fold_t f);
mpc_parser_t* mpc_bind(mpc_parser_t* a, mpc_parser_t* b, mpc_dtor_t da, mpc_fold_t f);
mpc_parser_t* mpc_or(int n, ...);
mpc_parser_t* mpc_and(int n, mpc_afold_t f, ...);
mpc_parser_t* mpc_or_va(int n, va_list va);
mpc_parser_t* mpc_and_va(int n, mpc_afold_t f, va_list va);

/*
** Common Parsers
*/

mpc_parser_t* mpc_eoi(void);
mpc_parser_t* mpc_soi(void);

mpc_parser_t* mpc_space(void);
mpc_parser_t* mpc_spaces(void);
mpc_parser_t* mpc_whitespace(void);

mpc_parser_t* mpc_newline(void);
mpc_parser_t* mpc_tab(void);
mpc_parser_t* mpc_escape(void);

mpc_parser_t* mpc_digit(void);
mpc_parser_t* mpc_hexdigit(void);
mpc_parser_t* mpc_octdigit(void);
mpc_parser_t* mpc_digits(void);
mpc_parser_t* mpc_hexdigits(void);
mpc_parser_t* mpc_octdigits(void);

mpc_parser_t* mpc_lower(void);
mpc_parser_t* mpc_upper(void);
mpc_parser_t* mpc_alpha(void);
mpc_parser_t* mpc_underscore(void);
mpc_parser_t* mpc_alphanum(void);

mpc_parser_t* mpc_int(void);
mpc_parser_t* mpc_hex(void);
mpc_parser_t* mpc_oct(void);
mpc_parser_t* mpc_number(void);

mpc_parser_t* mpc_real(void);
mpc_parser_t* mpc_float(void);

mpc_parser_t* mpc_semi(void);
mpc_parser_t* mpc_comma(void);
mpc_parser_t* mpc_colon(void);
mpc_parser_t* mpc_dot(void);

mpc_parser_t* mpc_char_lit(void);
mpc_parser_t* mpc_string_lit(void);
mpc_parser_t* mpc_regex_lit(void);

mpc_parser_t* mpc_ident(void);

/*
** Useful Parsers
*/

mpc_parser_t* mpc_start(mpc_parser_t* a);
mpc_parser_t* mpc_end(mpc_parser_t* a, mpc_dtor_t da);
mpc_parser_t* mpc_enclose(mpc_parser_t* a, mpc_dtor_t da);

mpc_parser_t* mpc_skip_many(mpc_parser_t* a, mpc_fold_t f);
mpc_parser_t* mpc_skip_many1(mpc_parser_t* a, mpc_fold_t f);

mpc_parser_t* mpc_strip(mpc_parser_t* a);
mpc_parser_t* mpc_tok(mpc_parser_t* a); 
mpc_parser_t* mpc_sym(const char* s);
mpc_parser_t* mpc_total(mpc_parser_t* a, mpc_dtor_t da);

mpc_parser_t* mpc_between(mpc_parser_t* a, mpc_dtor_t ad, const char* o, const char* c);
mpc_parser_t* mpc_parens(mpc_parser_t* a, mpc_dtor_t ad);
mpc_parser_t* mpc_braces(mpc_parser_t* a, mpc_dtor_t ad);
mpc_parser_t* mpc_brackets(mpc_parser_t* a, mpc_dtor_t ad);
mpc_parser_t* mpc_squares(mpc_parser_t* a, mpc_dtor_t ad);

mpc_parser_t* mpc_tok_between(mpc_parser_t* a, mpc_dtor_t ad, const char* o, const char* c);
mpc_parser_t* mpc_tok_parens(mpc_parser_t* a, mpc_dtor_t ad);
mpc_parser_t* mpc_tok_braces(mpc_parser_t* a, mpc_dtor_t ad);
mpc_parser_t* mpc_tok_brackets(mpc_parser_t* a, mpc_dtor_t ad);
mpc_parser_t* mpc_tok_squares(mpc_parser_t* a, mpc_dtor_t ad);

/*
** Regular Expression Parsers
*/

mpc_parser_t* mpc_re(const char* re);

/*
** Common Fold Functions
*/

void mpcf_dtor_null(mpc_val_t* x);
mpc_val_t* mpcf_lift_null(void);
mpc_val_t* mpcf_lift_emptystr(void);

mpc_val_t* mpcf_free(mpc_val_t* x);
mpc_val_t* mpcf_int(mpc_val_t* x);
mpc_val_t* mpcf_hex(mpc_val_t* x);
mpc_val_t* mpcf_oct(mpc_val_t* x);
mpc_val_t* mpcf_float(mpc_val_t* x);
mpc_val_t* mpcf_escape(mpc_val_t* x);
mpc_val_t* mpcf_unescape(mpc_val_t* x);

mpc_val_t* mpcf_fst(mpc_val_t* x, mpc_val_t* y);
mpc_val_t* mpcf_snd(mpc_val_t* x, mpc_val_t* y);

mpc_val_t* mpcf_fst_free(mpc_val_t* x, mpc_val_t* y);
mpc_val_t* mpcf_snd_free(mpc_val_t* x, mpc_val_t* y);

mpc_val_t* mpcf_freefold(mpc_val_t* t, mpc_val_t* x);
mpc_val_t* mpcf_strfold(mpc_val_t* t, mpc_val_t* x);

mpc_val_t* mpcf_afst(int n, mpc_val_t** xs);
mpc_val_t* mpcf_asnd(int n, mpc_val_t** xs);
mpc_val_t* mpcf_atrd(int n, mpc_val_t** xs);

mpc_val_t* mpcf_astrfold(int n, mpc_val_t** xs);
mpc_val_t* mpcf_between_free(int n, mpc_val_t** xs);
mpc_val_t* mpcf_maths(int n, mpc_val_t** xs);


/*
** Printing
*/

void mpc_print(mpc_parser_t* p);

  
/*
** AST
*/

typedef struct mpc_ast_t {
  char* tag;
  char* contents;
  int children_num;
  struct mpc_ast_t** children;
} mpc_ast_t;

void mpc_ast_delete(mpc_ast_t* a);
mpc_ast_t* mpc_ast_new(const char* tag, const char* contents);
mpc_ast_t* mpc_ast_build(int n, const char* tag, ...);
mpc_ast_t* mpc_ast_insert_root(mpc_ast_t* a);

void mpc_ast_add_child(mpc_ast_t* r, mpc_ast_t* a);
void mpc_ast_tag(mpc_ast_t* a, const char* t);
void mpc_ast_print(mpc_ast_t* a);
bool mpc_ast_eq(mpc_ast_t* a, mpc_ast_t* b);

mpc_val_t* mpcf_fold_ast(mpc_val_t* a, mpc_val_t* b);
mpc_val_t* mpcf_afold_ast(int n, mpc_val_t** as);
mpc_val_t* mpcf_apply_str_ast(mpc_val_t* c);

mpc_parser_t* mpca_tag(mpc_parser_t* a, const char* t);
mpc_parser_t* mpca_total(mpc_parser_t* a);
mpc_parser_t* mpca_not(mpc_parser_t* a);
mpc_parser_t* mpca_maybe(mpc_parser_t* a);
mpc_parser_t* mpca_many(mpc_parser_t* a);
mpc_parser_t* mpca_many1(mpc_parser_t* a);
mpc_parser_t* mpca_count(mpc_parser_t* a, int n);
mpc_parser_t* mpca_else(mpc_parser_t* a, mpc_parser_t* b);
mpc_parser_t* mpca_also(mpc_parser_t* a, mpc_parser_t* b);
mpc_parser_t* mpca_bind(mpc_parser_t* a, mpc_parser_t* b);
mpc_parser_t* mpca_or(int n, ...);
mpc_parser_t* mpca_and(int n, ...);
mpc_parser_t* mpca_grammar(const char* grammar, ...);


/*
** Testing
*/

bool mpc_unmatch(mpc_parser_t* p, const char* s, void* d,
  bool(*tester)(void*, void*),
  mpc_dtor_t destructor,
  void(*printer)(void*));

bool mpc_match(mpc_parser_t* p, const char* s, void* d,
  bool(*tester)(void*, void*), 
  mpc_dtor_t destructor, 
  void(*printer)(void*));

#endif