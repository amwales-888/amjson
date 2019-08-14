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

#include <stdio.h>
#include <string.h>

#include "amjson.h"
#include "extras/amjson_dump.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static void dump_spaces(int count, size_t *written, char *buf, size_t len);
static void dump(struct jhandle *jhandle, struct jobject *jobject,
		 int type, int depth, int pretty, size_t *written,
		 char *buf, size_t len);
static size_t cpyout(char *dst, size_t dlen, char *src, size_t slen, 
		     size_t offset);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define MAX_SPACECOUNT 8

static char *spaces[] = {
  "","  ","    ","      ","        ","          ",
  "            ","              "
};

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void dump_spaces(int depth, size_t *written, char *buf, size_t len) {

  if (depth == 0) return;
  
  if (depth < MAX_SPACECOUNT) {
    *written += cpyout(buf, len, spaces[depth], depth*2, *written);
  } else {    

    for (;;) {
      *written += cpyout(buf, len, spaces[MAX_SPACECOUNT-1], (MAX_SPACECOUNT-1)*2, *written);
      
      depth -= MAX_SPACECOUNT-1;
      if (depth < MAX_SPACECOUNT) break;     
    }

    dump_spaces(depth, written, buf, len);
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static size_t cpyout(char *dst, size_t dlen, char *src, size_t slen, 
		     size_t offset) {

  size_t towrite   = 0;
  size_t available = 0;

  if (dst == (char *)0) return printf("%.*s", (int)slen, src);

  if (offset < dlen) {
    available = dlen - offset;

    towrite = slen;
    if (towrite > available) {
      towrite = available;
    }

    if (towrite > 0) memcpy(&dst[offset], src, towrite);
  }

  return slen;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void dump(struct jhandle *jhandle, struct jobject *jobject,
		 int type, int depth, int pretty, size_t *written,
		 char *buf, size_t len) {

  char *sep = "";
    
  while (jobject) {

    *written += cpyout(buf, len, sep, strlen(sep), *written);

    if ((pretty) && (*sep != ':')) {
      dump_spaces(depth, written, buf, len);
    }
    
    switch (JOBJECT_TYPE(jobject)) {

    case AMJSON_STRING:
      *written += cpyout(buf, len, 
			 JOBJECT_STRING_PTR(jhandle, jobject), JOBJECT_STRING_LEN(jobject), 
			 *written);
      break;
    case AMJSON_NUMBER:
      *written += cpyout(buf, len, 
			 JOBJECT_STRING_PTR(jhandle, jobject), JOBJECT_STRING_LEN(jobject), 
			 *written);
      break;
    case AMJSON_OBJECT:
      *written += cpyout(buf, len, "{", 1, *written);
      if (pretty) *written += cpyout(buf, len, "\n", 1, *written);
      dump(jhandle, OBJECT_FIRST_KEY(jhandle, jobject), AMJSON_OBJECT, depth+1, pretty, written, buf, len);
      if (pretty) *written += cpyout(buf, len, "\n", 1, *written);
      if (pretty) dump_spaces(depth, written, buf, len);
      *written += cpyout(buf, len, "}", 1, *written);
      break;
    case AMJSON_ARRAY:
      *written += cpyout(buf, len, "[", 1, *written);
      if (pretty) *written += cpyout(buf, len, "\n", 1, *written);
      dump(jhandle, ARRAY_FIRST(jhandle, jobject), AMJSON_ARRAY, depth+1, pretty, written, buf, len);
      if (pretty) *written += cpyout(buf, len, "\n", 1, *written);
      if (pretty) dump_spaces(depth, written, buf, len);
      *written += cpyout(buf, len, "]", 1, *written);
      break;
    case AMJSON_TRUE:
      *written += cpyout(buf, len, "true", 4, *written);
      break;
    case AMJSON_FALSE:
      *written += cpyout(buf, len, "false", 5, *written);
      break;
    case AMJSON_NULL:
      *written += cpyout(buf, len, "null", 4, *written);
      break;
    }

    if (depth == 0) {    
      jobject = (void *)0;
    } else {
      jobject = JOBJECT_NEXT(jhandle,jobject);
    }

    if ((type == AMJSON_OBJECT) &&
	((*sep == '\0') || (*sep == ','))) {      
      if (pretty) {
	sep = ": ";
      } else {
	sep = ":";
      }
    } else {
      if (pretty) {
	sep = ",\n";
      } else {
	sep = ",";
      }
    }
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
size_t amjson_dump(struct jhandle *jhandle, struct jobject *jobject, 
		   int pretty, char *buf, size_t len) {

  size_t written = 0;

  if (!jobject) jobject = JOBJECT_ROOT(jhandle);
  
  dump(jhandle, jobject, JOBJECT_TYPE(jobject), 0, pretty, &written, buf, len);

  written += cpyout(buf, len, "\n", 1, written);
  return written;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
