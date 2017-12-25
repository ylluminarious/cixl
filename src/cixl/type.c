#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/type.h"

static bool default_call(struct cx_box *value, struct cx_scope *scope) {
  cx_box_copy(cx_push(scope), value);
  return true;
}

struct cx_type *cx_type_init(struct cx_type *type, const char *id) {
  type->id = strdup(id);
  cx_set_init(&type->parents, sizeof(struct cx_type *), cx_cmp_ptr);
  type->call = default_call;
  type->copy = NULL;
  type->fprint = NULL;
  type->deinit = NULL;
  return type;
}

struct cx_type *cx_type_deinit(struct cx_type *type) {
  free(type->id);
  cx_set_deinit(&type->parents);
  return type;  
}

void cx_type_add_parent(struct cx_type *type, struct cx_type *parent) {
  *(struct cx_type **)cx_ok(cx_set_insert(&type->parents, parent)) = parent;
}

bool cx_isa(struct cx_type *type, struct cx_type *parent) {
  if (type == parent) { return true; }
  
  cx_do_set(&type->parents, struct cx_type *, pt) {
    if (cx_isa(*pt, parent)) { return true; }
  }

  return false;
}

static void fprint(struct cx_box *value, FILE *out) {
  fprintf(out, "Type(%s)", value->as_type->id);
}

struct cx_type *cx_init_meta_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Meta", cx->any_type, NULL);
  t->fprint = fprint;
  return t;
}
