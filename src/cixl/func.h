#ifndef CX_FUNC_H
#define CX_FUNC_H

#include <stdarg.h>

#include "cixl/set.h"

struct cx_type;

struct cx_func {
  char *id;
  struct cx_set imps;
  int nargs;
};

struct cx_func *cx_func_init(struct cx_func *func, char *id, int nargs);
struct cx_func *cx_func_deinit(struct cx_func *func);
			  
struct cx_func_imp {
  char *id;
  struct cx_vec args;
};

struct cx_func_imp *cx_func_imp_init(struct cx_func_imp *imp, char *id);
struct cx_func_imp *cx_func_imp_deinit(struct cx_func_imp *imp);
struct cx_func_imp *cx_func_add_imp(struct cx_func *func, va_list args);

struct cx_func_arg {
  struct cx_type *type;
  int narg;
};

struct cx_func_arg cx_arg(struct cx_type *type);
struct cx_func_arg cx_narg(int n);

#endif
