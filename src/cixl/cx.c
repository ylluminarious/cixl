#include <stdarg.h>
#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/int.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/type.h"

static const void *get_type_id(const void *value) {
  struct cx_type *const *type = value;
  return &(*type)->id;
}

static const void *get_macro_id(const void *value) {
  struct cx_macro *const *macro = value;
  return &(*macro)->id;
}

static ssize_t let_eval(struct cx_macro_eval *eval,
			struct cx *cx,
			struct cx_vec *toks,
			ssize_t i) {
  struct cx_scope *s = cx_begin(cx, true);
  cx_eval(cx, &eval->toks, 1);
  struct cx_box *val = cx_pop(s, false);
  
  if (!val) {
    cx_end(cx);
    return -1;
  }

  struct cx_tok *id = cx_vec_get(&eval->toks, 0);
  struct cx_box *var = cx_set(s->parent, id->data);
  *var = *val;
  cx_end(cx);
  
  return i+1;
}

static bool let_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_macro_eval *eval =
    cx_macro_eval_init(malloc(sizeof(struct cx_macro_eval)), let_eval);

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &eval->toks)) {
    cx_error(cx, "Malformed let binding at %d:%d", row, col);
    free(cx_macro_eval_deinit(eval));
    return false;
  }

  struct cx_tok *id = cx_vec_peek(&eval->toks);

  if (id->type != CX_TID) {
    cx_error(cx, "Invalid id at %d:%d", row, col);
    free(cx_macro_eval_deinit(eval));
    return false;
  }
  
  if (!cx_parse_end(cx, in, &eval->toks)) {
    cx_error(cx, "Empty let binding at %d:%d", row, col);
    free(cx_macro_eval_deinit(eval));
    return false;
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO, eval, row, col);
  return true;
}

struct cx *cx_init(struct cx *cx) {
  cx_set_init(&cx->separators, sizeof(char), cx_cmp_char);
  cx_add_separators(cx, " \t\n;(){}[]");

  cx_set_init(&cx->types, sizeof(struct cx_type *), cx_cmp_str);
  cx->types.key = get_type_id;
  
  cx->any_type = cx_add_type(cx, "Any", NULL);
  cx_add_meta_type(cx);
  cx_add_int_type(cx);

  cx_set_init(&cx->macros, sizeof(struct cx_macro *), cx_cmp_str);
  cx->macros.key = get_macro_id;
  cx_add_macro(cx, "let:", let_parse);
  
  cx_vec_init(&cx->scopes, sizeof(struct cx_scope *));
  cx->main = cx_begin(cx, false);
  
  cx->row = cx->col = -1;
  cx_vec_init(&cx->errors, sizeof(char *));
  return cx;
}

struct cx *cx_deinit(struct cx *cx) {
  cx_set_deinit(&cx->separators);
  
  cx_do_vec(&cx->scopes, struct cx_scope *, s) { free(cx_scope_deinit(*s)); }
  cx_vec_deinit(&cx->scopes);

  cx_do_set(&cx->types, struct cx_type *, t) { free(cx_type_deinit(*t)); }
  cx_set_deinit(&cx->types);

  cx_do_set(&cx->macros, struct cx_macro *, m) { free(cx_macro_deinit(*m)); }
  cx_set_deinit(&cx->macros);

  cx_do_vec(&cx->errors, char *, e) { free(*e); }
  cx_vec_deinit(&cx->errors);
  return cx;
}

void cx_add_separators(struct cx *cx, const char *cs) {
  for (const char *c = cs; *c; c++) {
    *(char *)cx_ok(cx_set_insert(&cx->separators, c)) = *c;
  }
}

bool cx_is_separator(struct cx *cx, char c) {
  return cx_set_get(&cx->separators, &c);
}

struct cx_type *cx_add_type(struct cx *cx, const char *id, ...) {
  struct cx_type **t = cx_ok(cx_set_insert(&cx->types, &id));

  if (!t) {
    cx_error(cx, "Duplicate type '%s' at %d:%d", id, cx->row, cx->col);
    return NULL;
  }
  
  *t = cx_type_init(malloc(sizeof(struct cx_type)), id);
    
  va_list parents;
  va_start(parents, id);				
  struct cx_type *pt = NULL;
  while ((pt = va_arg(parents, struct cx_type *))) { cx_type_add_parent(*t, pt); }
  va_end(parents);					
  return *t;
}

struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent) {
  struct cx_type **t = cx_set_get(&cx->types, &id);

  if (!t && !silent) {
    cx_error(cx, "Unknown type '%s' at %d:%d", id, cx->row, cx->col);
  }

  return t ? *t : NULL;
}

struct cx_macro *cx_add_macro(struct cx *cx, const char *id, cx_macro_parse_t imp) {
  struct cx_macro **m = cx_ok(cx_set_insert(&cx->macros, &id));

  if (!m) {
    cx_error(cx, "Duplicate macro '%s' at %d:%d", id, cx->row, cx->col);
    return NULL;
  }

  *m = cx_macro_init(malloc(sizeof(struct cx_macro)), id, imp); 
  return *m;
}

struct cx_macro *cx_get_macro(struct cx *cx, const char *id, bool silent) {
  struct cx_macro **m = cx_set_get(&cx->macros, &id);

  if (!m && !silent) {
    cx_error(cx, "Unknown macro '%s' at %d:%d", id, cx->row, cx->col);
  }
  
  return m ? *m : NULL;
}

struct cx_scope *cx_begin(struct cx *cx, bool child) {
  struct cx_scope *s = cx_scope_init(malloc(sizeof(struct cx_scope)),
				     cx,
				     child ? cx_scope(cx) : NULL);
  *(struct cx_scope **)cx_vec_push(&cx->scopes) = s;
  return s;
}

void cx_end(struct cx *cx) {
  cx_ok(cx->scopes.count > 1);
  struct cx_scope **s = cx_vec_pop(&cx->scopes);
  free(cx_scope_deinit(*s));
}

struct cx_scope *cx_scope(struct cx *cx) {
  cx_ok(cx->scopes.count);
  return *(struct cx_scope **)cx_vec_get(&cx->scopes, cx->scopes.count-1);
}

void cx_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_ok(cx_get_type(&cx, "Int", true));
  cx_deinit(&cx);
}
