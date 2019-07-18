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

#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

static char *query_index(char *ptr);
static char *query_identifier(char *ptr);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *query_index(char *ptr) {

  char *optr = ptr;
  
  if (*ptr == '\0') goto fail;
  if (*ptr == '[') {
    ptr++;

    if (*ptr == '\0') goto fail;
    if (*ptr == '0') {
      ptr++;
    } else if ((*ptr >= '1') && (*ptr <= '9')) {
      ptr++;

    nextdigit:
      if ((*ptr >= '0') && (*ptr <= '9')) {
	ptr++;
	goto nextdigit;
      }

    } else {
      goto fail;
    }

    if (*ptr == '\0') goto fail;
    if (*ptr == ']') {
      ptr++;
      goto success;
    }    
  }
  goto fail;

 success:
  return ptr;
 fail:
  return optr;  
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
static char *query_identifier(char *ptr) {

  while ((*ptr != '\0') && (*ptr != '[') &&
	 (*ptr != ']') && (*ptr != '.')) {
    
    ptr++;
  }
      
  return ptr;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_queryXXX(struct jhandle *jhandle,
			     struct jobject *jobject, char *ptr) {  
  char *nptr;

  nptr = query_identifier(ptr);
  if (nptr == ptr) {
    goto fail;
  } else {
    jobject = object_find(jhandle, jobject, ptr, (nptr - ptr));
    if (!jobject) goto fail;    
  }
  ptr = nptr;

  if (*ptr == '\0') goto success;
  
  for (;;) {
  
    nptr = query_index(ptr);
    if (nptr == ptr) {    
      nptr = query_identifier(ptr);
      if (nptr == ptr) {
	goto fail;
      } else {

	if (JOBJECT_TYPE(jobject) != JSON_OBJECT) goto fail;       	
	jobject = object_find(jhandle, jobject, ptr, (nptr - ptr));
	if (!jobject) goto fail;    
      }
    } else {
      int index = 0;
      
      if (JOBJECT_TYPE(jobject) != JSON_ARRAY) goto fail;       	

      ptr++; /* '[' */     
      while (ptr != (nptr-1)) {
	index *= 10;
	index += *ptr - '0';
	ptr++;
      }

      ptr++; /* ']' */     
      if ((*ptr != '\0') &&
	  (*ptr != '.')) goto fail;  
      
      jobject = array_index(jhandle, jobject, index);
      if (!jobject) goto fail;    
    }
    ptr = nptr;
    
    if (*ptr == '\0') goto success;
  }

 success:
  return jobject;
 fail:
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
struct jobject *json_query(struct jhandle *jhandle,
			     struct jobject *jobject, char *ptr) {  

  if (*ptr == '\0') goto fail;
  
  for (;;) {
  
    char *nptr;

    nptr = query_index(ptr);
    if (nptr == ptr) {
      nptr = query_identifier(ptr);
      if (nptr == ptr) {
	goto fail;
      } else {

	if (JOBJECT_TYPE(jobject) != JSON_OBJECT) goto fail;       	
	jobject = object_find(jhandle, jobject, ptr, (nptr - ptr));
	if (!jobject) goto fail;    

	ptr = nptr;
	if (*ptr == '\0') goto success;
      }
    } else {
      int index = 0;
      
      if (JOBJECT_TYPE(jobject) != JSON_ARRAY) goto fail;       	

      ptr++; /* '[' */     
      while (ptr != (nptr-1)) {
	index *= 10;
	index += *ptr - '0';
	ptr++;
      }
      
      ptr++; /* ']' */     

      jobject = array_index(jhandle, jobject, index);
      if (!jobject) goto fail;    

      if (*ptr == '\0') goto success;

      if ((*ptr != '.') &&
	  (*ptr != '['))
	goto fail;

      if (*ptr == '.') ptr++;      
    }
    
  }

 success:
  return jobject;
 fail:
  return (void *)0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
