#include "cixl/box.h"
#include "cixl/coro.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_coro *cx_coro_init(struct cx_coro *coro, struct cx_scope *scope) {
  coro->scope = scope;
  coro->pc = scope->cx->pc+1;
  coro->nrefs = 1;
  coro->done = false;
  return coro;
}

struct cx_coro *cx_coro_deinit(struct cx_coro *coro) {
  cx_scope_unref(coro->scope);
  return coro;
}

static void yield_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (scope->coro) {
    scope->coro->pc = cx->pc+1;    
  } else {
    scope = cx_pop_scope(cx, false);
    if (!scope) { return; }
    scope->coro = cx_coro_init(malloc(sizeof(struct cx_coro)), scope);
    cx_box_init(cx_push(cx_scope(cx, 0)), cx->coro_type)->as_coro = scope->coro;
  }
  
  cx->stop_pc = cx->pc+1;
}

static void call(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_coro *coro = value->as_coro;
  
  if (coro->done) {
    cx_error(cx, cx->row, cx->col, "Coro is done");
  } else {
    cx_push_scope(cx, coro->scope);
    if (!cx_eval(cx, &coro->scope->toks, coro->pc)) { return; }
    coro->pc = cx->pc;
    cx_pop_scope(cx, coro->scope);
    if (coro->pc == coro->scope->toks.count) { coro->done = true; }
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
  struct cx_coro *coro = value->as_coro;
  cx_ok(coro->nrefs > 0);
  coro->nrefs--;
  if (!coro->nrefs) { free(cx_coro_deinit(coro)); }
}

struct cx_type *cx_init_coro_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Coro", cx->any_type, NULL);
  t->fprint = fprint;
  t->call = call;
  t->copy = copy;
  t->deinit = deinit;

  cx_add_func(cx, "yield")->ptr = yield_imp;

  return t;
}
