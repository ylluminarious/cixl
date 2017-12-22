#ifndef CX_EVAL_H
#define CX_EVAL_H

#include <stdbool.h>

struct cx;
struct cx_vec;

ssize_t cx_eval_tok(struct cx *cx, struct cx_vec *toks, ssize_t i);
bool cx_eval(struct cx *cx, struct cx_vec *toks, ssize_t i);
bool cx_eval_str(struct cx *cx, const char *in);

#endif
