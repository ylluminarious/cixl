#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/tests.h"

static void run(struct cx *cx, const char *in) {
  if (!cx_eval_str(cx, in)) {
    printf("%s\n", in);
    
    cx_do_vec(&cx->errors, struct cx_error, e) {
      printf("Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
      cx_error_deinit(e);
    }
    
    cx_vec_clear(&cx->errors);
  }
}

static void stack_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "7 14 @ + 28 = test");
  run(&cx, "7 14 @ _ + 21 = test");

  cx_deinit(&cx);
}

static void group_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "(7 14 21) 21 = test");

  cx_deinit(&cx);
}

static void int_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "42 ? test");
  run(&cx, "0 ? not test");
  run(&cx, "21 + 21 = 42 test");
  run(&cx, "1 = 2 not test");

  cx_deinit(&cx);
}

void cx_tests() {
  stack_tests();
  group_tests();
  int_tests();
}
