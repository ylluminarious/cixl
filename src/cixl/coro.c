#include "cixl/box.h"
#include "cixl/coro.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_coro *cx_coro_init(struct cx_coro *coro, struct cx_scope *scope) {
  coro->scope = scope;
  scope->coro = coro;
  cx_vec_init(&coro->toks, sizeof(struct cx_tok));
  coro->nrefs = 1;
  coro->done = false;
  cx_coro_yield(coro);
  return coro;
}

struct cx_coro *cx_coro_deinit(struct cx_coro *coro) {
  free(cx_scope_deinit(coro->scope));
  cx_do_vec(&coro->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&coro->toks);
  return coro;
}

void cx_coro_yield(struct cx_coro *coro) {
  cx_do_vec(&coro->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_clear(&coro->toks);
  struct cx *cx = coro->scope->cx;
		    
  for (size_t i = cx->pc+1; i < cx->toks->count; i++) {
    cx_tok_copy(cx_vec_push(&coro->toks), cx_vec_get(cx->toks, i));
  }
}

static void call(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_coro *coro = value->as_coro;
  
  if (coro->done) {
    cx_error(cx, cx->row, cx->col, "Coro is done");
  } else {
    cx_push_scope(cx, coro->scope);
    cx_eval(cx, &coro->toks, 0);
    cx_pop_scope(cx, coro->scope);
    if (cx->pc == coro->toks.count) { coro->done = true; }
  }
}

static void fprint(struct cx_box *value, FILE *out) { 
  fprintf(out, "Coro(%p:%d)", value->as_coro, value->as_coro->nrefs);
}

static void copy(struct cx_box *dst, struct cx_box *src) {
  struct cx_coro *c = src->as_coro;
  dst->as_coro = c;
  c->nrefs++;
}

static void deinit(struct cx_box *value) {
  value->as_coro->nrefs--;
  if (!value->as_coro->nrefs) { free(cx_coro_deinit(value->as_coro)); }
}

struct cx_type *cx_init_coro_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Coro", cx->any_type, NULL);
  t->fprint = fprint;
  t->call = call;
  t->copy = copy;
  t->deinit = deinit;
  return t;
}
