#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/eval.h"
#include "cixl/lambda.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_lambda *cx_lambda_init(struct cx_lambda *lambda) {
  cx_vec_init(&lambda->body, sizeof(struct cx_tok));
  lambda->nrefs = 0;
  return lambda;
}

struct cx_lambda *cx_lambda_deinit(struct cx_lambda *lambda) {
  cx_do_vec(&lambda->body, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&lambda->body);
  return lambda;
}

static void call(struct cx_box *value, struct cx_scope *scope) {
  cx_eval(scope->cx, &value->as_lambda->body, 0);
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
  value->as_lambda->nrefs--;
  if (!value->as_lambda->nrefs) { free(cx_lambda_deinit(value->as_lambda)); }
}

struct cx_type *cx_init_lambda_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Lambda", cx->any_type, NULL);
  t->fprint = fprint;
  t->call = call;
  t->copy = copy;
  t->deinit = deinit;
  return t;
}
