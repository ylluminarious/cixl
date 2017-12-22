#ifndef CX_BOX_H
#define CX_BOX_H

#include <stdio.h>
#include "cixl/int.h"

struct cx_type;

struct cx_box {
  struct cx_type *type;
  
  union {
    cx_int_t as_int;
    struct cx_type *as_type;
  };
};

struct cx_box *cx_box_init(struct cx_box *box, struct cx_type *type);
struct cx_box *cx_box_deinit(struct cx_box *box);

struct cx_box *cx_copy_value(struct cx_box *dst, struct cx_box *src);
void cx_fprint_value(struct cx_box *box, FILE *out);

#endif
