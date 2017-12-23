#ifndef CX_PARSE_H
#define CX_PARSE_H

#include <stdbool.h>
#include <stdio.h>

struct cx;
struct cx_vec;

enum cx_tok_type { CX_TEND, CX_TFUNC, CX_TGROUP, CX_TID, CX_TLITERAL, CX_TMACRO,
		   CX_TUNGROUP };

struct cx_tok {
  enum cx_tok_type type;
  void *data;
  int row, col;
};

struct cx_tok *cx_tok_init(struct cx_tok *tok,
			   enum cx_tok_type type,
			   void *data,
			   int row, int col);

struct cx_tok *cx_tok_deinit(struct cx_tok *tok);

bool cx_parse_tok(struct cx *cx, FILE *in, struct cx_vec *out);
bool cx_parse_end(struct cx *cx, FILE *in, struct cx_vec *out);
bool cx_parse(struct cx *cx, FILE *in, struct cx_vec *out);

#endif
