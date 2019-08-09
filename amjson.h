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

/* This project provides a JSON parser that is able to
 * successfully parse correct JSON formatted data as described in the 
 * JSON grammar available at https://www.json.org/
 *
 * As per RFC 8259 section (9) we set a maximum depth when parsing 
 * elements, this is configurable and a default compile time constant 
 * has been provided.
 * 
 * This library is usable with just 2 files being required amjson.h and 
 * amjson.c ALL other files are optional.
 * See the extras directory for additional features, including dumping,
 * pretty printing, object queries and JSON DOM creation.
 */

#ifndef _AMJSON_H_
#define _AMJSON_H_

/* -------------------------------------------------------------------- */

#include <setjmp.h>
#include <sys/types.h>
#include <stdint.h>

/* -------------------------------------------------------------------- */

#define AMJSON_MAXDEPTH 64        /* Set the maximum depth we will allow
				   * lists and disctions to descend. Since we
				   * use a recursive descent parser this is 
				   * also affects the maximum stack depth used.
				   * You may lower this number but it will affect 
				   * the maximum nesting of your JSON objects */

/* #define AMBIGJSON */           /* Set string offset to use 'unsigned long',
				   * on 64 bit Linux platforms, this will allow 
				   * us to index strings at offset >4GB The 
				   * downside of this is that every jobject will 
				   * now consume 16bytes instead of 12bytes on a 
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
   |{ "key": 1234 }|           call to amjson_decode() The JSON buffer is never 
   +---------------+           modified but MUST remain accessible to the 
                               system once parsed. This is because the system 
                               generates references into the JSON Buffer as 
                               it is parsed. After The call to amjson_free() 
                               the buffer is no longer required.

  Type           LP64   LLP64( *Windows )
  char              8       8
  short            16      16
  int              32      32
  long             64      32
  long long        64      64
  pointer          64      64

  Linux and MacOS X LP64, Windows LLP64 

                          AMJSON_8 AMJSON_16 AMJSON_32     
  MAX Array Entries       31       8191      536870911
  MAX Object Entries*     15       4095      268435455   *( Key/Value Pairs )
  MAX String Length       15       4095      268435455
  MAX Number Length       15       4095      268435455
  MAX Jobect Pool Size    255      65535     4294967295
  Max JSON Buffer Length  255      65535     4294967295
  Size of Jobject         3 Bytes  6 Bytes   12 Bytes

 * -------------------------------------------------------------------- */

#define AMJSON_32
/* #define AMJSON_16 */
/* #define AMJSON_8  */

#ifdef AMJSON_32
typedef uint32_t jsize_t;
typedef uint32_t joff_t;
#define JSIZE_MAX       UINT32_MAX
#define JOFF_MAX        UINT32_MAX
#define AMJSON_TYPEBITS 3
#define AMJSON_LENBITS  29

#ifdef AMBIGJSON
typedef uint64_t boff_t;     /* Offset of character into JSON buffer */
#define BOFF_MAX        UINT64_MAX 
#else
typedef uint32_t boff_t;
#define BOFF_MAX        UINT32_MAX
#endif
#endif

#ifdef AMJSON_16
typedef uint16_t jsize_t;
typedef uint16_t joff_t;
#define JSIZE_MAX       UINT16_MAX
#define JOFF_MAX        UINT16_MAX
#define AMJSON_TYPEBITS 3
#define AMJSON_LENBITS  13

#ifdef AMBIGJSON
typedef uint32_t boff_t;
#define BOFF_MAX        UINT32_MAX
#else
typedef uint16_t boff_t;
#define BOFF_MAX        UINT16_MAX
#endif
#endif

#ifdef AMJSON_8
typedef uint8_t jsize_t;
typedef uint8_t joff_t;
#define JSIZE_MAX       UINT8_MAX
#define JOFF_MAX        UINT8_MAX

#define AMJSON_TYPEBITS 3
#define AMJSON_LENBITS  5

#ifdef AMBIGJSON
typedef uint16_t boff_t;
#define BOFF_MAX        UINT16_MAX
#else
typedef uint8_t boff_t;
#define BOFF_MAX        UINT8_MAX
#endif
#endif

#define AMJSON_LENMASK    ((JSIZE_MAX) >> (AMJSON_TYPEBITS))
#define AMJSON_TYPEMASK   ((JSIZE_MAX) << (AMJSON_LENBITS))

/* String/Numbers are found in either the JSON buffer (STRJSONBUF) 
 * or the Jobject Pool (STRJOBJECTPOOL)
 */
#define AMJSON_STRLENMASK ((JSIZE_MAX) >> (AMJSON_TYPEBITS+1))
#define AMJSON_MAXSTR     AMJSON_STRLENMASK
#define STRJSONBUF        0
#define STRJOBJECTPOOL    1
#define AMJSON_STRBUFMASK (1 << (AMJSON_LENBITS-1))

/* -------------------------------------------------------------------- */

struct jobject {

#define AMJSON_UNDEFINED 0 
#define AMJSON_OBJECT    1
#define AMJSON_ARRAY     2 
#define AMJSON_STRING    3 
#define AMJSON_NUMBER    4 
#define AMJSON_TRUE      5 
#define AMJSON_FALSE     6 
#define AMJSON_NULL      7 

#define AMJSON_INVALID   0        /* Next offset use as value indicating 
                                   * end of list */

  jsize_t blen;                   /* type:len packed JSON_TYPEBITS 
                                   * and AMJSON_LENBITS */
  union {
    struct {
      joff_t  child;              /* Index of first child */
    } object;

    struct {
      boff_t  offset;             /* First character Offset from start of 
                                   * AMJSON buffer */ 
    } string;
  } u;

