#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/bool.h"
#include "cixl/func.h"
#include "cixl/scope.h"

static void fprint(struct cx_type *type, struct cx_box *box, FILE *out) {
  fputs(box->as_bool ? "true" : "false", out);
}

static void true_imp(struct cx_scope *scope) {
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = true;
}

static void false_imp(struct cx_scope *scope) {
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = false;
}

void cx_add_bool_type(struct cx *cx) {
  cx->bool_type = cx_add_type(cx, "Bool", cx->any_type, NULL);
  cx->bool_type->fprint = fprint;

  cx_add_func(cx, "true")->ptr = true_imp;
  cx_add_func(cx, "false")->ptr = false_imp;
}
