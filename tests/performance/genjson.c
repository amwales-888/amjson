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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static unsigned int prnd(void);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static unsigned int prnd(void) {

  static unsigned int nSeed = 5323;
  nSeed = (8253729 * nSeed + 2396403);
  return nSeed  % 32767;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc, char **argv) {

  FILE          *fp;
  char          *filepath;
  char          *endptr;
  unsigned long sizebytes;
  unsigned long total = 0;
  int           written;
  char          *sep;
  unsigned int  key;
  unsigned int  value;
  
  if (argc != 3) {
    printf("Usage: %s filepath sizebytes\n", argv[0]);
    return 1;
  }

  filepath = argv[1];
  sizebytes = strtoul(argv[2], &endptr, 10);

  if (*endptr != '\0') {
    if ((*endptr == 'K') && (endptr[1] == '\0')) {
      sizebytes *= 1024;
    } else if ((*endptr == 'M') && (endptr[1] == '\0')) {
      sizebytes *= 1024 * 1024;
    } else     if ((*endptr == 'G') && (endptr[1] == '\0')) {
      sizebytes *= 1024 * 1024 * 1024;
    } else {
      fprintf(stderr, "Error: Parsing sizebytes\n");
      return 1;
    }
  }

  printf("Creating file '%s'...\n", filepath);

  fp = fopen(filepath, "w");
  if (!fp) {
    fprintf(stderr, "Error: Creating file '%s' - %s\n", filepath, strerror(errno));
    return 1;
  }

  written = fprintf(fp, "{");
  sep = "";
  
  for (;;) {

    key   = prnd();
    value = prnd();
    
    written = fprintf(fp, "%s\"%d\":%d", sep, key, value);
    if (written == -1) {
      goto error;
    }
        
    sep = ",";

    total += written;
    if (total > sizebytes) {
      break;
    }
  }

  written = fprintf(fp, "}");

  fclose(fp);
  fprintf(stderr, "Done.\n");
  return 0;
 error:
  fclose(fp);
  fprintf(stderr, "Failed creating file\n");
  return 1;
}
