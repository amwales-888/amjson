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

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static char *json_element(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_object(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_array(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_value(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_string(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_digit(char *ptr, char * const eptr);
static char *json_integer(char *ptr, char * const eptr);
static char *json_fraction(char *ptr, char * const eptr);
static char *json_exponent(char *ptr, char * const eptr);
static char *json_number(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_true(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_false(struct jhandle * const jhandle, char * const optr, char * const eptr);
static char *json_null(struct jhandle * const jhandle, char * const optr, char * const eptr);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/*  hexdigit is one of:    '0'-'9'  48-57
                           'A'-'Z'  65-90  
                           'a'-'z'  97-122
*/
static unsigned char const hexdigit[] = {

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
  0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

/* escaped is one of: '"', '\\', '/', 'b', 'f', 'n', 'r', 't'
*/
static unsigned char const escaped[] = {

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
  0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  
/* whitespace is one of: '\t'  9
                         '\n' 10
                         '\r' 13
                         ' '  32
*/
static unsigned char const whitespace[] = {

  0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

#define CONSUME_WHITESPACE(ptr, eptr)                       \
                                                            \
  do {							    \
    while (((ptr) < (eptr)) &&                              \
           (whitespace[(unsigned char)(*(ptr))])) (ptr)++;  \
  } while (0)

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int json_alloc(struct jhandle * const jhandle, struct jobject *ptr,
               unsigned int count) {

  memset(jhandle, 0, sizeof(struct jhandle));
  
  jhandle->count = count;
  jhandle->root  = JSON_INVALID;

  if (ptr) {
    jhandle->userbuffer = (unsigned int)1;
    jhandle->jobject    = ptr;
    return 0;
  }

  if ((jhandle->jobject = (struct jobject *)malloc((size_t)jhandle->count *
						   sizeof(struct jobject)))) {
    return 0;
  }

  errno = EINVAL;
  return -1;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void json_free(struct jhandle *jhandle) {

  if (jhandle->onfree) {
    jhandle->onfree(jhandle);
  }

  if (!jhandle->userbuffer) {
    free(jhandle->jobject);
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int json_decode(struct jhandle * const jhandle, char *buf, size_t len) {

  struct jobject *object;

#ifndef BIGJSON
  /* We only support files >4GB if BIGJSON is defined 
   */
  if (len >= 0xFFFFFFFF) {
    errno = EINVAL;
    return -1;
  }
#endif

  jhandle->buf = buf;
  jhandle->len = len;
  
  jhandle->max_depth = JSON_MAXDEPTH;
  jhandle->depth     = 0;

  jhandle->useljmp = 1;
  if (setjmp(jhandle->setjmp_ctx) == 1) {

    /* We returned from calling json_element() with an 
     * allocation failure.
     */
    errno = ENOMEM;
    return -1;
  }
  
  jhandle->hasdecoded = 1;  /* Prevent user from modifying 
			     * jobject pool */

  if (json_element(jhandle, buf, &buf[len]) == buf) {

    errno = EINVAL;
    return -1;
  }

  /* Our root object can be almost anything.
   * The only guarentee we have is that it is something
   * other than whitespace.
   */
  object = JOBJECT_LAST(jhandle);
  jhandle->root = JOBJECT_OFFSET(jhandle, object);
  
  return 0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *jobject_allocate(struct jhandle * const jhandle, unsigned int count) {

  if (jhandle->used + count < jhandle->count) {

    struct jobject *jobject = &jhandle->jobject[jhandle->used];
    jhandle->used += count;
    
    return jobject;
  }

  if (!jhandle->userbuffer) {
    
    void *ptr;
    
    jhandle->count = (jhandle->count * 2) + count;
    ptr = realloc(jhandle->jobject, (jhandle->count * sizeof(struct jobject)));	  
    if (ptr) {
      jhandle->jobject = ptr;
      return jobject_allocate(jhandle, count);
    }
  }

  if (jhandle->useljmp) {
    /* Jump right back to json_decode() 
     */   
    longjmp(jhandle->setjmp_ctx, 1);  
  }
    
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_element(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  register char *ptr = optr;
  char *nptr;

  /* Consume UTF-8 BOM if it is present */
  if (((eptr - ptr) >= 3) &&
      ((ptr[0] == ((char)(0xEF))) &&
       (ptr[1] == ((char)(0xBB))) && 
       (ptr[2] == ((char)(0xBF))))) {
	
	ptr += 3;
  } 

  if (eptr == ptr) goto fail;
  CONSUME_WHITESPACE(ptr, eptr);

  nptr = json_value(jhandle, ptr, eptr);
  if (nptr == ptr) goto fail;
  ptr = nptr;
  
  CONSUME_WHITESPACE(ptr, eptr);
  if (eptr == ptr) return ptr;

 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_object(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  register char *ptr = optr;
  struct jobject *object;

  unsigned int first = JSON_INVALID;
  unsigned int count = 0;

  jhandle->depth++;
  
  if (eptr == ptr) goto fail;
  if ((*ptr == '{') &&
      (jhandle->depth < jhandle->max_depth)) {

    char *nptr;
    char *comma = (void *)0;
    struct jobject *string;
    struct jobject *value;
    struct jobject *jobject;

    unsigned int last = JSON_INVALID;

    ptr++;
    CONSUME_WHITESPACE(ptr, eptr);

    if (eptr == ptr) goto fail;  
    if (*ptr == '}') {
      ptr++;
      goto success;
    } 

  nextobject:

    CONSUME_WHITESPACE(ptr, eptr);

    nptr = json_string(jhandle, ptr, eptr);
    if (nptr == ptr) {
      if (comma) ptr = comma;
      
      goto fail;
    }
    ptr = nptr;

    /* Add string to list */
    count++;
#ifndef BIGJSON
    if (count > JSON_MAXLEN) goto fail; /* overflow */
#endif
    
    string = JOBJECT_LAST(jhandle);
    if (last == JSON_INVALID) {
      first = JOBJECT_OFFSET(jhandle, string);
      last  = first;
    } else {
      jobject = JOBJECT_AT(jhandle, last);
      jobject->next = JOBJECT_OFFSET(jhandle, string);
      last = jobject->next;
    }
    /* String added */
        
    CONSUME_WHITESPACE(ptr, eptr);

    if ((eptr == ptr) ||
	(*ptr != ':')) {
      goto fail;
    }

    ptr++;
    
    nptr = json_value(jhandle, ptr, eptr);
    if (nptr == ptr) goto fail;

    ptr = nptr;

    /* Add value to list */
    count++;
#ifndef BIGJSON
    if (count > JSON_MAXLEN) goto fail; /* overflow */
#endif
    
    value = JOBJECT_LAST(jhandle);
    jobject = JOBJECT_AT(jhandle, last);
    jobject->next = JOBJECT_OFFSET(jhandle, value);
    last = jobject->next;
    /* Value added */
    
    if (eptr == ptr) goto fail;  
    if (*ptr == '}') {
      ptr++;
      goto success;
    } else if (*ptr == ',') {
      comma = ptr;
      ptr++;
      goto nextobject;
    } 
  }

  goto fail;

 success:
  
  object = jobject_allocate(jhandle, 1);
  
  object->type           = JSON_OBJECT;
  object->len            = count;
  object->u.object.child = first;
  object->next           = JSON_INVALID;

  jhandle->depth--;
  return ptr;  
 fail:
  jhandle->depth--;
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_array(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  register char *ptr = optr;
  struct jobject *array;

  unsigned int first = JSON_INVALID;
  unsigned int count = 0;

  jhandle->depth++;

  if (eptr == ptr) goto fail;  
  if ((*ptr == '[') &&
      (jhandle->depth < jhandle->max_depth)) {

    char *nptr;
    char *comma = (void *)0;
    struct jobject *value;
    struct jobject *jobject;
    unsigned int last = JSON_INVALID;
  
    ptr++;
    CONSUME_WHITESPACE(ptr, eptr);

    if (eptr == ptr) goto fail;  
    if (*ptr == ']') {
      ptr++;
      goto success;
    } 
    
  nextvalue:

    nptr = json_value(jhandle, ptr, eptr);
    if (nptr == ptr) {
      if (comma) ptr = comma;

      if (eptr == ptr) goto fail;  
      if (*ptr == ']') {
	ptr++;
	goto success;
      }
      goto fail;
    }
    ptr = nptr;

    /* Add value to list */
    count++;
#ifndef BIGJSON
    if (count > JSON_MAXLEN) goto fail;
#endif
    
    value = JOBJECT_LAST(jhandle);
    if (last == JSON_INVALID) {
      first = JOBJECT_OFFSET(jhandle, value);
      last  = first;
    } else {
      jobject = JOBJECT_AT(jhandle, last);
      jobject->next = JOBJECT_OFFSET(jhandle, value);
      last = jobject->next;
    }
    /* Value added */
    
    if (eptr == ptr) goto fail;  
    if (*ptr == ']') {
      ptr++;
      goto success;
    } else if (*ptr == ',') {
      comma = ptr;
      ptr++;
      goto nextvalue;
    } 
  }

  goto fail;

 success:
 
  array = jobject_allocate(jhandle, 1);
  
  array->type           = JSON_ARRAY;
  array->len            = count;
  array->u.object.child = first;
  array->next           = JSON_INVALID;  
  
  jhandle->depth--;
  return ptr;
 fail:
  jhandle->depth--;
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_value(struct jhandle * const jhandle, char * const optr, char * const eptr) {
  
  register char *ptr = optr;
  char *nptr;

  CONSUME_WHITESPACE(ptr, eptr);
  if (ptr == eptr) goto fail;

  if (*ptr == '"') {
    nptr = json_string(jhandle, ptr, eptr);
  } else if (((unsigned char)(*ptr - '0') < 10) ||
	     (*ptr == '-')) {
    nptr = json_number(jhandle, ptr, eptr);
  } else if (*ptr == '{') {
    nptr = json_object(jhandle, ptr, eptr);
  } else if (*ptr == '[') {
    nptr = json_array(jhandle, ptr, eptr);
  } else if (*ptr == 't') {
    nptr = json_true(jhandle, ptr, eptr);
  } else if (*ptr == 'f') {
    nptr = json_false(jhandle, ptr, eptr);
  } else if (*ptr == 'n') {
    nptr = json_null(jhandle, ptr, eptr);
  } else {
    goto fail;
  }
  
  if (nptr == ptr) goto fail;
  ptr = nptr;

  CONSUME_WHITESPACE(ptr, eptr);
  return ptr;
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_string(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  struct jobject *jobject;
  register char *ptr = optr;
#ifndef BIGJSON
  unsigned int len;
#endif
  
  if (eptr == ptr) goto fail;
  if (*ptr == '"') {
    ptr++;

  nextchar:    

    /* Must not contain control characters */
    if ((eptr == ptr) ||
	((unsigned char)(*ptr) < 32)) {
      goto fail;
    }
    
    if (*ptr == '\\') {
      ptr++;

      if (eptr == ptr) goto fail;  
      if (escaped[(unsigned char)(*ptr)]) {

	ptr++;
	goto nextchar;

      } else if (*ptr == 'u') {
	ptr++;
	
	if (((eptr - ptr) >= 4) &&
	    ((hexdigit[(unsigned char)(ptr[0])]) &&
	     (hexdigit[(unsigned char)(ptr[1])]) &&
	     (hexdigit[(unsigned char)(ptr[2])]) &&
	     (hexdigit[(unsigned char)(ptr[3])]))) {

	  ptr += 4;
	  goto nextchar;
	}

	goto fail;	  
	
      } else {
	goto fail;
      }
    } else if (*ptr == '"') {
      ptr++;
      goto success;
    } else {
      ptr++;
      goto nextchar;
    }
  }

  goto fail;

 success:

#ifndef BIGJSON
  len = (ptr-1) - (optr+1);
  if (len > JSON_MAXLEN) goto fail; /* overflow */
#endif
  
  jobject = jobject_allocate(jhandle, 1);

  jobject->type            = JSON_STRING;
#ifndef BIGJSON
  jobject->len             = len;
#else
  jobject->len             = (ptr-1) - (optr+1);
#endif
  jobject->u.string.offset = (optr+1) - jhandle->buf;
  jobject->next            = JSON_INVALID;
  return ptr;
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_digit(char *xptr, char * const xeptr) {

  register char *ptr  = xptr;
  register char * const eptr = xeptr;
  
  if ((eptr != ptr) &&
      ((unsigned char)(*ptr - '0') < 10)) {
    ptr++;
  }
    
  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_integer(char *optr, char * const eptr) {

  register char *ptr = optr;
  
  if ((eptr != ptr) && 
      ((unsigned char)(*ptr - '1') < 9)) {
    ptr++;

  nextdigit:
    if ((eptr != ptr) &&
	((unsigned char)(*ptr - '0') < 10)) {
      ptr++;
      goto nextdigit;
    }

    return ptr;
  }

  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_fraction(char *optr, char * const eptr) {

  register char *ptr = optr;

  if (eptr == ptr) goto fail;  
  if (*ptr == '.') {

    char *nptr;
    ptr++;

    nptr = json_digit(ptr, eptr);
    if (nptr == ptr) goto fail;

    ptr = nptr;

    for (;;) {
      nptr = json_digit(ptr, eptr);
      if (nptr == ptr) goto success;

      ptr = nptr;
    }
  } 

 success:
  return ptr;
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_exponent(char *optr, char * const eptr) {

  register char *ptr = optr;

  if (eptr == ptr) goto fail;  
  if ((*ptr == 'E') || (*ptr == 'e'))  {

    char *nptr;
    ptr++;

    if (eptr == ptr) goto fail;  
    if (*ptr == '+') {
      ptr++;
    } else if (*ptr == '-') {
      ptr++;
    }

    nptr = json_digit(ptr, eptr);
    if (nptr == ptr) goto fail;

    ptr = nptr;

    for (;;) {
      nptr = json_digit(ptr, eptr);
      if (nptr == ptr) goto success;

      ptr = nptr;
    }
  }

 success:
  return ptr;
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_number(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  struct jobject *jobject;
  register char *ptr = optr;
#ifndef BIGJSON
  unsigned int len;
#endif
  char *nptr;

  if (eptr == ptr) goto fail;  
  if (*ptr == '-') {
    ptr++;
  }

  nptr = json_integer(ptr, eptr);
  if (nptr == ptr) {
    if (eptr == ptr) goto fail;  
    if (*ptr == '0') {
      ptr++;
      goto fraction;
    }
    goto fail;
  }
  ptr = nptr;
  
 fraction:

  if ((eptr != ptr) && (*ptr == '.')) ptr = json_fraction(ptr, eptr);

  if ((eptr != ptr) &&
      ((*ptr == 'e') || (*ptr == 'E'))) ptr = json_exponent(ptr, eptr);

#ifndef BIGJSON
  len = ptr - optr;
  if (len > JSON_MAXLEN) goto fail; /* overflow */
#endif
  
  jobject = jobject_allocate(jhandle, 1);
  
  jobject->type            = JSON_NUMBER;
#ifndef BIGJSON
  jobject->len             = len;
#else
  jobject->len             = ptr - optr;
#endif
  jobject->u.string.offset = optr - jhandle->buf;
  jobject->next            = JSON_INVALID;
  return ptr;  
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_true(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  register char *ptr = optr;
  
  if (((eptr - ptr) >= 4) &&
      ((ptr[0] == 't') && (ptr[1] == 'r') && 
       (ptr[2] == 'u') && (ptr[3] == 'e'))) {

    struct jobject *jobject = jobject_allocate(jhandle, 1);
    jobject->type = JSON_TRUE;
    jobject->next = JSON_INVALID;
    return ptr + 4;
  } 

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_false(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  register char *ptr = optr;

  if (((eptr - ptr) >= 5) &&
      ((ptr[0] == 'f') && (ptr[1] == 'a') &&
       (ptr[2] == 'l') && (ptr[3] == 's') &&
       (ptr[4] == 'e'))) {

    struct jobject *jobject = jobject_allocate(jhandle, 1);
    jobject->type = JSON_FALSE;
    jobject->next = JSON_INVALID;
    return ptr + 5;
  } 

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_null(struct jhandle * const jhandle, char * const optr, char * const eptr) {

  register char *ptr = optr;

  if (((eptr - ptr) >= 4) &&
      ((ptr[0] == 'n') && (ptr[1] == 'u') &&
       (ptr[2] == 'l') && (ptr[3] == 'l'))) {

    struct jobject *jobject = jobject_allocate(jhandle, 1);
    jobject->type = JSON_NULL;
    jobject->next = JSON_INVALID;
    return ptr + 4;
  } 

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
