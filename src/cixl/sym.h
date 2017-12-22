#ifndef CX_SYM_H
#define CX_SYM_H

#define _cx_sym(x, y)				\
  x ## y					\
  
#define cx_sym(x, y)				\
  _cx_sym(x, y)					\
  
#define cx_gensym(prefix)			\
  cx_sym(prefix, __COUNTER__)			\

#endif
