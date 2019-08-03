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

#include "amjson.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static char *amjson_element(struct jhandle * const jhandle, char * const optr);
static char *amjson_object(struct jhandle * const jhandle, char * const optr);
static char *amjson_array(struct jhandle * const jhandle, char * const optr);
static char *amjson_value(struct jhandle * const jhandle, char * const optr);
static char *amjson_string(struct jhandle * const jhandle, char * const optr);
static char *amjson_digit(char *ptr, char * const xeptr);
static char *amjson_integer(char *ptr, char * const xeptr);
static char *amjson_fraction(char *ptr, char * const xeptr);
static char *amjson_exponent(char *ptr, char * const xeptr);
static char *amjson_number(struct jhandle * const jhandle, char * const optr);
static char *amjson_true(struct jhandle * const jhandle, char * const optr);
static char *amjson_false(struct jhandle * const jhandle, char * const optr);
static char *amjson_null(struct jhandle * const jhandle, char * const optr);

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
int amjson_alloc(struct jhandle * const jhandle, struct jobject *ptr,
               unsigned int count) {

  memset(jhandle, 0, sizeof(struct jhandle));
  
  jhandle->count = count;
  jhandle->root  = AMJSON_INVALID;

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
void amjson_free(struct jhandle *jhandle) {

  if (!jhandle->userbuffer) {
    free(jhandle->jobject);
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int amjson_decode(struct jhandle * const jhandle, char *buf, jsize_t len) {

  struct jobject *object;

  jhandle->buf  = buf;
  jhandle->len  = len;
  jhandle->eptr = &buf[len];
  
  jhandle->max_depth = AMJSON_MAXDEPTH;
  jhandle->depth     = 0;

  jhandle->useljmp = 1;
  if (setjmp(jhandle->setjmp_ctx) == 1) {

    /* We returned from calling amjson_element() with an 
     * allocation failure.
     */
    errno = ENOMEM;
    return -1;
  }
  
  jhandle->hasdecoded = 1;  /* Prevent user from modifying 
			     * jobject pool */

  if (amjson_element(jhandle, buf) == buf) {

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
struct jobject *jobject_allocate(struct jhandle * const jhandle, joff_t count) {

  joff_t used = jhandle->used + count;

  if (used <= jhandle->used) goto error; /* overflow */
  
  if (used < jhandle->count) {

    struct jobject *jobject = &jhandle->jobject[jhandle->used];
    jhandle->used = used;
    
    return jobject;
  }

  if (!jhandle->userbuffer) {
    
    void *ptr;
    joff_t ncount = (jhandle->count * 2) + count;
    
    if (ncount <= jhandle->count) goto error; /* overflow */
        
    ptr = realloc(jhandle->jobject, (ncount * sizeof(struct jobject)));	  
    if (ptr) {
      jhandle->count   = ncount;
      jhandle->jobject = ptr;
      return jobject_allocate(jhandle, count);
    }
  }

 error:

  if (jhandle->useljmp) {
    /* Jump right back to amjson_decode() 
     */   
    longjmp(jhandle->setjmp_ctx, 1);  
  }
    
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_element(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
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

  nptr = amjson_value(jhandle, ptr);
  if (nptr == ptr) goto fail;
  ptr = nptr;
  
  CONSUME_WHITESPACE(ptr, eptr);
  if (eptr == ptr) return ptr;

 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_object(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;
  struct jobject *object;

  joff_t first  = AMJSON_INVALID;
  jsize_t count = 0;

  jhandle->depth++;
  
  if (eptr == ptr) goto fail;
  if ((*ptr == '{') &&
      (jhandle->depth < jhandle->max_depth)) {

    char *nptr;
    char *comma = (void *)0;
    struct jobject *string;
    struct jobject *value;
    struct jobject *jobject;

    joff_t last = AMJSON_INVALID;

    ptr++;
    CONSUME_WHITESPACE(ptr, eptr);

    if (eptr == ptr) goto fail;  
    if (*ptr == '}') {
      ptr++;
      goto success;
    } 

  nextobject:

    CONSUME_WHITESPACE(ptr, eptr);

    nptr = amjson_string(jhandle, ptr);
    if (nptr == ptr) {
      if (comma) ptr = comma;
      
      goto fail;
    }
    ptr = nptr;

    /* Add string to list */
    count++;
    
    string = JOBJECT_LAST(jhandle);
    if (last == AMJSON_INVALID) {
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
    
    nptr = amjson_value(jhandle, ptr);
    if (nptr == ptr) goto fail;

    ptr = nptr;

    /* Add value to list */
    count++;
    
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
 
  object->blen           = count | (AMJSON_OBJECT << AMJSON_LENBITS);
  object->next           = AMJSON_INVALID;
  object->u.object.child = first;

  jhandle->depth--;
  return ptr;  
 fail:
  jhandle->depth--;
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_array(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;
  struct jobject *array;

  joff_t first  = AMJSON_INVALID;
  jsize_t count = 0;

  jhandle->depth++;

  if (eptr == ptr) goto fail;  
  if ((*ptr == '[') &&
      (jhandle->depth < jhandle->max_depth)) {

    char *nptr;
    char *comma = (void *)0;
    struct jobject *value;
    struct jobject *jobject;
    joff_t last = AMJSON_INVALID;
  
    ptr++;
    CONSUME_WHITESPACE(ptr, eptr);

    if (eptr == ptr) goto fail;  
    if (*ptr == ']') {
      ptr++;
      goto success;
    } 
    
  nextvalue:

    nptr = amjson_value(jhandle, ptr);
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
    
    value = JOBJECT_LAST(jhandle);
    if (last == AMJSON_INVALID) {
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
  
  array->blen           = count | (AMJSON_ARRAY << AMJSON_LENBITS);
  array->next           = AMJSON_INVALID;
  array->u.object.child = first;
  
  jhandle->depth--;
  return ptr;
 fail:
  jhandle->depth--;
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_value(struct jhandle * const jhandle, char * const optr) {
  
  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;
  char *nptr;

  CONSUME_WHITESPACE(ptr, eptr);
  if (ptr == eptr) goto fail;

  if (*ptr == '"') {
    nptr = amjson_string(jhandle, ptr);
  } else if (((unsigned char)(*ptr - '0') < 10) ||
	     (*ptr == '-')) {
    nptr = amjson_number(jhandle, ptr);
  } else if (*ptr == '{') {
    nptr = amjson_object(jhandle, ptr);
  } else if (*ptr == '[') {
    nptr = amjson_array(jhandle, ptr);
  } else if (*ptr == 't') {
    nptr = amjson_true(jhandle, ptr);
  } else if (*ptr == 'f') {
    nptr = amjson_false(jhandle, ptr);
  } else if (*ptr == 'n') {
    nptr = amjson_null(jhandle, ptr);
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
static char *amjson_string(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;
  struct jobject *jobject;
  jsize_t len;
  
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

  len = (ptr-1) - (optr+1); /* overflow??? */
  
  jobject = jobject_allocate(jhandle, 1);

  jobject->blen            = len | (AMJSON_STRING << AMJSON_LENBITS);
  jobject->next            = AMJSON_INVALID;
  jobject->u.string.offset = (optr+1) - jhandle->buf;
  return ptr;
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_digit(char *xptr, char * const xeptr) {

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
static char *amjson_integer(char *optr, char * const xeptr) {

  register char *ptr = optr;
  register char * const eptr = xeptr;
  
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
static char *amjson_fraction(char *optr, char * const xeptr) {

  register char *ptr = optr;
  register char * const eptr = xeptr;

  if (eptr == ptr) goto fail;  
  if (*ptr == '.') {

    char *nptr;
    ptr++;

    nptr = amjson_digit(ptr, eptr);
    if (nptr == ptr) goto fail;

    ptr = nptr;

    for (;;) {
      nptr = amjson_digit(ptr, eptr);
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
static char *amjson_exponent(char *optr, char * const xeptr) {

  register char *ptr = optr;
  register char * const eptr = xeptr;

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

    nptr = amjson_digit(ptr, eptr);
    if (nptr == ptr) goto fail;

    ptr = nptr;

    for (;;) {
      nptr = amjson_digit(ptr, eptr);
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
static char *amjson_number(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;
  struct jobject *jobject;
  char *nptr;
  jsize_t len;

  if (eptr == ptr) goto fail;  
  if (*ptr == '-') {
    ptr++;
  }

  nptr = amjson_integer(ptr, eptr);
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

  if ((eptr != ptr) && (*ptr == '.')) ptr = amjson_fraction(ptr, eptr);

  if ((eptr != ptr) &&
      ((*ptr == 'e') || (*ptr == 'E'))) ptr = amjson_exponent(ptr, eptr);

  len = ptr - optr; /* overflow??? */
  
  jobject = jobject_allocate(jhandle, 1);

  jobject->blen            = len | (AMJSON_NUMBER << AMJSON_LENBITS);
  jobject->next            = AMJSON_INVALID;
  jobject->u.string.offset = optr - jhandle->buf;
  return ptr;  
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_true(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;
  
  if (((eptr - ptr) >= 4) &&
      ((ptr[0] == 't') && (ptr[1] == 'r') && 
       (ptr[2] == 'u') && (ptr[3] == 'e'))) {

    struct jobject *jobject = jobject_allocate(jhandle, 1);
    jobject->blen           = AMJSON_TRUE << AMJSON_LENBITS; 
    jobject->next           = AMJSON_INVALID;
    return ptr + 4;
  } 

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_false(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;

  if (((eptr - ptr) >= 5) &&
      ((ptr[0] == 'f') && (ptr[1] == 'a') &&
       (ptr[2] == 'l') && (ptr[3] == 's') &&
       (ptr[4] == 'e'))) {

    struct jobject *jobject = jobject_allocate(jhandle, 1);
    jobject->blen           = AMJSON_FALSE << AMJSON_LENBITS; 
    jobject->next           = AMJSON_INVALID;
    return ptr + 5;
  } 

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *amjson_null(struct jhandle * const jhandle, char * const optr) {

  register char * const eptr = jhandle->eptr;
  register char *ptr = optr;

  if (((eptr - ptr) >= 4) &&
      ((ptr[0] == 'n') && (ptr[1] == 'u') &&
       (ptr[2] == 'l') && (ptr[3] == 'l'))) {

    struct jobject *jobject = jobject_allocate(jhandle, 1);
    jobject->blen           = AMJSON_NULL << AMJSON_LENBITS; 
    jobject->next           = AMJSON_INVALID;
    return ptr + 4;
  } 

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
