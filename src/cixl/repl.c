#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/repl.h"
#include "cixl/scope.h"

void cx_repl(FILE *in, FILE *out) {
  fprintf(out, "cixl v%s\n\n", CX_VERSION);
  fputs("Press Return twice to evaluate input.\n\n", out);

  struct cx cx;
  cx_init(&cx);
  
  struct cx_buf body;
  cx_buf_init(&body);
  char line[CX_REPL_LINE_MAX];

  while (true) {
    fflush(body.stream);
    fputs(body.size ? ".." : "> ", out);
    
    if (fgets(line, sizeof(line), in) == NULL) { break; }

    if (strcmp(line, "\n") == 0) {
      cx_buf_close(&body);

      if (cx_eval_str(&cx, body.data)) {
	cx_fprint_stack(cx_scope(&cx), out);
      } else {
	cx_do_vec(&cx.errors, struct cx_error, e) {
	  fprintf(out, "Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
	  cx_error_deinit(e);
	}
	
	cx_vec_clear(&cx.errors);
      }

      free(body.data);
      cx_buf_init(&body);
    } else {
      if (strcmp(line, "quit\n") == 0) { break; }
      fputs(line, body.stream);
    }
  }

  cx_buf_deinit(&body);
  free(body.data);
  cx_deinit(&cx);
}
