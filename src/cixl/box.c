#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/type.h"

struct cx_box *cx_box_init(struct cx_box *box, struct cx_type *type) {
  box->type = type;
  return box;
}

struct cx_box *cx_box_deinit(struct cx_box *box) {
  if (box->type->deinit) { box->type->deinit(box->type, box); }
  return box;
}

struct cx_box *cx_copy_value(struct cx_box *dst, struct cx_box *src) {
  if (src->type->copy) {
    src->type->copy(src->type, dst, src);
  }
  else {
    *dst = *src;
  }

  return dst;
}

void cx_fprint_value(struct cx_box *box, FILE *out) {
  cx_ok(box->type->fprint)(box->type, box, out);
}

