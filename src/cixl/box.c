#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/type.h"

struct cx_box *cx_box_new(struct cx_type *type) {
  return cx_box_init(malloc(sizeof(struct cx_box)), type);
}

struct cx_box *cx_box_init(struct cx_box *box, struct cx_type *type) {
  box->type = type;
  return box;
}

struct cx_box *cx_box_deinit(struct cx_box *box) {
  if (box->type->deinit) { box->type->deinit(box); }
  return box;
}

bool cx_box_call(struct cx_box *box, struct cx_scope *scope) {
  return box->type->call(box, scope);
}

struct cx_box *cx_box_copy(struct cx_box *dst, struct cx_box *src) {
  if (src->type->copy) {
    dst->type = src->type;
    src->type->copy(dst, src);
  }
  else {
    *dst = *src;
  }

  return dst;
}

void cx_box_fprint(struct cx_box *box, FILE *out) {
  cx_ok(box->type->fprint)(box, out);
}

