#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/scope.h"

static void fprint(struct cx_type *type, struct cx_box *box, FILE *out) {
  fprintf(out, "%" PRId64, box->as_int);
}

static void ok_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int != 0;
}

static void eq_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int == y.as_int;
}

static void add_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int + y.as_int;
}

void cx_add_int_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Int", cx->any_type, NULL);
  t->fprint = fprint;

  cx_add_func(cx, "?", cx_arg(t))->ptr = ok_imp;
  cx_add_func(cx, "=", cx_arg(t), cx_arg(t))->ptr = eq_imp;
  cx_add_func(cx, "+", cx_arg(t), cx_arg(t))->ptr = add_imp;

  cx->int_type = t;
}
