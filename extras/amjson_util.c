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

#include "amjson.h"
#include "extras/amjson_util.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_array_index(struct jhandle *jhandle,
				   struct jobject *array, joff_t index) {
  joff_t next; 

  if (index >= ARRAY_COUNT(array)) return (struct jobject *)0;

  next = array->u.object.child;
  while (index--) {
    struct jobject *jobject = JOBJECT_AT(jhandle, next);
    next = jobject->next;    
  }

  return JOBJECT_AT(jhandle, next);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_object_find(struct jhandle *jhandle,
				   struct jobject *object,
				   char *key,
				   jsize_t len) {
  joff_t next;

  if (OBJECT_COUNT(object) == 0) return (struct jobject *)0;

  next = object->u.object.child;
  do {

    struct jobject *jobject = JOBJECT_AT(jhandle, next);

    if ((JOBJECT_STRING_LEN(jobject) == len) &&
	(memcmp(&jhandle->buf[jobject->u.string.offset],
		key, len) == 0)) {
      return JOBJECT_AT(jhandle, jobject->next);
    }

    jobject = JOBJECT_AT(jhandle, jobject->next);
    next = jobject->next;
        
  } while (next != AMJSON_INVALID);
  
  return (struct jobject *)0;
}
