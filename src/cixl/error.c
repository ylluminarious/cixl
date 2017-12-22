#include <stdarg.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/util.h"

void cx_error(struct cx *cx, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  *(char **)cx_vec_push(&cx->errors) = cx_vfmt(spec, args);
  va_end(args);
}
