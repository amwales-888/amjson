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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "json.h"
#include "json_file.h"
#include "json_dump.h"
#include "json_query.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static double tstos(struct timespec* ts) {
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static int local_mkstemp(char *tmpfile) {

  int value = getpid();
  int len   = strlen(tmpfile);
  int fd    = -1;
  
  while (fd == -1) {
   
    sprintf(&tmpfile[len-6], "%.6d", value);
  
    do {

      fd = open(tmpfile, O_RDWR|O_CREAT|O_EXCL, 0644);
    } while ((fd == -1) && (errno == EINTR));
    
    if (fd == -1) {
      if (errno != EEXIST) return -1;
    }
    
    value++;
    if (value > 999999) {
      value = 0;
    }
  }

  return fd;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static int copy_stdin(char *tmpfile) {

  int  fd;
  int  bytes_read;
  
  fd = local_mkstemp(tmpfile);
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
    fprintf(stderr, "       %s filepath --dump-pretty\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "filepath        - Path to file or '-' to read from stdin\n");
    fprintf(stderr, "   query        - Path to JSON object to display\n");
    fprintf(stderr, "  --dump        - Output compact JSON representation of data\n");
    fprintf(stderr, "  --dump-pretty - Output pretty printed JSON representation of data\n");
    return 1;
  }

  filepath = argv[1];
  clock_gettime(CLOCK_MONOTONIC, &start); 
  
  if (json_alloc(&jhandle, (void *)0, 1024 * 1024) == 0) {    

    char tmpfile[] = "/tmp/json.XXXXXX";

    if (strcmp(filepath, "-") == 0) {
      if (copy_stdin(tmpfile) == -1) {
	fprintf(stderr, "Failed to copy stdin\n");
	return 1;					  
      }

      filepath = tmpfile;
    }

    if (json_file_decode(&jhandle, filepath) == 0) {      
      fprintf(stderr, "JSON valid [%d items]\n", jhandle.used);

      if (argc == 3) {  
	if (strcmp(argv[2],"--dump") == 0) {
	  json_dump(&jhandle, (void *)0, 0);
	} else if (strcmp(argv[2],"--dump-pretty") == 0) {
	  json_dump(&jhandle, (void *)0, 1);
	} else {
	  char *query = argv[2];
	  struct jobject *jobject = json_query(&jhandle, JOBJECT_ROOT(&jhandle), query);
	  if (jobject) {
	    json_dump(&jhandle, jobject, 1);
	  } else {
	    fprintf(stderr, "'%s' not found\n", query);
	    return 1;
	  }
	}
      }
    } else {
      if (errno == ENOMEM) {
	fprintf(stderr, "Failed allocating memory\n");
      } else {
	fprintf(stderr, "JSON invalid\n");
      }      
      return 1;
    }

    json_free(&jhandle);

    if (filepath == tmpfile) {
      unlink(tmpfile);
    }
    
  } else {
    fprintf(stderr, "JSON alloc failed\n");
    return 1;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsed = tstos(&end) - tstos(&start);
  fprintf(stderr, "Ellapsed time seconds:%f\n", elapsed);
  
  return 0;
}
