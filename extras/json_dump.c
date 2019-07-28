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

#include <stdio.h>

#include "json.h"
#include "json_dump.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static void dump_spaces(int count);
static void dump(struct jhandle *jhandle, struct jobject *jobject,
		 int type, int depth, int pretty);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define MAX_SPACECOUNT 3

static char *spaces[] = {
  "","  ","    ","      ","        ","          ",
  "            ","              "
};

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void dump_spaces(int depth) {

  if (depth == 0) return;
  
  if (depth < MAX_SPACECOUNT) {
    printf(spaces[depth]);
  } else {    

    for (;;) {

      printf(spaces[MAX_SPACECOUNT-1]);
      
      depth -= MAX_SPACECOUNT-1;
      if (depth < MAX_SPACECOUNT) break;     
    }

    dump_spaces(depth);
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void dump(struct jhandle *jhandle, struct jobject *jobject,
		 int type, int depth, int pretty) {

  char *sep   = "";
  char *nl    = "";
  
  if (pretty) {
    nl = "\n";
  }
  
  while (jobject) {

    printf(sep);
    if ((pretty) && (*sep != ':')) {
      dump_spaces(depth);
    }
    
    switch (JOBJECT_TYPE(jobject)) {

    case JSON_STRING:
      printf("\"%.*s\"", JOBJECT_STRING_LEN(jobject), JOBJECT_STRING_PTR(jhandle, jobject));
      break;
    case JSON_NUMBER:
      printf("%.*s", JOBJECT_STRING_LEN(jobject), JOBJECT_STRING_PTR(jhandle, jobject));
      break;
    case JSON_OBJECT:
      printf("{");
      if (pretty) printf(nl);      
      dump(jhandle, OBJECT_FIRST_KEY(jhandle, jobject), JSON_OBJECT, depth+1, pretty);
      if (pretty) printf(nl);
      if (pretty) dump_spaces(depth);
      printf("}");
      break;
    case JSON_ARRAY:
      printf("[");
      if (pretty) printf(nl);
      dump(jhandle, ARRAY_FIRST(jhandle, jobject), JSON_ARRAY, depth+1, pretty);
      printf(nl);
      if (pretty) dump_spaces(depth);
      printf("]");
      break;
    case JSON_TRUE:
      printf("true");
      break;
    case JSON_FALSE:
      printf("false");
      break;
    case JSON_NULL:
      printf("null");
      break;
    }

    if (depth == 0) {    
      jobject = (void *)0;
    } else {
      jobject = JOBJECT_NEXT(jhandle,jobject);
    }

    if ((type == JSON_OBJECT) &&
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
void json_dump(struct jhandle *jhandle, struct jobject *jobject, int pretty) {

  if (!jobject) jobject = JOBJECT_ROOT(jhandle);
  
  dump(jhandle, jobject, JOBJECT_TYPE(jobject), 0, pretty);

  printf("\n");
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
