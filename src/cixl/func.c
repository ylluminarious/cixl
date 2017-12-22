#include <stdlib.h>

#include "cixl/buf.h"
#include "cixl/func.h"
#include "cixl/type.h"

static const void *get_imp_id(const void *value) {
  struct cx_func_imp *const *imp = value;
  return &(*imp)->id;
}

struct cx_func *cx_func_init(struct cx_func *func, char *id, int nargs) {
  func->id = id;
  cx_set_init(&func->imps, sizeof(struct cx_func_imp *), cx_cmp_str);
  func->imps.key = get_imp_id;
  func->nargs = nargs;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  cx_set_deinit(&func->imps);
  return func; 
}

struct cx_func_imp *cx_func_add_imp(struct cx_func *func, va_list args) {
  struct cx_vec imp_args;
  cx_vec_init(&imp_args, sizeof(struct cx_func_arg));
  struct cx_buf id;
  cx_buf_open(&id);
  
  for (int i=0; i < func->nargs; i++) {
    struct cx_func_arg a = va_arg(args, struct cx_func_arg);
    *(struct cx_func_arg *)cx_vec_push(&imp_args) = a;
    
    if (i) { fputc(' ', id.stream); }
    
    if (a.type) {
      fputs(a.type->id, id.stream);
    } else {
      fprintf(id.stream, "%d", a.narg);
    }
  }
    
  cx_buf_close(&id);
  struct cx_func_imp *imp = cx_set_get(&func->imps, &id.data);
  
  if (imp) {
    cx_vec_deinit(&imp_args);
    free(id.data);
  } else {
    imp = cx_func_imp_init(malloc(sizeof(struct cx_func_imp)), id.data);
    imp->args = imp_args;
  }

  return imp;
}

struct cx_func_imp *cx_func_imp_init(struct cx_func_imp *imp, char *id) {
  imp->id = id;
  return imp;
}

struct cx_func_imp *cx_func_imp_deinit(struct cx_func_imp *imp) {
  free(imp->id);
  cx_vec_deinit(&imp->args);
  return imp;
}

struct cx_func_arg cx_arg(struct cx_type *type) {
  return (struct cx_func_arg) { type, -1 };
}

struct cx_func_arg cx_narg(int n) {
  return (struct cx_func_arg) { NULL, n };
}

