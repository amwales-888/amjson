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
#include <stdint.h>

/* -------------------------------------------------------------------- */

#define JSON_MAXDEPTH 64          /* Set the maximum depth we will allow
				   * lists and disctions to descend. Since we
				   * use a recursive descent parser this is 
				   * also affects the maximum stack depth used.
				   * You may lower this number but it will affect 
				   * the maximum nesting of your JSON objects */

/* #define BIGJSON */             /* Set string offset to use 'unsigned long',
				   * on 64 bit platforms, this will allow us to 
				   * index string at offset >4GB The downside
				   * of this is that every jobject will now
				   * consume 16bytes instead of 12bytes on a 
				   * 64 bit platform */

/* -------------------------------------------------------------------- *

   Jobject Pool                This is a collection of equal sized structures.
   +----+----+----+----+----+  They are chained via a next (offset).
   |    |    |    |    |    |  The Jobject pool is either unmanaged, that is 
   +---|+-^-|+-^--+----+----+  allocated by the user OR maneged, that is 
       |  | |  |               automaticaly managed by the system. An 
       +--+ +--+               unmanaged pool will return ENOMEM in errno 
                               if it becomes full. A managed pool will be
			       reallocated, if there is no available memory
			       to reallocate ENOMEM will be returned in errno.

   JSON Buffer                 This is a collection of bytes that contains
   +---------------+           the JSON data that is to be parsed after the 
   |{ "key": 1234 }|           call to json_decode() The JSON buffer is never 
   +---------------+           modified but MUST remain accessible to the 
                               system once parsed. This is because the system 
                               generates references into the JSON Buffer as 
                               it is parsed. After The call to json_free() 
                               the buffer is no longer required.

 * -------------------------------------------------------------------- */


/* 
Type           ILP64   LP64   LLP64
char              8      8       8
short            16     16      16
int              64     32      32
long             64     64      32
long long        64     64      64
pointer          64     64      64

Linux and MacOS X LP64
Windows LLP64 
*/


/*          
                        JSON_8   JSON_16  JSON_32     
MAX Array Entries       31       8191     536870911
MAX Object Entries      15       4095     268435455
MAX String Length       31       8191     536870911
MAX Number Length       31       8191     536870911
MAX Jobect Pool Size    255      65535    4294967295
Max JSON Buffer Length  255      65535    4294967295
Size of Jobject         3 Bytes  6 Bytes  12 Bytes

*/


#define JSON_32


#ifdef JSON_8
typedef uint8_t jsize_t;
typedef uint8_t joff_t;
#define JSIZE_MAX UINT8_MAX
#define JOFF_MAX  UINT8_MAX

#define JSON_LENMASK  0x1F
#define JSON_TYPEBITS 3
#define JSON_LENBITS  5
#define JSON_TYPEMASK 0xE0

#ifdef BIGJSON
typedef uint16_t boff_t;
#define BOFF_MAX  UINT16_MAX
#else
typedef uint8_t boff_t;
#define BOFF_MAX  UINT8_MAX
#endif
#endif



#ifdef JSON_16
typedef uint16_t jsize_t;
typedef uint16_t joff_t;
#define JSIZE_MAX UINT16_MAX
#define JOFF_MAX  UINT16_MAX

#define JSON_LENMASK  0x1FFF
#define JSON_TYPEBITS 3
#define JSON_LENBITS  13
#define JSON_TYPEMASK 0xE000

#ifdef BIGJSON
typedef uint32_t boff_t;
#define BOFF_MAX  UINT32_MAX
#else
typedef uint16_t boff_t;
#define BOFF_MAX  UINT16_MAX
#endif
#endif

#ifdef JSON_32
typedef uint32_t jsize_t;
typedef uint32_t joff_t;
#define JSIZE_MAX UINT32_MAX
#define JOFF_MAX  UINT32_MAX

