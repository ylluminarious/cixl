#include <stdarg.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/util.h"

struct cx_error *cx_error_init(struct cx_error *e, int row, int col, char *msg) {
  e->row = row;
  e->col = col;
  e->msg = msg;
  return e;
}

struct cx_error *cx_error_deinit(struct cx_error *e) {
  free(e->msg);
  return e;
}

struct cx_error *cx_error(struct cx *cx, int row, int col, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  char *msg = cx_vfmt(spec, args);
  va_end(args);

  return cx_error_init(cx_vec_push(&cx->errors), row, col, msg); 
}
