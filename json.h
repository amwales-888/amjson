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

#ifndef _JSON_H_
#define _JSON_H_

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define JSON_OBJECT 1
#define JSON_ARRAY  2
#define JSON_STRING 3
#define JSON_NUMBER 4
#define JSON_TRUE   5
#define JSON_FALSE  6
#define JSON_NULL   7

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

/* All next/child values are offsets not pointers.
 */
struct jobject_s {
  int type;                       /* One of JSON_OBJECT, JSON_ARRAY... */

  union {

    struct {
      int count;                  /* Count of ALL children */
      int child;                  /* Index of first child */
    } object;

    struct {
      char *ptr;                  /* First character in string NOT '\0' terminated */
      int  len;                   /* Length of the string */
    } string;

  } u;

  int next;                       /* Index of chained jobject, -1 used for end of list */
};

struct jhandle_s {

  int userbuffer:1;               /* Did user supply the buffer? */
  int spare:31;

  void (*onfree)
  (struct jhandle_s *jhandle);    /* Function to call on freeing */
  
  char *buf;                      /* Unparsed json data */
  int  len;                       /* Length of json data */
  
  struct jobject_s *jobject;      /* Preallocated jobject pool */
  int count;                      /* Size of jobject pool */
  int used;                       /* Jobjects in use */
  int root;                       /* Index of our root object */
};

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#define JOBJECT_LAST(jhandle)          (&(jhandle)->jobject[(jhandle)->used-1])
#define JOBJECT_OFFSET(jhandle, o)     ((((char *)(o)) - ((char *)&(jhandle)->jobject[0]))) / sizeof(struct jobject_s)
#define JOBJECT_AT(jhandle, offset)    (&(jhandle)->jobject[(offset)])

/* json.c ------------------------------------------------------------- */

int json_alloc(struct jhandle_s *jhandle, struct jobject_s *ptr, int count);
void json_free(struct jhandle_s *jhandle);
int json_decode(struct jhandle_s *jhandle, char *buf, int len);

/* json_dump.c -------------------------------------------------------- */

void json_dump(struct jhandle_s *jhandle, struct jobject_s *jobject);

/* json_file.c -------------------------------------------------------- */

int json_file_decode(struct jhandle_s *jhandle, char *pathname);

/* json_query.c ------------------------------------------------------- */

struct jobject_s *json_query(struct jhandle_s *jhandle, struct jobject_s *jobject, char *ptr);

/* json_util.c -------------------------------------------------------- */

#define JOBJECT_ROOT(jhandle)          (JOBJECT_AT((jhandle), (jhandle)->root))
#define JOBJECT_NEXT(jhandle,o)        (((o)->next == -1)?(void *)0:(JOBJECT_AT((jhandle), (o)->next)))
#define JOBJECT_TYPE(o)                ((o)->type)
#define JOBJECT_STRING_LEN(o)          ((o)->u.string.len)
#define JOBJECT_STRING_PTR(o)          ((o)->u.string.ptr)
#define ARRAY_COUNT(o)                 ((o)->u.object.count)
#define ARRAY_FIRST(jhandle, o)        (((o)->u.object.count == 0)?(void *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define ARRAY_NEXT(jhandle, o)         (((o)->next == -1)?(void *)0:(JOBJECT_AT((jhandle), (o)->next)))
#define OBJECT_COUNT(o)                ((o)->u.object.count)
#define OBJECT_FIRST_KEY(jhandle, o)   (((o)->u.object.count == 0)?(void *)0:(JOBJECT_AT((jhandle),(o)->u.object.child)))
#define OBJECT_NEXT_KEY(jhandle, o)    (((o)->next == -1)?(void *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), (o)->next)->next))
#define OBJECT_FIRST_VALUE(jhandle, o) (((o)->u.object.count == 0)?(void *)0:JOBJECT_AT((jhandle), JOBJECT_AT((jhandle), (o)->u.object.child)->next))
#define OBJECT_NEXT_VALUE(jhandle, o)  (((o)->next == -1)?(void *)0:JOBJECT_AT((jhandle),JOBJECT_AT((jhandle), (o)->next)->next)))
#define JOBJECT_STRDUP(o)              ((JOBJECT_TYPE((o)) != JSON_STRING)?((void *)0):strndup(JOBJECT_STRING_PTR((o)),JOBJECT_STRING_LEN((o))))

struct jobject_s *array_index(struct jhandle_s *jhandle, struct jobject_s *array, int index);
struct jobject_s *object_find(struct jhandle_s *jhandle, struct jobject_s *object, char *key, int len);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#endif
