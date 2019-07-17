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

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static void munmap_file(struct jhandle_s *jhandle);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void munmap_file(struct jhandle_s *jhandle) {

  munmap(jhandle->buf, jhandle->len);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int json_file_decode(struct jhandle_s *jhandle, char *pathname) {

  int         fd;
  struct stat sb;
  int         result;
  char        *ptr;

  fd = open(pathname, O_RDONLY);
  if (fd == -1) goto error1;

  if (fstat(fd, &sb) == -1) goto error2;
        
  ptr = (char *)mmap((void *)0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    goto error2;
  }

  jhandle->onfree = munmap_file;
  
  result = json_decode(jhandle, ptr, sb.st_size);

  close(fd);
  return result;
error2:
  close(fd);
error1:
  return -1;
}

