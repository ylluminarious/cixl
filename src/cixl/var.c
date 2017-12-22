#include <stdlib.h>
#include <string.h>

#include "cixl/var.h"

struct cx_var *cx_var_init(struct cx_var *var, const char *id) {
  var->id = strdup(id);
  return var;
}

struct cx_var *cx_var_deinit(struct cx_var *var) {
  free(var->id);
  cx_box_deinit(&var->value);
  return var;
}
