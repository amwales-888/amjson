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

#ifndef _JSON_UTIL_H_
#define _JSON_UTIL_H_

#include "json.h"

/* -------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {  
#endif

/* -------------------------------------------------------------------- */

struct jobject *array_index(struct jhandle *jhandle, struct jobject *array, unsigned int index);
struct jobject *object_find(struct jhandle *jhandle, struct jobject *object, char *key, unsigned int len);

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif



