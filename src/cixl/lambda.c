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

static bool call(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_lambda *l = value->as_lambda;
  cx_push_scope(cx, l->scope); 
  bool ok = cx_eval(cx, &l->body, 0);

  if (cx->scopes.count > 1 && cx_scope(cx, 0) == l->scope) {
    cx_pop_scope(cx, false);
  }
  
  return ok;
}

static void fprint(struct cx_box *value, FILE *out) { 
  fprintf(out, "Lambda(%p:%d)", value->as_lambda, value->as_lambda->nrefs);
}

static void copy(struct cx_box *dst, struct cx_box *src) {
  struct cx_lambda *l = src->as_lambda;
  dst->as_lambda = l;
  l->nrefs++;
}

static void deinit(struct cx_box *value) {
  struct cx_lambda *l = value->as_lambda;
  cx_ok(l->nrefs > 0);
  l->nrefs--;
  if (!l->nrefs) { free(cx_lambda_deinit(l)); }
}

struct cx_type *cx_init_lambda_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Lambda", cx->any_type, NULL);
  t->fprint = fprint;
  t->call = call;
  t->copy = copy;
  t->deinit = deinit;
  return t;
}
