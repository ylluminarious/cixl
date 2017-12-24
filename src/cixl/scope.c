#include "cixl/box.h"
#include "cixl/coro.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/var.h"

struct cx_scope *cx_scope_init(struct cx_scope *scope,
			       struct cx *cx,
			       struct cx_scope *parent) {
  scope->cx = cx;
  scope->parent = parent;
  scope->coro = NULL;
  cx_vec_init(&scope->stack, sizeof(struct cx_box));

  cx_set_init(&scope->env, sizeof(struct cx_var), cx_cmp_str);
  scope->env.key_offset = offsetof(struct cx_var, id);

  cx_vec_init(&scope->toks, sizeof(struct cx_tok));
  
  return scope;
}

struct cx_scope *cx_scope_deinit(struct cx_scope *scope) {
  cx_do_vec(&scope->stack, struct cx_box, b) { cx_box_deinit(b); }
  cx_vec_deinit(&scope->stack);

  cx_do_set(&scope->env, struct cx_var, v) { cx_var_deinit(v); }
  cx_set_deinit(&scope->env);

  cx_do_vec(&scope->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&scope->toks);

  return scope;
}

struct cx_box *cx_push(struct cx_scope *scope) {
  return cx_vec_push(&scope->stack);
}

struct cx_box *cx_pop(struct cx_scope *scope, bool silent) {
  if (!scope->stack.count) {
    if (silent) { return NULL; }
    cx_error(scope->cx, scope->cx->row, scope->cx->col, "Stack is empty");
  }

  return cx_vec_pop(&scope->stack);
}

struct cx_box *cx_peek(struct cx_scope *scope, bool silent) {
  if (!scope->stack.count) {
    if (silent) { return NULL; }
    cx_error(scope->cx, scope->cx->row, scope->cx->col, "Stack is empty");
  }

  return cx_vec_peek(&scope->stack, 0);
}

void cx_fprint_stack(struct cx_scope *scope, FILE *out) {
  fputc('[', out);
  char sep = 0;
  
  cx_do_vec(&scope->stack, struct cx_box, b) {
    if (sep) { fputc(sep, out); }
    cx_box_fprint(b, out);
    sep = ' ';
  }

  fputs("]\n", out);
}

struct cx_box *cx_set(struct cx_scope *scope, const char *id) {
  struct cx_var *var = cx_set_get(&scope->env, &id);

  if (var) {
    cx_box_deinit(&var->value);
  } else {
    var = cx_var_init(cx_set_insert(&scope->env, &id), id);
  }

  return &var->value;
}

struct cx_box *cx_get(struct cx_scope *scope, const char *id, bool silent) {
  struct cx_var *var = cx_set_get(&scope->env, &id);

  if (!var && !silent) {
    cx_error(scope->cx, scope->cx->row, scope->cx->col, "Unknown variable: '%s'", id);
    return scope->parent ? cx_get(scope->parent, id, silent) : NULL;
  }

  return &var->value;
}
