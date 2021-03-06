#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/eval.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/lambda.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/util.h"
#include "cixl/vec.h"

ssize_t cx_eval_id(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  struct cx_tok *t = cx_vec_get(toks, pc);
  char *id = t->data;

  if (isupper(id[0])) {
    struct cx_type *type = cx_get_type(cx, id, false);
    if (!type) { return -1; }
    struct cx_box *box = cx_push(cx_scope(cx, 0));
    box->type = cx->meta_type;
    box->as_ptr = type;
  } else {
    if (id[0] == '$') {
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get(s, id+1, false);
      if (!v) { return -1; }
      cx_box_copy(cx_push(s), v);
    } else {
      cx_error(cx, t->row, t->col, "Unknown id: '%s'", id);
      return -1;
    }
  }
  
  return pc+1;
}

ssize_t cx_eval_literal(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  struct cx_tok *t = cx_vec_get(toks, pc);
  cx_box_copy(cx_push(cx_scope(cx, 0)), t->data);
  return pc+1;
}

ssize_t cx_eval_macro(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  struct cx_tok *t = cx_vec_get(toks, pc);
  struct cx_macro_eval *eval = t->data;
  return eval->imp(eval, cx, toks, pc);
}

ssize_t cx_eval_lambda(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  struct cx_tok *t = cx_vec_get(toks, pc);
  struct cx_scope *scope = cx_scope(cx, 0);
  
  struct cx_lambda *lambda = cx_lambda_init(malloc(sizeof(struct cx_lambda)),
					    scope,
					    t->data);

  struct cx_box *v = cx_box_init(cx_push(scope), cx->lambda_type);
  v->as_ptr = lambda;
  return pc+1;
}

ssize_t cx_eval_func(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  struct cx_tok *t = cx_vec_get(toks, pc++);
  struct cx_func *func = t->data;
  int row = cx->row, col = cx->col;
  
  while (pc < toks->count) {
    struct cx_scope *s = cx_scope(cx, 0);
    if (s->stack.count - s->cut_offs >= func->nargs) { break; }
    if ((pc = cx_eval_tok(cx, toks, pc)) == -1) { return -1; }
  }

  struct cx_scope *scope = cx_scope(cx, 0);
  
  if (scope->stack.count < func->nargs) {
    cx_error(cx, row, col, "Not enough args for func: '%s'", func->id);
    return -1;
  }

  scope->cut_offs = 0;
  return cx_funcall(func, scope, row, col) ? pc : -1;
}

ssize_t cx_eval_group(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  struct cx_tok *t = cx_vec_get(toks, pc);
  struct cx_vec *body = t->data;
  cx_begin(cx, true);
  bool ok = cx_eval(cx, body, 0);
  cx_end(cx);
  return ok ? pc+1 : -1;
}

ssize_t cx_eval_tok(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  cx_ok(toks->count);
  struct cx_tok *t = cx_vec_get(toks, pc);

  cx->row = t->row;
  cx->col = t->col;
  
  switch (t->type) {
  case CX_TFUNC:
    return cx_eval_func(cx, toks, pc);
  case CX_TGROUP:
    return cx_eval_group(cx, toks, pc);
  case CX_TID:
    return cx_eval_id(cx, toks, pc);
  case CX_TLAMBDA:
    return cx_eval_lambda(cx, toks, pc);
  case CX_TLITERAL:
    return cx_eval_literal(cx, toks, pc);
  case CX_TMACRO:
    return cx_eval_macro(cx, toks, pc);
  case CX_TCUT: {
    struct cx_scope *s = cx_scope(cx, 0);
    s->cut_offs = s->stack.count;
    return pc+1;
  }
  default:
    cx_error(cx, t->row, t->col, "Unexpected token: %d", t->type);
    break;
  }

  return -1;
}

bool cx_eval(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  cx->pc = pc;
  bool ok = false;
  
  while (cx->pc < toks->count && cx->pc != cx->stop_pc) {
    cx->toks = toks;
    if ((cx->pc = cx_eval_tok(cx, toks, cx->pc)) == -1) { goto exit; }
    if (cx->errors.count) { goto exit; }
  }

  ok = true;
 exit:
  cx->toks = NULL;
  cx->stop_pc = -1;
  return ok;
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
