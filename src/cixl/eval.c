#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/eval.h"
#include "cixl/error.h"
#include "cixl/func.h"
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
    if (id[0] == '$') {
      struct cx_scope *s = cx_scope(cx);
      struct cx_box *v = cx_get(s, id+1, false);
      if (!v) { return -1; }
      cx_box_copy(cx_push(s), v);
    } else {
      cx_error(cx, t->row, t->col, "Unknown id: '%s'", id);
      return -1;
    }
  }
  
  return i+1;
}

ssize_t cx_eval_literal(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i);
  cx_box_copy(cx_push(cx_scope(cx)), t->data);
  return i+1;
}

ssize_t cx_eval_macro(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i);
  struct cx_macro_eval *eval = t->data;
  return eval->imp(eval, cx, toks, i);
}

ssize_t cx_eval_func(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i++);
  struct cx_func *func = t->data;
  int row = cx->row, col = cx->col;
  
  while (i < toks->count && cx_scope(cx)->stack.count < func->nargs) {
    if ((i = cx_eval_tok(cx, toks, i)) == -1) { return -1; }
  }

  struct cx_scope *scope = cx_scope(cx);

  if (scope->stack.count < func->nargs) {
    cx_error(cx, row, col, "Not enough args for func: '%s'", func->id);
    return -1;
  }

  struct cx_func_imp *imp = cx_func_get_imp(func, &scope->stack);

  if (!imp) {
    cx_error(cx, row, col, "Func not applicable: '%s'", func->id);
    return -1;
  }

  if (imp->ptr) {
    imp->ptr(scope);
  } else {
    if (!cx_eval(cx, &imp->toks, 0)) { return -1; }
    cx_end(cx);
  }
  
  return i;
}

ssize_t cx_eval_group(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  struct cx_tok *t = cx_vec_get(toks, i);
  struct cx_vec *body = t->data;
  cx_begin(cx, true);
  if (!cx_eval(cx, body, 0)) { return -1; }
  cx_end(cx);
  return i+1;
}

ssize_t cx_eval_tok(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  cx_ok(toks->count);
  struct cx_tok *t = cx_vec_get(toks, i);
  cx->row = t->row;
  cx->col = t->col;
  
  switch (t->type) {
  case CX_TFUNC:
    return cx_eval_func(cx, toks, i);
  case CX_TGROUP:
    return cx_eval_group(cx, toks, i);
  case CX_TID:
    return cx_eval_id(cx, toks, i);
  case CX_TLITERAL:
    return cx_eval_literal(cx, toks, i);
  case CX_TMACRO:
    return cx_eval_macro(cx, toks, i);
  default:
    break;
  }

  cx_error(cx, t->row, t->col, "Unexpected token: %d", t->type);
  return -1;
}

bool cx_eval(struct cx *cx, struct cx_vec *toks, ssize_t i) {
  while (i < toks->count) {
    if ((i = cx_eval_tok(cx, toks, i)) == -1) { return false; }
    if (cx->errors.count) { return false; }
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

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *ids,
		  struct cx_vec *func_args) {
  struct cx_vec tmp_ids;
  cx_vec_init(&tmp_ids, sizeof(struct cx_tok));

  cx_do_vec(toks, struct cx_tok, t) {
    switch (t->type) {
    case CX_TID: {
      char *id = t->data;
      if (isupper(id[0])) {
	if (!tmp_ids.count) {
	  cx_error(cx, t->row, t->col, "Missing ids for type: %s", id);
	  cx_vec_deinit(&tmp_ids);
	  return false;
	}

	struct cx_type *type = cx_get_type(cx, id, false);

	if (!type) {
	  cx_vec_deinit(&tmp_ids);
	  return false;
	}
	
	cx_do_vec(&tmp_ids, struct cx_tok, id) {
	  *(struct cx_tok *)cx_vec_push(ids) = *id;
	  *(struct cx_func_arg *)cx_vec_push(func_args) = cx_arg(type);      
	}

	cx_vec_clear(&tmp_ids);
      } else {
	*(struct cx_tok *)cx_vec_push(&tmp_ids) = *t;
	t->data = NULL;
      }
      
      break;
    }
      
    case CX_TLITERAL: {
      struct cx_box *v = t->data;

      if (v->type != cx->int_type || v->as_int >= func_args->count) {
	cx_error(cx, t->row, t->col, "Invalid arg: %d", v->as_int);
	cx_vec_deinit(&tmp_ids);
	return false;
      }

      if (!tmp_ids.count) {
	cx_error(cx, t->row, t->col, "Missing ids for type: %d", v->as_int);
	cx_vec_deinit(&tmp_ids);
	return false;
      }
      
      cx_do_vec(&tmp_ids, struct cx_tok, id) {
	*(struct cx_tok *)cx_vec_push(ids) = *id;
	*(struct cx_func_arg *)cx_vec_push(func_args) = cx_narg(v->as_int);      
      }

      break;
    }
      
    default:
	cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
	cx_vec_deinit(&tmp_ids);
	return false;
    }
  }
  
  cx_vec_deinit(&tmp_ids);
  return true;
}
