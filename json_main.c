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

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static double tstos(struct timespec* ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static int copy_stdin(char *tmpfile) {

  int  fd;
  int  bytes_read;
  
  fd = mkstemp(tmpfile);
  if (fd == -1) return -1;

  do {

    int  to_write;
    int  bytes_written;
    char buf[1024];
    char *ptr;

    do {
      bytes_read = read(0, buf, 1024);
    } while ((bytes_read == -1) && (errno == EINTR));

    if (bytes_read == -1) goto error;
    
    to_write = bytes_read;
    ptr      = buf;

    while (to_write) {
    
      do {
	bytes_written = write(fd, ptr, to_write);
      } while ((bytes_written == -1) && (errno == EINTR));

      if (bytes_written == -1) goto error;
            
      to_write -= bytes_written;
      ptr      += bytes_written;      
    } 

  } while (bytes_read);

  close(fd);
  return 0;
 error:  
  close(fd);
  return -1;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc, char **argv) {

  struct timespec start;
  struct timespec end;
  double elapsed;
  struct jhandle jhandle;
  char *filepath;
  
  if ((argc < 2) || (argc > 3)) {
    fprintf(stderr, "Usage: %s filepath\n", argv[0]);
    fprintf(stderr, "       %s filepath query\n", argv[0]);
    fprintf(stderr, "       %s filepath --dump\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "filepath - Path to file or '-' to read from stdin\n");
    fprintf(stderr, "   query - Path to JSON object to display\n");
    fprintf(stderr, "  --dump - Output JSON representation of data\n");
    return 1;
  }

  filepath = argv[1];
  clock_gettime(CLOCK_MONOTONIC, &start); 

  if (json_alloc(&jhandle, (void *)0, 1024 * 1024) == 0) {    

    char tmpfile[] = "/tmp/json.XXXXXX";

    if (strcmp(filepath, "-") == 0) {
      if (copy_stdin(tmpfile) == -1) {
	printf("Failed to copy stdin\n");
	return 1;					  
      }

      filepath = tmpfile;
    }

    if (json_file_decode(&jhandle, filepath) == 0) {      
      printf("JSON valid [%d items]\n", jhandle.used);

      if (argc == 3) {  
	if (strcmp(argv[2],"--dump") == 0) {
	  json_dump(&jhandle, (void *)0);
	} else {
	  char *query = argv[2];
	  struct jobject *jobject = json_query(&jhandle, JOBJECT_ROOT(&jhandle), query);
	  if (jobject) {
	    json_dump(&jhandle, jobject);
	  } else {
	    printf("'%s' not found\n", query);
	    return 1;
	  }
	}
      }
    } else {
      if (errno == ENOMEM) {
	printf("Failed allocating memory\n");
      } else {
	printf("JSON invalid\n");
      }      
      return 1;
    }

    json_free(&jhandle);

    if (filepath == tmpfile) {
      unlink(tmpfile);
    }
    
  } else {
    printf("JSON alloc failed\n");
    return 1;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsed = tstos(&end) - tstos(&start);
  printf("Ellapsed time seconds:%f\n", elapsed);
  
  return 0;
}
