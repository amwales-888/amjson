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

/* This project attempts to provide a JSON parser that is able to
 * successfully parse correct JSON formatted data as described in the 
 * JSON grammar available at https://www.json.org/
 *
 * As per RFC 8259 section (9) we set a maximum depth when parsing 
 * elements, this is configurable and a default compile time constant 
 * has been provided.
 * 
 * This library is usable with just 2 files being required json.h and 
 * json.c ALL other files are optional.
 */

#ifndef _JSON_H_
#define _JSON_H_

/* -------------------------------------------------------------------- */

#include <setjmp.h>
#include <sys/types.h>

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define JSON_MAXDEPTH 1024         /* Set the maximum depth we will allow
				    * lists and disctions to descend. */

/* #define BIGJSON */              /* Set string offset to use 'unsigned long',
				    * on 64 bit platforms this will allow us to 
				    * index string at offset >4GB Ths downside
				    * of this is that every jobject will now
				    * consume 16bytes instead of 12bytes 
				    */

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/* All next/child values are offsets not pointers.
 */
struct jobject {

#define JSON_OBJECT 1 
#define JSON_ARRAY  2 
#define JSON_STRING 3 
#define JSON_NUMBER 4 
#define JSON_TRUE   5 
#define JSON_FALSE  6 
#define JSON_NULL   7 
  unsigned int type:3;            /* One of JSON_OBJECT, JSON_ARRAY... */
  unsigned int len:29;            /* Count of ALL children OR length of string
				   * MAX:2^29-1 (536870911) */
  union {
    struct {
      unsigned int child;         /* Index of first child */
    } object;

    struct {

#ifdef BIGJSON
#define joff_t unsigned long
#else      
#define joff_t unsigned int
#endif
      joff_t offset;              /* First character Offset from start of JSON buffer */ 
    } string;

  } u;

#define JSON_INVALID ((unsigned int)-1)
  unsigned int next;              /* Index of chained jobject, JSON_INVALID 
				   * used for end of list */
};

struct jhandle {

  unsigned int userbuffer:1;      /* Did user supply the buffer? */
  unsigned int useljmp:1;         /* We want to longjmp on allocation failure */
  unsigned int hasdecoded:1;      /* json_decode has run, prevent us from modfying 
				   * the jobject pool */
  int spare:29;

  void (*onfree)
  (struct jhandle *jhandle);      /* Function to call on freeing */
  
  char         *buf;              /* Unparsed json data */
  size_t       len;               /* Length of json data */  
  jmp_buf      setjmp_ctx;        /* Allows us to return from allocation failure 
				   * from deeply nested calls */
  
  struct jobject *jobject;        /* Preallocated jobject pool */
  unsigned int   count;           /* Size of jobject pool */
  unsigned int   used;            /* Jobjects in use */
  unsigned int   root;            /* Index of our root object */

  int            depth;
  int            max_depth;       /* RFC 8259 section 9 allows us to set a max depth for
				   * list and object traversal */
};

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define JOBJECT_LAST(jhandle)          (&(jhandle)->jobject[(jhandle)->used-1])
#define JOBJECT_OFFSET(jhandle, o)     ((((char *)(o)) - ((char *)&(jhandle)->jobject[0]))) / sizeof(struct jobject)
#define JOBJECT_AT(jhandle, offset)    (&(jhandle)->jobject[(offset)])

/* json.c ------------------------------------------------------------- */

int json_alloc(struct jhandle *jhandle, struct jobject *ptr, unsigned int count);
void json_free(struct jhandle *jhandle);
int json_decode(struct jhandle *jhandle, char *buf, size_t len);

struct jobject *jobject_allocate(struct jhandle *jhandle, int count);

/* json_dump.c -------------------------------------------------------- */

void json_dump(struct jhandle *jhandle, struct jobject *jobject, int pretty);

/* json_file.c -------------------------------------------------------- */

int json_file_decode(struct jhandle *jhandle, char *pathname);

/* json_query.c ------------------------------------------------------- */

struct jobject *json_query(struct jhandle *jhandle, struct jobject *jobject, char *ptr);

/* json_util.c -------------------------------------------------------- */

#define JOBJECT_ROOT(jhandle)          (JOBJECT_AT((jhandle), (jhandle)->root))
#define JOBJECT_NEXT(jhandle,o)        (((o)->next == JSON_INVALID)?(void *)0:(JOBJECT_AT((jhandle), (o)->next)))
#define JOBJECT_TYPE(o)                ((o)->type)
#define JOBJECT_STRING_LEN(o)          ((o)->len)
#define JOBJECT_STRING_PTR(jhandle, o) (((jhandle)->buf)?(&((jhandle)->buf[(o)->u.string.offset])):((char *)(&(jhandle)->jobject[(o)->u.string.offset])))
#define ARRAY_COUNT(o)                 ((o)->len)
#define ARRAY_FIRST(jhandle, o)        (((o)->len == 0)?(void *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define ARRAY_NEXT(jhandle, o)         (((o)->next == JSON_INVALID)?(void *)0:(JOBJECT_AT((jhandle), (o)->next)))
#define OBJECT_COUNT(o)                ((o)->len)
#define OBJECT_FIRST_KEY(jhandle, o)   (((o)->len == 0)?(void *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define OBJECT_NEXT_KEY(jhandle, o)    (((o)->next == JSON_INVALID)?(void *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), (o)->next)->next))
#define OBJECT_FIRST_VALUE(jhandle, o) (((o)->len == 0)?(void *)0:JOBJECT_AT((jhandle), JOBJECT_AT((jhandle), (o)->u.object.child)->next))
#define OBJECT_NEXT_VALUE(jhandle, o)  (((o)->next == JSON_INVALID)?(void *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), (o)->next)->next)))
#define JOBJECT_STRDUP(o)              ((JOBJECT_TYPE((o)) != JSON_STRING)?((void *)0):strndup(JOBJECT_STRING_PTR((o)),JOBJECT_STRING_LEN((o))))

struct jobject *array_index(struct jhandle *jhandle, struct jobject *array, unsigned int index);
struct jobject *object_find(struct jhandle *jhandle, struct jobject *object, char *key, unsigned int len);

/* json_mod.c -------------------------------------------------------- */

struct jobject *json_string_new(struct jhandle *jhandle, char *ptr, int len);
struct jobject *json_object_add(struct jhandle *jhandle,
				struct jobject *object,
				struct jobject *string,
				struct jobject *value);
struct jobject *json_object_new(struct jhandle *jhandle, ...);
struct jobject *json_array_new(struct jhandle *jhandle, ...);
struct jobject *json_array_add(struct jhandle *jhandle,
			       struct jobject *array,
			       struct jobject *value);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#endif
