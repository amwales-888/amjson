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
#include <stdarg.h>

#include "amjson.h"
#include "amjson_mod.h"

/* -------------------------------------------------------------------- */

extern struct jobject *jobject_allocate(struct jhandle *jhandle, joff_t count);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static joff_t amjson_strdup(struct jhandle *jhandle, char *ptr, jsize_t len);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static joff_t amjson_strdup(struct jhandle *jhandle, char *ptr, jsize_t len) {

  char *dptr = (char *)jobject_allocate(jhandle,
					(len + (sizeof(struct jobject)-1)) / sizeof(struct jobject));

  if (dptr == (void *)0) return (joff_t)-1;
  
  memcpy(dptr, ptr, len);
  return JOBJECT_OFFSET(jhandle, dptr);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_string_new(struct jhandle *jhandle,
			    char *ptr, jsize_t len) {

  if (!jhandle->hasdecoded) {  

    joff_t offset = amjson_strdup(jhandle, ptr, len);
    if (offset != (joff_t)-1) {
  
      struct jobject *jobject = jobject_allocate(jhandle, 1);
      if (jobject == (void *)0) return (void *)0;

      jobject->blen            = len | (AMJSON_STRING << AMJSON_LENBITS);
      jobject->next            = AMJSON_INVALID;
      jobject->u.string.offset = offset;
      return jobject;
    }
  }
  
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_object_add(struct jhandle *jhandle,
				struct jobject *object,
				struct jobject *string,
				struct jobject *value) {

  if ((!jhandle->hasdecoded) &&
      (JOBJECT_TYPE(object) == AMJSON_OBJECT)) {

    string->next = JOBJECT_OFFSET(jhandle, value);

    if (OBJECT_COUNT(object) == 0) {
      
      object->u.object.child = JOBJECT_OFFSET(jhandle, string);

    } else {

      struct jobject *jobject;
      joff_t next = object->u.object.child;
      
      for (;;) {

	jobject = JOBJECT_AT(jhandle, next);
	if (jobject->next == AMJSON_INVALID) break;
	
	next = jobject->next;
      }

      jobject->next = JOBJECT_OFFSET(jhandle, string);
    }
    
    object->blen = (OBJECT_COUNT(object) + 2) | (AMJSON_OBJECT << AMJSON_LENBITS);
    return object;
  }

  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_object_new(struct jhandle *jhandle, ...) {

  if (!jhandle->hasdecoded) {  

    struct jobject *object;
    va_list ap;

    joff_t last  = AMJSON_INVALID;
    joff_t first = AMJSON_INVALID;

    jsize_t count = 0;
  
    va_start(ap, jhandle);

    for(;;) {
    
      struct jobject *string;
      struct jobject *value;
      struct jobject *jobject;

      string = va_arg(ap, struct jobject *);
      if (string == (void *)0) {
	break;
      }

      count++;
      if (last == AMJSON_INVALID) {
	first = JOBJECT_OFFSET(jhandle, string);
	last  = first;
      } else {
	jobject = JOBJECT_AT(jhandle, last);
	jobject->next = JOBJECT_OFFSET(jhandle, string);
	last = jobject->next;
      }

      value = va_arg(ap, struct jobject *);

      count++;
      jobject = JOBJECT_AT(jhandle, last);
      jobject->next = JOBJECT_OFFSET(jhandle, value);
      last = jobject->next;
    }

    va_end(ap);

    object = jobject_allocate(jhandle, 1);
    if (object == (void *)0) return (void *)0;

    object->blen           = count | (AMJSON_OBJECT << AMJSON_LENBITS);
    object->next           = AMJSON_INVALID;
    object->u.object.child = first;
    return object;
  }

  return (void *)0;  
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_array_new(struct jhandle *jhandle, ...) {

  if (!jhandle->hasdecoded) {  

    struct jobject *array;
    va_list ap;

    joff_t last  = AMJSON_INVALID;
    joff_t first = AMJSON_INVALID;

    jsize_t count = 0;
  
    va_start(ap, jhandle);

    for(;;) {
    
      struct jobject *value = va_arg(ap, struct jobject *);
      if (value == (void *)0) {
	break;
      }

      count++;
      if (last == AMJSON_INVALID) {
	first = JOBJECT_OFFSET(jhandle, value);
	last  = first;
      } else {
	struct jobject *jobject = JOBJECT_AT(jhandle, last);

	jobject->next = JOBJECT_OFFSET(jhandle, value);
	last = jobject->next;
      }
    }
  
    va_end(ap);

    array = jobject_allocate(jhandle, 1);
    if (array == (void *)0) return (void *)0;

    array->blen           = count | (AMJSON_ARRAY << AMJSON_LENBITS);
    array->next           = AMJSON_INVALID;
    array->u.object.child = first;
    return array;
  }
  
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *amjson_array_add(struct jhandle *jhandle,
			       struct jobject *array,
			       struct jobject *value) {

  if ((!jhandle->hasdecoded) &&
      (JOBJECT_TYPE(array) == AMJSON_ARRAY)) {

    if (ARRAY_COUNT(array) == 0) {
      
      array->u.object.child = JOBJECT_OFFSET(jhandle, value);

    } else {

      struct jobject *jobject;
      joff_t next = array->u.object.child;
      
      for (;;) {

	jobject = JOBJECT_AT(jhandle, next);
	if (jobject->next == AMJSON_INVALID) break;
	
	next = jobject->next;
      }
      
      jobject->next = JOBJECT_OFFSET(jhandle, value);
    }

    array->blen = (ARRAY_COUNT(array) + 1) | (AMJSON_ARRAY << AMJSON_LENBITS);
    return array;
  }

  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
