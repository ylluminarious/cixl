#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/eval.h"
#include "cixl/error.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/vec.h"

ssize_t cx_eval_id(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i);
  char *id = t->data;
  
  if (isupper(id[0])) {
    struct cx_type *type = cx_get_type(cx, id, false);
    if (!type) { return -1; }
    struct cx_box *box = cx_push(cx_scope(cx));
    box->type = cx->meta_type;
    box->as_type = type;
  } else {
    if (strcmp(id, "!") == 0) {
      cx_reset(cx_scope(cx));
    } else if (strcmp(id, "_") == 0) {
      cx_pop(cx_scope(cx), false);
    } else if (id[0] == '$' && id[1]) {
      struct cx_scope *s = cx_scope(cx);
      struct cx_box *v = cx_get(s, id+1, false);
      if (!v) { return -1; }
      cx_copy_value(cx_box_init(cx_push(s), v->type), v);
    } else if (strcmp(id, "$") == 0) {
      struct cx_scope *s = cx_scope(cx);
      struct cx_box *vp = cx_peek(s, false);

      if (vp) {
	struct cx_box v = *vp;
	cx_copy_value(cx_box_init(cx_push(s), v.type), &v);
      }
    } else {
      cx_error(cx, "Unknown identifier '%s' at %d:%d", id, t->row, t->col);
      return -1;
    }
  }
  
  return i+1;
}

ssize_t cx_eval_literal(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i);
  cx_copy_value(cx_push(cx_scope(cx)), t->data);
  return i+1;
}

ssize_t cx_eval_macro(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i);
  struct cx_macro_eval *eval = t->data;
  return eval->imp(eval, cx, toks, i);
}

ssize_t cx_eval_tok(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  cx_ok(toks->count);
  struct cx_tok *t = cx_vec_get(toks, i);
  cx->row = t->row;
  cx->col = t->col;
  
  switch (t->type) {
  case CX_TID:
    return cx_eval_id(cx, toks, i);
  case CX_TLITERAL:
    return cx_eval_literal(cx, toks, i);
  case CX_TMACRO:
    return cx_eval_macro(cx, toks, i);
  default:
    cx_error(cx, "Unexpected token at %d:%d", t->row, t->col);
  }

  return -1;
}

bool cx_eval(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  while (i < toks->count) {
    if ((i = cx_eval_tok(cx, toks, i)) == -1) { return false; }
  }
  
  return true;
}

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  FILE *is = fmemopen ((void *)in, strlen(in), "r");
  bool res = false;
  if (!cx_parse(cx, is, &toks)) { goto exit; }
  fclose(is);
  if (!cx_eval(cx, &toks, 0)) { goto exit; }
  res = true;
 exit: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return res;
  }
}
