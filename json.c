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

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static char *json_whitespace(char *ptr, char *eptr);
static char *json_element(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_object(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_array(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_value(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_hexdigit(char *ptr, char *eptr);
static char *json_string(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_digit(char *ptr, char *eptr);
static char *json_integer(char *ptr, char *eptr);
static char *json_fraction(char *ptr, char *eptr);
static char *json_exponent(char *ptr, char *eptr);
static char *json_number(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_true(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_false(struct jhandle_s *jhandle, char *ptr, char *eptr);
static char *json_null(struct jhandle_s *jhandle, char *ptr, char *eptr);

static struct jobject_s *jobject_allocate(struct jhandle_s *jhandle);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int json_alloc(struct jhandle_s *jhandle, struct jobject_s *ptr,
	       int count) {

  jhandle->count  = count;
  jhandle->used   = 0;
  jhandle->onfree = (void *)0;

  if (ptr) {
    jhandle->userbuffer = (unsigned int)1;
    jhandle->jobject    = ptr;
    
  } else {
    jhandle->userbuffer = 0;
    jhandle->jobject    = (struct jobject_s *)calloc(jhandle->count,
						     sizeof(struct jobject_s));

    if (jhandle->jobject == (void *)0) {
      return -1;
    }
  } 
  
  jhandle->root= -1;
  return 0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void json_free(struct jhandle_s *jhandle) {

  if (jhandle->onfree) {
    jhandle->onfree(jhandle);
  }

  if (!jhandle->userbuffer) {
    free(jhandle->jobject);
  }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int json_decode(struct jhandle_s *jhandle, char *buf, int len) {

  struct jobject_s *object;

  jhandle->buf = buf;
  jhandle->len = len;
  
  jhandle->max_depth = JSON_MAXDEPTH;
  jhandle->depth     = 0;

  if (json_element(jhandle, buf, &buf[len]) == buf) {
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
static struct jobject_s *jobject_allocate(struct jhandle_s *jhandle) {

  if (jhandle->used < jhandle->count-1) {
    return &jhandle->jobject[jhandle->used++];
  }

  if (!jhandle->userbuffer) {
    
    void *ptr;
      
    jhandle->count = jhandle->count * 2;
    ptr = realloc(jhandle->jobject, (jhandle->count * sizeof(struct jobject_s)));	  
    if (ptr) {
      jhandle->jobject = ptr;
      return jobject_allocate(jhandle);
    }
  }

  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/* 
 whitespace is one of:

    '\t'  9
    '\n' 10
    '\r' 13
    ' '  32
*/
static unsigned char whitespace[] = {

  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *json_whitespace(char *ptr, char *eptr) {

  while ((ptr < eptr) &&
	 (whitespace[(unsigned char)(*ptr)])) {
    ptr++;
  }

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_element(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  char *optr = ptr;
  char *nptr;

  if (eptr == ptr) goto fail;
  ptr = json_whitespace(ptr, eptr);

  nptr = json_value(jhandle, ptr, eptr);
  if (nptr == ptr) {      
    goto fail;
  }
  ptr = nptr;
  
  ptr = json_whitespace(ptr, eptr);
  if (eptr == ptr) goto success;
  
  goto fail;

 success:
  return ptr;
 fail:
  return optr;

}


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_object(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  char *optr  = ptr;
  char *nptr;
  char *comma = (void *)0;

  struct jobject_s *string;
  struct jobject_s *value;
  struct jobject_s *object;
  struct jobject_s *jobject;

  int last  = -1;
  int first = -1;
  int count = 0;

  jhandle->depth++;
  
  if (eptr == ptr) goto fail;
  if ((*ptr == '{') &&
      (jhandle->depth < jhandle->max_depth)) {
    ptr++;

    ptr = json_whitespace(ptr, eptr);

    if (eptr == ptr) goto fail;  
    if (*ptr == '}') {
      ptr++;
      goto success;
    } 

  nextobject:

    ptr = json_whitespace(ptr, eptr);

    nptr = json_string(jhandle, ptr, eptr);
    if (nptr == ptr) {
      if (comma) ptr = comma;
      
      goto fail;
    }
    ptr = nptr;

    /* Add string to list */
    count++;
    string = JOBJECT_LAST(jhandle);
    if (last == -1) {
      first = JOBJECT_OFFSET(jhandle, string);
      last  = first;
    } else {
      jobject = JOBJECT_AT(jhandle, last);
      jobject->next = JOBJECT_OFFSET(jhandle, string);
      last = jobject->next;
    }
    /* String added */
        
    ptr = json_whitespace(ptr, eptr);

    if (eptr == ptr) goto fail;  
    if (*ptr == ':') {
      ptr++;
    } else {
      goto fail;
    }

    nptr = json_value(jhandle, ptr, eptr);
    if (nptr == ptr) {      
      goto fail;
    }
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
  
  if ((object = jobject_allocate(jhandle))) {
    
    object->type           = JSON_OBJECT;
    object->u.object.child = first;
    object->u.object.count = count;
    object->next           = -1;

    jhandle->depth--;
    return ptr;
  }
  
 fail:
  jhandle->depth--;
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_array(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  char *optr  = ptr;
  char *nptr;
  char *comma = (void *)0;

  struct jobject_s *value;
  struct jobject_s *array;
  struct jobject_s *jobject;

  int last  = -1;
  int first = -1;
  int count = 0;

  jhandle->depth++;

  if (eptr == ptr) goto fail;  
  if ((*ptr == '[') &&
      (jhandle->depth < jhandle->max_depth)) {
    ptr++;
    
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
    value = JOBJECT_LAST(jhandle);
    if (last == -1) {
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
 
  if ((array = jobject_allocate(jhandle))) {

    array->type          = JSON_ARRAY;
    array->u.object.child = first;
    array->u.object.count = count;
    array->next          = -1;  

    jhandle->depth--;
    return ptr;
  }

 fail:
  jhandle->depth--;
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_value(struct jhandle_s *jhandle, char *ptr, char *eptr) {
  
  char *optr = ptr;
  char *nptr;

  ptr = json_whitespace(ptr, eptr);

  nptr = json_string(jhandle, ptr, eptr);
  if (nptr == ptr) {
    nptr = json_number(jhandle, ptr, eptr);
    if (nptr == ptr) {
      nptr = json_object(jhandle, ptr, eptr);
      if (nptr == ptr) {
	nptr = json_array(jhandle, ptr, eptr);
	if (nptr == ptr) {
	  nptr = json_true(jhandle, ptr, eptr);
	  if (nptr == ptr) {
	    nptr = json_false(jhandle, ptr, eptr);
	    if (nptr == ptr) {
	      nptr = json_null(jhandle, ptr, eptr);
	      if (nptr == ptr) {
		return optr;
	      }
	    }
	  }
	}
      }
    }
  }
  ptr = nptr;

  ptr = json_whitespace(ptr, eptr);
  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/* 
 hexdigit is one of:

    '0'-'9'  48-57
    'A'-'Z'  65-90  
    'a'-'z'  97-122
*/
static unsigned char hexdigit[] = {

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *json_hexdigit(char *ptr, char *eptr) {

  if ((eptr != ptr) &&
      (hexdigit[(unsigned char)(*ptr)])) {
    ptr++;
  }

  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_string(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  struct jobject_s *jobject;
  char *optr = ptr;
  char *nptr;

  if (eptr == ptr) goto fail;  
  if (*ptr == '"') {
    ptr++;

  nextchar:    
    
    if ((eptr == ptr) || ((unsigned char)(*ptr) < 32)) {
      goto fail;
    }
    
    if (*ptr == '\\') {
      ptr++;

      if (eptr == ptr) goto fail;  
      if ((*ptr == '"') || (*ptr == '\\') || (*ptr == '/') ||
	  (*ptr == 'b') || (*ptr == 'f') || (*ptr == 'n') ||
	  (*ptr == 'r') || (*ptr == 't')) {
	ptr++;
	goto nextchar;

      } else if (*ptr == 'u') {
	ptr++;
	
	nptr = json_hexdigit(ptr, eptr);
	if (nptr == ptr) {      
	  goto fail;
	}
	ptr = nptr;

	nptr = json_hexdigit(ptr, eptr);
	if (nptr == ptr) {      
	  goto fail;
	}
	ptr = nptr;

	nptr = json_hexdigit(ptr, eptr);
	if (nptr == ptr) {      
	  goto fail;
	}
	ptr = nptr;

	nptr = json_hexdigit(ptr, eptr);
	if (nptr == ptr) {      
	  goto fail;
	}
	ptr = nptr;

	goto nextchar;

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
  
  if ((jobject = jobject_allocate(jhandle))) {

    jobject->type         = JSON_STRING;
    jobject->u.string.ptr = (optr+1);
    jobject->u.string.len = (ptr-1) - (optr+1);
    jobject->next         = -1;
    return ptr;
  }

 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/* 
 digit is one of: 

    '0'-'9'  48-57
*/
static unsigned char digit[] = {

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *json_digit(char *ptr, char *eptr) {

  if ((eptr != ptr) &&
      (digit[(unsigned char)(*ptr)])) {
    ptr++;
  }
    
  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/* 
 realdigit is one of: 

    '1'-'9'  49-57
*/
static unsigned char real[] = {

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *json_integer(char *ptr, char *eptr) {

  char *optr = ptr;
  char *nptr;

  if ((eptr != ptr) &&
      (real[(unsigned char)(*ptr)])) {

    ptr++;
    for (;;) {
      nptr = json_digit(ptr, eptr);
      if (nptr == ptr) {
	return ptr;
      }
      ptr = nptr;
    }
  }

  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_fraction(char *ptr, char *eptr) {

  char *optr = ptr;
  char *nptr;

  if (eptr == ptr) goto fail;  
  if (*ptr == '.') {
    ptr++;

    nptr = json_digit(ptr, eptr);
    if (nptr == ptr) {
      goto fail;
    }
    ptr = nptr;

    for (;;) {
      nptr = json_digit(ptr, eptr);
      if (nptr == ptr) {
	goto success;
      }
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
static char *json_exponent(char *ptr, char *eptr) {

  char *optr = ptr;
  char *nptr;

  if (eptr == ptr) goto fail;  
  if ((*ptr == 'E') || (*ptr == 'e'))  {
    ptr++;

    if (eptr == ptr) goto fail;  
    if (*ptr == '+') {
      ptr++;
    } else if (*ptr == '-') {
      ptr++;
    }

    nptr = json_digit(ptr, eptr);
    if (nptr == ptr) {
      goto fail;
    }
    ptr = nptr;

    for (;;) {
      nptr = json_digit(ptr, eptr);
      if (nptr == ptr) {
	goto success;
      }
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
static char *json_number(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  struct jobject_s *jobject;
  char *optr = ptr;
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
  ptr = json_fraction(ptr, eptr);

  ptr = json_exponent(ptr, eptr);
  
  if ((jobject = jobject_allocate(jhandle))) {

    jobject->type           = JSON_NUMBER;
    jobject->u.string.ptr   = optr;
    jobject->u.string.len   = ptr - optr;
    jobject->next           = -1;
    return ptr;
  }
  
 fail:
  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_true(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  char *optr = ptr;
  
  if (((eptr - ptr) >= 4) &&
      ((*ptr++ == 't') && (*ptr++ == 'r') && 
       (*ptr++ == 'u') && (*ptr++ == 'e'))) {

    struct jobject_s *jobject = jobject_allocate(jhandle);
    if (jobject) { 
      jobject->type = JSON_TRUE;
      jobject->next = -1;
      return ptr;
    }
  } 

  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_false(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  char *optr = ptr;

  if (((eptr - ptr) >= 5) &&
      ((*ptr++ == 'f') && (*ptr++ == 'a') &&
       (*ptr++ == 'l') && (*ptr++ == 's') &&
       (*ptr++ == 'e'))) {
    
    struct jobject_s *jobject = jobject_allocate(jhandle);
    if (jobject) {
      jobject->type = JSON_FALSE;
      jobject->next = -1;
      return ptr;
    }
  } 

  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *json_null(struct jhandle_s *jhandle, char *ptr, char *eptr) {

  char *optr = ptr;

  if (((eptr - ptr) >= 4) &&
      ((*ptr++ == 'n') && (*ptr++ == 'u') &&
       (*ptr++ == 'l') && (*ptr++ == 'l'))) {

    struct jobject_s *jobject = jobject_allocate(jhandle);
    if (jobject) {
      jobject->type = JSON_NULL;
      jobject->next = -1;
      return ptr;
    }
  } 

  return optr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
