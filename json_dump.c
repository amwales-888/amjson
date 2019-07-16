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
		 int count);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void dump(struct jhandle_s *jhandle, struct jobject_s *jobject,
		 int count) {

  while (jobject) {

    switch (JOBJECT_TYPE(jobject)) {

    case JSON_STRING:
      printf("STRING:%.*s\n", JOBJECT_STRING_LEN(jobject), JOBJECT_STRING_PTR(jobject));
      break;
    case JSON_NUMBER:
      printf("NUMBER:%.*s\n", JOBJECT_STRING_LEN(jobject), JOBJECT_STRING_PTR(jobject));
      break;
    case JSON_OBJECT:
      printf("OBJECT: {\n");
      dump(jhandle, OBJECT_FIRST_KEY(jhandle, jobject), 1);
      printf("}\n");
      break;
    case JSON_ARRAY:
      printf("ARRAY: [\n");
      dump(jhandle, ARRAY_FIRST(jhandle, jobject), 1);
      printf("]\n");
      break;
    case JSON_TRUE:
      printf("true\n");
      break;
    case JSON_FALSE:
      printf("false\n");
      break;
    case JSON_NULL:
      printf("null\n");
      break;
    }

    if (count == 0) {    
      jobject = (void *)0;
    } else {
      jobject = JOBJECT_NEXT(jhandle,jobject);
    }
  }

}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void json_dump(struct jhandle_s *jhandle, struct jobject_s *jobject) {

  if (!jobject) jobject = JOBJECT_ROOT(jhandle);
  
  dump(jhandle, jobject, 0);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
