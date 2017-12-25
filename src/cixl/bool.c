#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/bool.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"

static void true_imp(struct cx_scope *scope) {
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = true;
}

static void false_imp(struct cx_scope *scope) {
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = false;
}

static void not_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = !x.as_bool;
}

static void fprint(struct cx_box *value, FILE *out) {
  fputs(value->as_bool ? "true" : "false", out);
}

struct cx_type *cx_init_bool_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Bool", cx->any_type, NULL);
  t->fprint = fprint;

  cx_add_func(cx, "true")->ptr = true_imp;
  cx_add_func(cx, "false")->ptr = false_imp;
  cx_add_func(cx, "not")->ptr = not_imp;

  return t;
}