#define JSON_LENMASK  0x1FFFFFFF
#define JSON_TYPEBITS 3
#define JSON_LENBITS  29
#define JSON_TYPEMASK 0xE0000000

#ifdef BIGJSON
typedef uint64_t boff_t;     /* Offset of character into JSON buffer */
#define BOFF_MAX  UINT64_MAX
#else
typedef uint32_t boff_t;
#define BOFF_MAX  UINT32_MAX
#endif
#endif

struct jobject {

#define JSON_UNDEFINED 0 
#define JSON_OBJECT    1
#define JSON_ARRAY     2 
#define JSON_STRING    3 
#define JSON_NUMBER    4 
#define JSON_TRUE      5 
#define JSON_FALSE     6 
#define JSON_NULL      7 

#define JSON_INVALID   0     /* Next offset use as value indicating end of list */

  jsize_t blen;              /* type:len packed JSON_TYPEBITS and JSON_LENBITS*/

  union {
    struct {
      joff_t  child;         /* Index of first child */
    } object;

    struct {
      boff_t  offset;        /* First character Offset from start of JSON buffer */ 
    } string;

  } u;

  joff_t next;               /* next offset into jobject pool */

} __attribute__((packed));

struct jhandle {

  unsigned int userbuffer:1;      /* Did user supply the buffer? */
  unsigned int useljmp:1;         /* We want to longjmp on allocation failure */
  unsigned int hasdecoded:1;      /* json_decode has run, prevent us from modfying 
				   * the jobject pool */
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

/* -------------------------------------------------------------------- */

#define JOBJECT_ROOT(jhandle)          (JOBJECT_AT((jhandle), (jhandle)->root))
#define JOBJECT_NEXT(jhandle,o)        ((((o)->next) == JSON_INVALID)?(void *)0:(JOBJECT_AT((jhandle), ((o)->next))))
#define JOBJECT_TYPE(o)                ((o)->blen >> JSON_LENBITS)
#define JOBJECT_STRING_LEN(o)          ((o)->blen & JSON_LENMASK)
#define JOBJECT_STRING_PTR(jhandle, o) (((jhandle)->buf)?(&((jhandle)->buf[(o)->u.string.offset])):((char *)(&(jhandle)->jobject[(o)->u.string.offset])))
#define ARRAY_COUNT(o)                 ((o)->blen & JSON_LENMASK)
#define ARRAY_FIRST(jhandle, o)        ((((o)->blen & JSON_LENMASK) == 0)?(void *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define ARRAY_NEXT(jhandle, o)         ((((o)->next) == JSON_INVALID)?(void *)0:(JOBJECT_AT((jhandle), ((o)->next))))
#define OBJECT_COUNT(o)                ((o)->blen & JSON_LENMASK)
#define OBJECT_FIRST_KEY(jhandle, o)   ((((o)->blen & JSON_LENMASK) == 0)?(void *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define OBJECT_NEXT_KEY(jhandle, o)    ((((o)->next) == JSON_INVALID)?(void *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), ((o)->next))->next))
#define OBJECT_FIRST_VALUE(jhandle, o) ((((o)->blen & JSON_LENMASK) == 0)?(void *)0:JOBJECT_AT((jhandle), JOBJECT_AT((jhandle), (o)->u.object.child)->next))
#define OBJECT_NEXT_VALUE(jhandle, o)  ((((o)->next) == JSON_INVALID)?(void *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), ((o)->next))->next))
#define JOBJECT_STRDUP(o)              ((JOBJECT_TYPE((o)) != JSON_STRING)?((void *)0):strndup(JOBJECT_STRING_PTR((o)),JOBJECT_STRING_LEN((o))))

/* -------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {  
#endif

/* -------------------------------------------------------------------- */

int json_alloc(struct jhandle *jhandle, struct jobject *ptr, unsigned int count);
void json_free(struct jhandle *jhandle);
int json_decode(struct jhandle *jhandle, char *buf, jsize_t len);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif
