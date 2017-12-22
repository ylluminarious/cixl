#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/type.h"

struct cx_type *cx_type_init(struct cx_type *type, const char *id) {
  type->id = strdup(id);
  cx_set_init(&type->parents, sizeof(struct cx_type *), cx_cmp_ptr);
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

static void fprint(struct cx_type *type, struct cx_box *box, FILE *out) {
  fprintf(out, "Type(%s)", box->as_type->id);
}

void cx_add_meta_type(struct cx *cx) {
  cx->meta_type = cx_add_type(cx, "Meta", cx->any_type, NULL);
  cx->meta_type->fprint = fprint;
}
