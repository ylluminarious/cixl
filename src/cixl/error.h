#ifndef CX_ERROR_H
#define CX_ERROR_H

#include <stdio.h>
#include <stdlib.h>

#define cx_ok(cond) ({						\
      typeof(cond) _cond = cond;				\
								\
      if (!_cond) {						\
	fprintf(stderr,						\
		"'%s' failed at line %d in %s\n",		\
		#cond, __LINE__, __FILE__);			\
	abort();						\
      }								\
								\
      _cond;							\
    })								\

struct cx;

void cx_error(struct cx *cx, const char *spec, ...);

#endif
