#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/int.h"

static void fprint(struct cx_type *type, struct cx_box *box, FILE *out) {
  fprintf(out, "%" PRId64, box->as_int);
}

void cx_add_int_type(struct cx *cx) {
  cx->int_type = cx_add_type(cx, "Int", cx->any_type, NULL);
  cx->int_type->fprint = fprint;
}
