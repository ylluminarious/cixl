#ifndef CX_VAR_H
#define CX_VAR_H

#include "cixl/box.h"

struct cx_var {
  char *id;
  struct cx_box value;
};

struct cx_var *cx_var_init(struct cx_var *var, const char *id);
struct cx_var *cx_var_deinit(struct cx_var *var);

#endif
