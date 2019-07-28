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
 * See the extras directory for additional features, including dumping,
 * pretty printing, object queries and JSON DOM creation.
 */

#ifndef _JSON_H_
#define _JSON_H_

/* -------------------------------------------------------------------- */

#include <setjmp.h>
#include <sys/types.h>

/* -------------------------------------------------------------------- *
 * WARNING!!! Assumptions have been made about the current platform.
 *            That is the minimum size of an int being 32 bits.
 *            This is intended to be fixed in a future release for 
 *            targeting small memory devices such as microcontrollers.
 * -------------------------------------------------------------------- */

#define JSON_MAXDEPTH 1024         /* Set the maximum depth we will allow
				    * lists and disctions to descend. Since we
				    * use a recursive descent parser this is 
				    * also affects the maximum stack depth used.
				    * You may lower this number but it will affect 
				    * the maximum nesting of your JSON objects */

/* #define BIGJSON */              /* Set string offset to use 'unsigned long',
				    * on 64 bit platforms this will allow us to 
				    * index string at offset >4GB Ths downside
				    * of this is that every jobject will now
				    * consume 16bytes instead of 12bytes */

/* -------------------------------------------------------------------- *

   Jobject Pool                This is a collection equal sized structures.
   +----+----+----+----+----+  They are chained via a next (offset).
   |    |    |    |    |    |  The Jobject pool is either unmanaged, that is 
   +---|+-^-|+-^--+----+----+  allocated by the user OR maneged, that is 
       |  | |  |               automaticaly managed by the system.
       +--+ +--+


   JSON Buffer                 This is a collection of bytes that contains
   +---------------+           the JSON data that is to be parsed.
   |{ "key": 1234 }|           The JSON buffer is never modified but MUST
   +---------------+           remain accessible to the system once parsed.
                               This is because the system generates references
                               into the JSON Buffer as it is parsed. After
                               The call to json_free() the buffer is no longer 
                               required.

 * -------------------------------------------------------------------- */

#ifdef BIGJSON
typedef unsigned long boff_t;     /* boff_t is used as the offset (pointer) into the JSON Buffer. */
#else      
typedef unsigned int boff_t;
#endif

typedef unsigned int poff_t;      /* poff_t is used as the offset (pointer) into the Jobject Pool. */
typedef unsigned int jsize_t;     /* jsize_t is used as the length of a String/Number in bytes 
                                   * OR the number of children in an Object/Array. */

struct jobject {

#define JSON_UNDEFINED 0 
#define JSON_OBJECT    1
#define JSON_ARRAY     2 
#define JSON_STRING    3 
#define JSON_NUMBER    4 
#define JSON_TRUE      5 
#define JSON_FALSE     6 
#define JSON_NULL      7 
  unsigned int type:3;            /* One of JSON_OBJECT, JSON_ARRAY... */

#define JSON_MAXLEN (536870911UL)
  jsize_t len:29;                 /* Count of ALL children OR length of string
				   * MAX:2^29-1 (536870911) */
  union {
    struct {
      poff_t child;               /* Index of first child */
    } object;

    struct {
      boff_t offset;              /* First character Offset from start of JSON buffer */ 
    } string;

  } u;

#define JSON_INVALID ((poff_t)-1)
  poff_t next;                    /* Index of chained jobject, JSON_INVALID 
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
  jsize_t        count;           /* Size of jobject pool */
  jsize_t        used;            /* Jobjects in use */
  poff_t         root;            /* Index of our root object */

  int            depth;
  int            max_depth;       /* RFC 8259 section 9 allows us to set a max depth for
				   * list and object traversal */
};

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define JOBJECT_LAST(jhandle)          (&(jhandle)->jobject[(jhandle)->used-1])
#define JOBJECT_OFFSET(jhandle, o)     ((((char *)(o)) - ((char *)&(jhandle)->jobject[0]))) / sizeof(struct jobject)
#define JOBJECT_AT(jhandle, offset)    (&(jhandle)->jobject[(offset)])

/* -------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {  
#endif

/* -------------------------------------------------------------------- */

int json_alloc(struct jhandle *jhandle, struct jobject *ptr, unsigned int count);
void json_free(struct jhandle *jhandle);
int json_decode(struct jhandle *jhandle, char *buf, size_t len);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif
