/* -------------------------------------------------------------------- *

Copyright 2012 Angelo Masci

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

#include "json.h"
#include "json_file.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static void file_munmap(struct jhandle *jhandle);
static int file_decode(struct jhandle *jhandle, char *pathname);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void file_munmap(struct jhandle *jhandle) {

  munmap(jhandle->buf, jhandle->len);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static int file_decode(struct jhandle *jhandle, char *pathname) {

  int         fd;
  struct stat sb;
  int         result = -1;
  char        *ptr;
  int         lerrno;
  
  fd = open(pathname, O_RDONLY);
  if (fd == -1) return -1;

  if (fstat(fd, &sb) == -1) goto error;

#ifndef BIGJSON
  /* We only support files >4GB if BIGJSON Is defined */
  if (sb.st_size >= 0xFFFFFFFF) goto error;
#endif
  
  ptr = (char *)mmap((void *)0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) goto error;

  jhandle->onfree = file_munmap;
  
  result = json_decode(jhandle, ptr, sb.st_size);

error:
  lerrno = errno;
  close(fd);
  errno = lerrno;

  return result;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int json_file_decode(struct jhandle *jhandle, char *pathname) {

  if (file_decode(jhandle, pathname) != 0) {
    if ((errno != EINVAL) &&
	(errno != ENOMEM)) {

      errno = EIO;
    }

    return -1;
  }

  return 0;
}
