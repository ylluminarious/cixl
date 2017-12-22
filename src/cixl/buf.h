#ifndef CX_BUF_H
#define CX_BUF_H

#include <stdio.h>

struct cx_buf {
  char *data;
  size_t size;
  FILE *stream;
};

struct cx_buf *cx_buf_init(struct cx_buf *buf);
struct cx_buf *cx_buf_deinit(struct cx_buf *buf);
void cx_buf_close(struct cx_buf *buf);

#endif
