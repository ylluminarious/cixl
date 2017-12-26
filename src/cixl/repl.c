#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/repl.h"
#include "cixl/scope.h"

void cx_repl(FILE *in, FILE *out) {
  struct cx cx;
  cx_init(&cx);

  fprintf(out, "cixl v%s, ", CX_VERSION);

  if (cx_eval_str(&cx,
		  "func: _fib(a b n Int) $n ? if {$b, $a + $b, -- $n recall} $a;"
		  "func: fib(n Int) _fib 0 1 $n;"
		  "1000000000, clock {fib 30 zap} /")) {
    struct cx_box bmips = *cx_pop(cx.main, false);
    fprintf(out, "%zd bmips\n\n", bmips.as_int);
  } else {
    fputs("? bmips\n\n", out);
  }

  fputs("Press Return twice to eval input.\n\n", out);
    
  struct cx_buf body;
  cx_buf_open(&body);
  char line[CX_REPL_LINE_MAX];

  while (true) {
    fflush(body.stream);
    fputs(body.size ? ".." : "> ", out);
    
    if (fgets(line, sizeof(line), in) == NULL) { break; }

    if (strcmp(line, "\n") == 0) {
      cx_buf_close(&body);

      if (cx_eval_str(&cx, body.data)) {
	cx_fprint_stack(cx_scope(&cx, 0), out);
      } else {
	cx_do_vec(&cx.errors, struct cx_error, e) {
	  fprintf(out, "Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
	  cx_error_deinit(e);
	}
	
	cx_vec_clear(&cx.errors);
      }

      free(body.data);
      cx_buf_open(&body);
    } else {
      if (strcmp(line, "quit\n") == 0) { break; }
      fputs(line, body.stream);
    }
  }

  cx_buf_close(&body);
  free(body.data);
  cx_deinit(&cx);
}
