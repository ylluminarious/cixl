#include <stdlib.h>

#include "cixl/buf.h"
#include "cixl/error.h"

struct cx_buf *cx_buf_open(struct cx_buf *buf) {
  buf->data = NULL;
  buf->size = 0;
  buf->stream  = open_memstream (&buf->data, &buf->size);
  return buf;
}

struct cx_buf *cx_buf_close(struct cx_buf *buf) {
  cx_ok(buf->stream);
  fclose(buf->stream);
  buf->stream = NULL;
  return buf;
}