  joff_t next;                    /* next offset into jobject pool */

} __attribute__((packed));

struct jhandle {

  unsigned int   userbuffer:1;    /* Did user supply the buffer? */
  unsigned int   useljmp:1;       /* We want to longjmp on allocation failure */

  char           *buf;            /* Unparsed json data, the JSON buffer */
  char           *eptr;           /* Pointer to character after the end of 
                                   * the JSON buffer */
  size_t         len;             /* Length of json data */  
  jmp_buf        setjmp_ctx;      /* Allows us to return from allocation failure 
				   * from deeply nested calls */
  
  struct jobject *jobject;        /* Preallocated jobject pool */
  joff_t         count;           /* Size of jobject pool */
  joff_t         used;            /* Jobjects in use */
  joff_t         root;            /* Index of our root object */

  int            depth;
  int            max_depth;       /* RFC 8259 section 9 allows us to set a 
                                   * max depth for list and object traversal */
};

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define JOBJECT_LAST(jhandle)          (&(jhandle)->jobject[(jhandle)->used-1])
#define JOBJECT_OFFSET(jhandle, o)     ((((char *)(o)) - ((char *)&(jhandle)->jobject[0]))) / sizeof(struct jobject)
#define JOBJECT_AT(jhandle, offset)    (&(jhandle)->jobject[(offset)])

/* -------------------------------------------------------------------- */

#define JOBJECT_ROOT(jhandle)          (JOBJECT_AT((jhandle), (jhandle)->root))
#define JOBJECT_NEXT(jhandle,o)        ((((o)->next) == AMJSON_INVALID)?(struct jobject *)0:(JOBJECT_AT((jhandle), ((o)->next))))
#define JOBJECT_TYPE(o)                ((o)->blen >> AMJSON_LENBITS)

#define JOBJECT_STRING_LEN(o)          ((o)->blen & AMJSON_STRLENMASK)
#define JOBJECT_STRING_PTR(jhandle, o) (((o)->blen & AMJSON_STRBUFMASK)?((char *)(&(jhandle)->jobject[(o)->u.string.offset])):(&((jhandle)->buf[(o)->u.string.offset])))

#define ARRAY_COUNT(o)                 ((o)->blen & AMJSON_LENMASK)
#define ARRAY_FIRST(jhandle, o)        ((((o)->blen & AMJSON_LENMASK) == 0)?(struct jobject *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define ARRAY_NEXT(jhandle, o)         ((((o)->next) == AMJSON_INVALID)?(struct jobject *)0:(JOBJECT_AT((jhandle), ((o)->next))))
#define OBJECT_COUNT(o)                ((o)->blen & AMJSON_LENMASK)
#define OBJECT_FIRST_KEY(jhandle, o)   ((((o)->blen & AMJSON_LENMASK) == 0)?(struct jobject *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define OBJECT_NEXT_KEY(jhandle, o)    ((((o)->next) == AMJSON_INVALID)?(struct jobject *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), ((o)->next))->next))
#define OBJECT_FIRST_VALUE(jhandle, o) ((((o)->blen & AMJSON_LENMASK) == 0)?(struct jobject *)0:JOBJECT_AT((jhandle), JOBJECT_AT((jhandle), (o)->u.object.child)->next))
#define OBJECT_NEXT_VALUE(jhandle, o)  ((((o)->next) == AMJSON_INVALID)?(struct jobject *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), ((o)->next))->next))
#define JOBJECT_STRDUP(o)              ((JOBJECT_TYPE((o)) != AMJSON_STRING)?((struct jobject *)0):strndup(JOBJECT_STRING_PTR((o)),JOBJECT_STRING_LEN((o))))

#define JOBJECT_P                      6
#define JOBJECT_COUNT_GUESS(size)      ((((size) / JOBJECT_P)<JOBJECT_P)?JOBJECT_P:((size) / JOBJECT_P))

/* -------------------------------------------------------------------- */

#ifndef __cplusplus
#define REGISTER register
#else
#define REGISTER
#endif

#ifdef __cplusplus
extern "C" {  
#endif

/* -------------------------------------------------------------------- */

/* Summary: Create a new amjson context.
 * jhandle: This is a pointer to an uninitialised jhandle structure.
 *          On success the context will be initialised, a subsequent call
 *          to amjson_free() must be made to release any resources held.
 * ptr:     This is a pointer to an allocated jobject pool. 
 *          If this is (struct jobject *)0 the jobject pool will be 
 *          allocated by this call.
 * count:   This is the count of struct jobject object in the pool or
 *          if the jobject pool is to be allocated, it's initial count.
 *
 * Return 0 on success and !0 on failure.
 */
int amjson_alloc(struct jhandle *jhandle, struct jobject *ptr, joff_t count);

/* Summary: Decode a buffer holding JSON data using the amjson context 
 *          allocated by the call to amjson_alloc()
 * jhandle: This is a pointer to an initialised jhandle structure.
 * buf:     This is a pointer to a buffer holding JSON data to be parsed.
 *          The contents of this buffer MUST not be freed or changed while
 *          the amjson context exists.
 * len:     This is the length of the JSON buffer in bytes.
 *
 * Return 0 on success and !0 on failure.
 * The value of errno will be set to EINVAL if an error ocurred parsing
 * the JSON buffer. ENOMEM indicates a problem allocating an object from
 * the jobject pool.
 */
int amjson_decode(struct jhandle *jhandle, char *buf, jsize_t len);

/* Summary: Release any resources held by an initialised amjson context.
 * jhandle: This is a pointer to an initialised jhandle structure.
 */
void amjson_free(struct jhandle *jhandle);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif
#endif
