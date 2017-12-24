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

  run(&cx, "7 14 dup + 28 = test");
  run(&cx, "7 14 dup zap + 21 = test");

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

static void func_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "func: foo() 42; foo = 42 test");
  run(&cx, "func: bar(x Int) $x + 35; bar 7 42 = test");
  run(&cx, "func: baz(x y Int z 0) $x + $y + $z; baz 1 3 5 9 = test");

  cx_deinit(&cx);
}

static void coro_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "(1 2 yield 3) call + 5 =");
  run(&cx, "func: foo() 1 2 yield 3; foo call + 5 =");
  run(&cx, "(let: x 42; yield $x) call 42 =");

  cx_deinit(&cx);
}

void cx_tests() {
  stack_tests();
  group_tests();
  int_tests();
  func_tests();
  coro_tests();
}
