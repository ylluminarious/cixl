#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/int.h"
#include "cixl/parse.h"
#include "cixl/vec.h"

struct cx_tok *cx_tok_init(struct cx_tok *tok,
			   enum cx_tok_type type,
			   void *data,
			   int row, int col) {
  tok->type = type;
  tok->data = data;
  tok->row = row;
  tok->col = col;
  return tok;
}

struct cx_tok *cx_tok_deinit(struct cx_tok *tok) {
  switch (tok->type) {
  case CX_TID:
  case CX_TLITERAL:
    if (tok->data) { free(tok->data); }
    break;
  case CX_TMACRO:
    free(cx_macro_eval_deinit(tok->data));
    break;
  default:
    break;
  }
   
  return tok;
}

static bool cx_parse_id(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_buf id;
  cx_buf_init(&id);
  bool ok = true;
  int col = cx->col;
  
  while (true) {
    char c = fgetc(in);

    if (cx_is_separator(cx, c)) {
      ok = ungetc(c, in) != EOF;
      goto exit;
    }

    fputc(c, id.stream);
    col++;
  }

 exit: {
    if (ok) {
      cx_buf_close(&id);
      struct cx_macro *m = cx_get_macro(cx, id.data, true);
      
      if (m) {
	cx->col = col;
	ok = m->imp(cx, in, out);
	free(id.data);
      } else {
	cx_tok_init(cx_vec_push(out), CX_TID, id.data, cx->row, cx->col);
	cx->col = col;
      }
    } else {
      cx_error(cx, cx->row, cx->col, "Failed parsing id");
      ok = false;
    }
    
    cx_buf_deinit(&id);
    return ok;
  }
}

static bool cx_parse_int(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_buf value;
  cx_buf_init(&value);
  int col = cx->col;
  bool ok = true;
  
  while (true) {
    char c = fgetc(in);
    
    if (!isdigit(c)) {
      ok = ungetc(c, in) != EOF;
      goto exit;
    }
    
    fputc(c, value.stream);
    col++;
  }

  exit:
    if (ok) {
      cx_buf_close(&value);
      cx_int_t int_value = strtoimax(value.data, NULL, 10);
      free(value.data);
      
      if (int_value || !errno) {
	struct cx_box *box = cx_box_init(malloc(sizeof(struct cx_box)),
					 cx->int_type);
	box->as_int = int_value;
	
	cx_tok_init(cx_vec_push(out),
		    CX_TLITERAL,
		    box,
		    cx->row, cx->col);

	cx->col = col;
      }
    } else {
      cx_error(cx, cx->row, cx->col, "Failed parsing int");
      ok = false;
    }
    
    cx_buf_deinit(&value);
    return ok;
}

bool cx_parse_tok(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  bool done = false;
  
  void push_tok(enum cx_tok_type type, void *data) {
    cx_tok_init(cx_vec_push(out), type, data, row, col);
  }
  
  while (!done) {
      char c = fgetc(in);
  
      switch (c) {
      case EOF:
	done = true;
      case ' ':
	cx->col++;
	break;
      case '\n':
	cx->row++;
	break;
      case ';':
	cx_tok_init(cx_vec_push(out), CX_TEND, NULL, row, col);
	return true;
      default:
	if (isdigit(c)) {
	  ungetc(c, in);
	  return cx_parse_int(cx, in, out);
	}
	
	if (isgraph(c)) {
	  ungetc(c, in);
	  return cx_parse_id(cx, in, out);
	}

	cx_error(cx, row, col, "Unexpected char: '%c' (#%d)", c, c); 
	return false;
      }
  }

  return false;
}

bool cx_parse_end(struct cx *cx, FILE *in, struct cx_vec *out) {
  int depth = 1, row = cx->row, col = cx->col;
  
  while (depth) {
    if (!cx_parse_tok(cx, in, out)) {
      cx_error(cx, row, col, "Missing end");
      return false;
    }
    
    struct cx_tok *tok = cx_vec_peek(out);

    switch(tok->type) {
    case CX_TID: {
      char *id = tok->data;
      if (id[strlen(id)-1] == ':') { depth++; }
    }
    case CX_TEND:
      depth--;
      break;
    default:
      break;
    }
  }

  cx_vec_pop(out);
  return true;
}

bool cx_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  cx->row = cx->col = 1;
  
  while (true) {
    if (!cx_parse_tok(cx, in, out)) { break; }
  }

  return cx->errors.count == 0;
}
