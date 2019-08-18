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
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "amjson.h"

/* -------------------------------------------------------------------- */

#ifdef USEBRANCHHINTS
#define AM_LIKELY(x)       __builtin_expect(!!(x), 1)
#define AM_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#define AM_LIKELY(x)       (x)
#define AM_UNLIKELY(x)     (x)
#endif

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static void amjson_element(struct jhandle * const jhandle, char **optr);
static void amjson_object(struct jhandle * const jhandle, char **optr);
static void amjson_array(struct jhandle * const jhandle, char **optr);
static void amjson_value(struct jhandle * const jhandle, char **optr);
static void amjson_string(struct jhandle * const jhandle, char **optr);
static void amjson_integer(struct jhandle * const jhandle, char **optr);
static void amjson_fraction(struct jhandle * const jhandle, char **optr);
static void amjson_exponent(struct jhandle * const jhandle, char **optr);
static void amjson_number(struct jhandle * const jhandle, char **optr);
static void amjson_true(struct jhandle * const jhandle, char **optr);
static void amjson_false(struct jhandle * const jhandle, char **optr);
static void amjson_null(struct jhandle * const jhandle, char **optr);

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
    for (;;) {                                              \
      if (AM_UNLIKELY((ptr) == (eptr))) break;		    \
      if (!whitespace[(unsigned char)(*(ptr))]) break;      \
      ptr++;                                                \
    }                                                       \
  } while (0)

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int amjson_alloc(struct jhandle * const jhandle, struct jobject *ptr,
		 joff_t count) {

  memset(jhandle, 0, sizeof(struct jhandle));
  
  jhandle->count = count;
  jhandle->root  = AMJSON_INVALID;

  if (ptr) {
    jhandle->userbuffer = (unsigned int)1;
    jhandle->jobject    = ptr;
    return 0;
  }

  if (count == 0) goto error;
  if ((jhandle->jobject = (struct jobject *)malloc((size_t)jhandle->count *
						   sizeof(struct jobject)))) {
    return 0;
  }

 error:
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
int amjson_decode(struct jhandle * const jhandle, char *buf, bsize_t len) {

  struct jobject *object;
  char *ptr = buf;

  jhandle->buf       = buf;
  jhandle->len       = len;
  jhandle->eptr      = &buf[len];
  jhandle->max_depth = AMJSON_MAXDEPTH;
  jhandle->depth     = 0;
  jhandle->useljmp   = 1;

  switch (setjmp(jhandle->setjmp_ctx)) {

  case 1:
    /* We returned from calling amjson_element() with an 
     * allocation failure.
     */
    errno = ENOMEM;
    return -1;

  case 2:

    /* We returned from calling amjson_element() with an 
     * parser failure.
     */
    errno = EINVAL;
    return -1;
  }
  
  amjson_element(jhandle, &ptr);

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

  if (AM_UNLIKELY(used <= jhandle->used)) goto error; /* overflow */
  if (AM_LIKELY(used < jhandle->count)) {

    struct jobject *jobject = &jhandle->jobject[jhandle->used];
    jhandle->used = used;
    
    return jobject;
  }

  if (!jhandle->userbuffer) {
    
    void *ptr;
    joff_t ncount = (jhandle->count * 2) + count;
    
    if (AM_UNLIKELY(ncount <= jhandle->count)) goto error; /* overflow */

    ptr = realloc(jhandle->jobject, (ncount * sizeof(struct jobject)));	  
    if (ptr) {
      jhandle->count   = ncount;
      jhandle->jobject = (struct jobject *)ptr;
      return jobject_allocate(jhandle, count);
    }
  }

 error:
  if (jhandle->useljmp) {
    /* The allocator is being used from with amjson_decode() 
     * it's safe to jump right back to amjson_decode() 
     */   
    longjmp(jhandle->setjmp_ctx, 1);  /* jump back to amjson_decode() with ENOMEM */
  }
    
  return (struct jobject *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_element(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;

  /* Consume UTF-8 BOM if it is present */
  if (((eptr - ptr) >= 3) &&
      ((ptr[0] == ((char)(0xEF))) &&
       (ptr[1] == ((char)(0xBB))) && 
       (ptr[2] == ((char)(0xBF))))) {
	
	ptr += 3;
  } 

  if (eptr == ptr) goto fail;
  CONSUME_WHITESPACE(ptr, eptr);

  amjson_value(jhandle, &ptr);
  
  CONSUME_WHITESPACE(ptr, eptr);
  if (eptr != ptr) goto fail;

  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_object(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;
  struct jobject *object;

  joff_t first  = AMJSON_INVALID;
  jsize_t count = 0;

  jhandle->depth++;

  if (jhandle->depth < jhandle->max_depth) {

    struct jobject *string;
    struct jobject *value;
    struct jobject *jobject;

    joff_t last = AMJSON_INVALID;

    ptr++;
    CONSUME_WHITESPACE(ptr, eptr);

    if (AM_UNLIKELY(eptr == ptr)) goto fail;  
    if (AM_UNLIKELY(*ptr == '}')) {
      ptr++;
      goto success;
    } 

  nextobject:

    CONSUME_WHITESPACE(ptr, eptr);

    if (AM_UNLIKELY(eptr == ptr)) goto fail; 
    if (AM_UNLIKELY(*ptr != '"')) goto fail;
      
    amjson_string(jhandle, &ptr);

    /* Add string to list */
    count++;
    
    string = JOBJECT_LAST(jhandle);
    if (count == 1) {
      first = JOBJECT_OFFSET(jhandle, string);
      last  = first;
    } else {
      jobject = JOBJECT_AT(jhandle, last);
      jobject->next = JOBJECT_OFFSET(jhandle, string);
      last = jobject->next;
    }
    /* String added */
        
    CONSUME_WHITESPACE(ptr, eptr);

    if (AM_UNLIKELY(eptr == ptr)) goto fail;
    if (AM_UNLIKELY(*ptr != ':')) goto fail;

    ptr++;
    
    amjson_value(jhandle, &ptr);

    /* Add value to list */
    count++;
    
    value = JOBJECT_LAST(jhandle);
    jobject = JOBJECT_AT(jhandle, last);
    jobject->next = JOBJECT_OFFSET(jhandle, value);
    last = jobject->next;
    /* Value added */
    
    if (AM_UNLIKELY(eptr == ptr)) goto fail;  
    if (*ptr == '}') {
      ptr++;
      goto success;
    } else if (*ptr == ',') {
      ptr++;
      goto nextobject;
    } 
  }

  goto fail;

 success:
  
  object                 = jobject_allocate(jhandle, 1);
  object->blen           = count | (AMJSON_OBJECT << AMJSON_LENBITS);
  object->next           = AMJSON_INVALID;
  object->u.object.child = first;

  jhandle->depth--;

  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_array(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;
  struct jobject *array;

  joff_t first  = AMJSON_INVALID;
  jsize_t count = 0;

  jhandle->depth++;

  if (jhandle->depth < jhandle->max_depth) {

    struct jobject *value;
    struct jobject *jobject;
    joff_t last = AMJSON_INVALID;
  
    ptr++;
    CONSUME_WHITESPACE(ptr, eptr);

    if (AM_UNLIKELY(eptr == ptr)) goto fail;  
    if (AM_UNLIKELY(*ptr == ']')) {
      ptr++;
      goto success;
    } 
    
  nextvalue:

    amjson_value(jhandle, &ptr);

    /* Add value to list */
    count++;
    
    value = JOBJECT_LAST(jhandle);
    if (count == 1) {
      first = JOBJECT_OFFSET(jhandle, value);
      last  = first;
    } else {
      jobject = JOBJECT_AT(jhandle, last);
      jobject->next = JOBJECT_OFFSET(jhandle, value);
      last = jobject->next;
    }
    /* Value added */
    
    if (AM_UNLIKELY(eptr == ptr)) goto fail;  
    if (*ptr == ']') {
      ptr++;
      goto success;
    } else if (*ptr == ',') {
      ptr++;

      CONSUME_WHITESPACE(ptr, eptr);
      if (AM_UNLIKELY(eptr == ptr)) goto fail;  

      goto nextvalue;
    } 
  }

  goto fail;

 success:
 
  array                 = jobject_allocate(jhandle, 1);
  array->blen           = count | (AMJSON_ARRAY << AMJSON_LENBITS);
  array->next           = AMJSON_INVALID;
  array->u.object.child = first;
  
  jhandle->depth--;

  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#ifdef USECOMPUTEDGOTO
static void amjson_value(struct jhandle * const jhandle, char **optr) {
  
  static void *vtbl[] = {

    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLSTRING,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLNUMBER,&&LLFAIL,&&LLFAIL,
    &&LLNUMBER,&&LLNUMBER,&&LLNUMBER,&&LLNUMBER,&&LLNUMBER,&&LLNUMBER,&&LLNUMBER,&&LLNUMBER,
    &&LLNUMBER,&&LLNUMBER,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLARRAY,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFALSE,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLNULL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLTRUE,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLOBJECT,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,
    &&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL,&&LLFAIL };

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;

  CONSUME_WHITESPACE(ptr, eptr);
  if (AM_UNLIKELY(eptr == ptr)) goto fail;

  goto *vtbl[(unsigned char)(*ptr)];

 LLSTRING:
  amjson_string(jhandle, &ptr);
  goto success;
 LLNUMBER:
  amjson_number(jhandle, &ptr);
  goto success;
 LLOBJECT:
  amjson_object(jhandle, &ptr);
  goto success;
 LLARRAY:
  amjson_array(jhandle, &ptr);
  goto success;
 LLTRUE:
  amjson_true(jhandle, &ptr);
  goto success;
 LLFALSE:
  amjson_false(jhandle, &ptr);
  goto success;
 LLNULL:
  amjson_null(jhandle, &ptr);
  goto success;
 LLFAIL:
  goto fail;
  
 success:  
  CONSUME_WHITESPACE(ptr, eptr);
  
  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

#else

static void amjson_value(struct jhandle * const jhandle, char **optr) {
  
  char *ptr = *optr;
  char * const eptr = jhandle->eptr;

  CONSUME_WHITESPACE(ptr, eptr);
  if (AM_UNLIKELY(eptr == ptr)) goto fail;

  switch (*ptr) {

  case '"':
    amjson_string(jhandle, &ptr);
    break;
  case '-':
    amjson_number(jhandle, &ptr);
    break;
  case '{':
    amjson_object(jhandle, &ptr);
    break;
  case '[':
    amjson_array(jhandle, &ptr);
    break;
  case 't':
    amjson_true(jhandle, &ptr);
    break;
  case 'f':
    amjson_false(jhandle, &ptr);
    break;
  case 'n':
    amjson_null(jhandle, &ptr);
    break;
  default:
    if ((unsigned char)(*ptr - '0') < 10) {
      amjson_number(jhandle, &ptr);
    } else {
      goto fail;
    }
  }
  
  CONSUME_WHITESPACE(ptr, eptr);

  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}
#endif

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_string(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;
  struct jobject *jobject;
  jsize_t len;
  
  ptr++;

  nextchar:    

  if (AM_UNLIKELY(eptr == ptr)) goto fail;
  if (AM_UNLIKELY((unsigned char)(*ptr) < 32)) goto fail;
    
  if (AM_UNLIKELY(*ptr == '\\')) {
    ptr++;

    if (AM_UNLIKELY(eptr == ptr)) goto fail;  
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
  } else if (AM_UNLIKELY(*ptr == '"')) {
    ptr++;
    goto success;
  } else {
    ptr++;
    goto nextchar;
  }

goto fail;

 success:

  len = (ptr-1) - ((*optr)+1); 
  if (AM_UNLIKELY(len > AMJSON_MAXSTR)) goto fail; 
  
  jobject                  = jobject_allocate(jhandle, 1);
  jobject->blen            = len | (AMJSON_STRING << AMJSON_LENBITS);
  jobject->next            = AMJSON_INVALID;
  jobject->u.string.offset = ((*optr)+1) - jhandle->buf;

  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_integer(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;
  
  if (AM_UNLIKELY(eptr == ptr)) goto fail;
  if ((unsigned char)(*ptr - '1') < 9) {
    ptr++;
    
    for (;;) {
      if (AM_UNLIKELY(eptr == ptr)) break;
      if ((unsigned char)(*ptr - '0') >= 10) break;
      ptr++;
    }
    
    *optr = ptr;
    return;
  }

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_fraction(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;

  ptr++; /* consume '.' */

  if (AM_UNLIKELY(eptr == ptr)) goto fail;
  if ((unsigned char)(*ptr - '0') < 10) {
    ptr++;

    for (;;) {
      if (AM_UNLIKELY(eptr == ptr)) break;
      if ((unsigned char)(*ptr - '0') >= 10) break;
      ptr++;
    }

    *optr = ptr;
    return;
  }

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_exponent(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;

  ptr++; /* consume 'e' or 'E' */

  if (AM_UNLIKELY(eptr == ptr)) goto fail;  
  if (AM_UNLIKELY(*ptr == '+')) {
    ptr++;
    } else if (AM_UNLIKELY(*ptr == '-')) {
    ptr++;
  }

  if (AM_UNLIKELY(eptr == ptr)) goto fail;
  if ((unsigned char)(*ptr - '0') < 10) {
    ptr++;

    for (;;) {
      if (AM_UNLIKELY(eptr == ptr)) break;
      if ((unsigned char)(*ptr - '0') >= 10) break;
      ptr++;
    }

    *optr = ptr;  
    return;
  }

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_number(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  char * const eptr = jhandle->eptr;
  struct jobject *jobject;
  jsize_t len;

  if (AM_UNLIKELY(eptr == ptr)) goto fail;  
  if (AM_UNLIKELY(*ptr == '-')) {
    ptr++;
  }

  if (AM_UNLIKELY(eptr == ptr)) goto fail;  
  if (*ptr == '0') {
    ptr++;
    goto fraction;
  }

  amjson_integer(jhandle, &ptr);
  
 fraction:

  if ((eptr != ptr) && 
      (*ptr == '.')) amjson_fraction(jhandle, &ptr);

  if ((eptr != ptr) &&
      ((*ptr == 'e') || (*ptr == 'E'))) amjson_exponent(jhandle, &ptr);

  len = ptr - (*optr); 
  if (AM_UNLIKELY(len > AMJSON_MAXSTR)) goto fail; 
  
  jobject                  = jobject_allocate(jhandle, 1);
  jobject->blen            = len | (AMJSON_NUMBER << AMJSON_LENBITS);
  jobject->next            = AMJSON_INVALID;
  jobject->u.string.offset = (*optr) - jhandle->buf;

  *optr = ptr;
  return;

 fail:
  longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_true(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  struct jobject *jobject;
  
  if (!(((jhandle->eptr - ptr) >= 4) &&
	((ptr[1] == 'r') && 
	 (ptr[2] == 'u') && (ptr[3] == 'e')))) {

    longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
  }

  jobject                 = jobject_allocate(jhandle, 1);
  jobject->blen           = AMJSON_OBJECT << AMJSON_LENBITS; 
  jobject->u.object.child = AMJSON_TRUE;
  jobject->next           = AMJSON_INVALID;
  
  *optr = ptr + 4;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_false(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  struct jobject *jobject;

  if (!(((jhandle->eptr - ptr) >= 5) &&
	((ptr[1] == 'a') && (ptr[2] == 'l') && 
	 (ptr[3] == 's') && (ptr[4] == 'e')))) {

    longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
  }

  jobject                 = jobject_allocate(jhandle, 1);
  jobject->blen           = AMJSON_OBJECT << AMJSON_LENBITS; 
  jobject->u.object.child = AMJSON_FALSE;
  jobject->next           = AMJSON_INVALID;
  
  *optr = ptr + 5;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static void amjson_null(struct jhandle * const jhandle, char **optr) {

  char *ptr = *optr;
  struct jobject *jobject;

  if (!(((jhandle->eptr - ptr) >= 4) &&
	((ptr[1] == 'u') &&
	 (ptr[2] == 'l') && (ptr[3] == 'l')))) {

    longjmp(jhandle->setjmp_ctx, 2); /* jump back to amjson_decode() with EINVAL */
  }

  jobject                 = jobject_allocate(jhandle, 1);
  jobject->blen           = AMJSON_OBJECT << AMJSON_LENBITS; 
  jobject->u.object.child = AMJSON_NULL;
  jobject->next           = AMJSON_INVALID;

  *optr = ptr + 4;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
