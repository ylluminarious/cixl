#include "cixl/repl.h"
#include "cixl/set.h"
#include "cixl/tests.h"
#include "cixl/vec.h"

int main() {
  cx_vec_tests();
  cx_set_tests();
  cx_tests();

  cx_repl(stdin, stdout);
  return 0;
}
