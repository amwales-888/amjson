/* -------------------------------------------------------------------- *

Copyright 2019 Angelo Masci

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to permit 
persons to whom the Software is furnished to do so, subject to the 
following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * -------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "amjson.h"
#include "extras/amjson_file.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static int file_map(struct mhandle *mhandle, char *pathname, int flags);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void amjson_file_unmap(struct mhandle *mhandle) {

  munmap(mhandle->buf, mhandle->len);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int file_map(struct mhandle *mhandle, char *pathname, int flags) {

  int         fd;
  struct stat sb;
  
  fd = open(pathname, O_RDONLY);
  if (fd == -1) return -1;

  if (fstat(fd, &sb) == -1) goto error;

  /* buffer size is limited to the maximum offset we can address
   * into a buffer. */
  if (sb.st_size > BOFF_MAX) goto error;

  mhandle->len = sb.st_size;  
  mhandle->buf = (char *)mmap((void *)0, mhandle->len,
			      PROT_READ, MAP_SHARED|flags, fd, 0);
  if (mhandle->buf == MAP_FAILED) goto error;

  return 0;
error:
  close(fd);
  return -1;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int amjson_file_map(struct mhandle *mhandle, char *pathname, int flags) {

  if (file_map(mhandle, pathname, flags) != 0) {
    errno = EIO;
    return -1;
  }

  return 0;
}
