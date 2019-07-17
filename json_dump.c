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

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static void dump(struct jhandle_s *jhandle, struct jobject_s *jobject,
		 int type, int count);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void dump(struct jhandle_s *jhandle, struct jobject_s *jobject,
		 int type, int count) {

  char sep = '\0';
  
  while (jobject) {

    printf("%c", sep);
    
    switch (JOBJECT_TYPE(jobject)) {

    case JSON_STRING:
      printf("\"%.*s\"", JOBJECT_STRING_LEN(jobject), JOBJECT_STRING_PTR(jobject));
      break;
    case JSON_NUMBER:
      printf("%.*s", JOBJECT_STRING_LEN(jobject), JOBJECT_STRING_PTR(jobject));
      break;
    case JSON_OBJECT:
      printf("{");
      dump(jhandle, OBJECT_FIRST_KEY(jhandle, jobject), JSON_OBJECT, 1);
      printf("}");
      break;
    case JSON_ARRAY:
      printf("[");
      dump(jhandle, ARRAY_FIRST(jhandle, jobject), JSON_ARRAY, 1);
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

    if (count == 0) {    
      jobject = (void *)0;
    } else {
      jobject = JOBJECT_NEXT(jhandle,jobject);
    }

    if ((type == JSON_OBJECT) &&
	((sep == '\0') || (sep == ','))) {      
      sep = ':';      
    } else {
      sep = ',';
    }
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void json_dump(struct jhandle_s *jhandle, struct jobject_s *jobject) {

  if (!jobject) jobject = JOBJECT_ROOT(jhandle);
  
  dump(jhandle, jobject, JOBJECT_TYPE(jobject), 0);

  printf("\n");
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
