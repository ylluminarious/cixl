#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/lambda.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_lambda *cx_lambda_init(struct cx_lambda *lambda,
				 struct cx_scope *scope,
				 struct cx_vec *body) {
  lambda->scope = cx_scope_ref(scope);
  lambda->nrefs = 1;
  cx_vec_init(&lambda->body, sizeof(struct cx_tok));
  
  cx_do_vec(body, struct cx_tok, t) {
    cx_tok_copy(cx_vec_push(&lambda->body), t);
  }
  
  return lambda;
}

struct cx_lambda *cx_lambda_deinit(struct cx_lambda *lambda) {
  cx_do_vec(&lambda->body, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&lambda->body);
  cx_scope_unref(lambda->scope);
  return lambda;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_lambda *l = value->as_ptr;
  cx_push_scope(cx, l->scope); 
  bool ok = cx_eval(cx, &l->body, 0);

  if (cx->scopes.count > 1 && cx_scope(cx, 0) == l->scope) {
    cx_pop_scope(cx, false);
  }
  
  return ok;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx_lambda *l = src->as_ptr;
  dst->as_ptr = l;
  l->nrefs++;
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_lambda *l = value->as_ptr;
  fprintf(out, "Lambda(%p:%d)", l, l->nrefs);
}

static void deinit_imp(struct cx_box *value) {
  struct cx_lambda *l = value->as_ptr;
  cx_ok(l->nrefs > 0);
  l->nrefs--;
  if (!l->nrefs) { free(cx_lambda_deinit(l)); }
}

struct cx_type *cx_init_lambda_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Lambda", cx->any_type, NULL);
  t->equid = equid_imp;
  t->call = call_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;
  return t;
}
