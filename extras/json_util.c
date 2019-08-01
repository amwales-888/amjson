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

#include <string.h>

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *array_index(struct jhandle *jhandle,
			      struct jobject *array, unsigned int index) {

  int next; 

  if (index >= array->u.object.count) return (void *)0;

  next = array->u.object.child;
  while (index--) {
    struct jobject *jobject = JOBJECT_AT(jhandle, next);
    next = jobject->bnext & JSON_NEXTMASK;    
  }

  return JOBJECT_AT(jhandle, next);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *object_find(struct jhandle *jhandle,
			    struct jobject *object,
			    char *key,
			    unsigned int len) {

  int next;

  if (object->u.object.count == 0) return (void *)0;

  next = object->u.object.child;
  do {

    struct jobject *jobject = JOBJECT_AT(jhandle, next);

    if ((jobject->u.string.len == len) &&
	(memcmp(&jhandle->buf[jobject->u.string.offset],
		key, len) == 0)) {
      return JOBJECT_AT(jhandle, jobject->bnext & JSON_NEXTMASK);
    }

    jobject = JOBJECT_AT(jhandle, jobject->bnext & JSON_NEXTMASK);
    next = jobject->bnext & JSON_NEXTMASK;
        
  } while (next != -1);
  
  return (void *)0;
}
