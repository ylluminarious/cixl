#ifndef CX_LAMBDA_H
#define CX_LAMBDA_H

#include "cixl/vec.h"

struct cx;

struct cx_lambda {
  struct cx_vec body;
  int nrefs;
};

struct cx_lambda *cx_lambda_init(struct cx_lambda *lambda);
struct cx_lambda *cx_lambda_deinit(struct cx_lambda *lambda);

void cx_add_lambda_type(struct cx *cx);

#endif
