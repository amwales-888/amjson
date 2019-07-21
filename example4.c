/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#include "json.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc __attribute__((unused)),
	 char **argv __attribute__((unused))) {

  struct jhandle jhandle;
  if (json_alloc(&jhandle, (void *)0, 2) == 0) {

    struct jobject *jobject;
    struct jobject *jobjectXXX;

#if 0
    jobject  = json_array_new(&jhandle,
			  json_string_new(&jhandle, "hello", strlen("hello")),
			  json_string_new(&jhandle, "1", strlen("1")),
			  json_object_new(&jhandle,
				      json_string_new(&jhandle, "name", strlen("name")),
				      json_string_new(&jhandle, "angelo", strlen("angelo")),
				      (void *)0),
			  json_string_new(&jhandle, "2", strlen("2")),
			  json_string_new(&jhandle, "3", strlen("3")),
			  json_string_new(&jhandle, "4", strlen("4")),
			  (void *)0);
#endif

    jobject = json_object_new(&jhandle, (void *)0);    
    json_object_add(&jhandle,
		    jobject, 
		    json_string_new(&jhandle, "name", strlen("name")),
		    json_string_new(&jhandle, "angelo", strlen("angelo")));

    jobjectXXX = json_array_new(&jhandle, (void *)0);
    json_array_add(&jhandle,
		   jobjectXXX,
		   json_string_new(&jhandle, "1", strlen("1")));

    json_array_add(&jhandle,
		   jobjectXXX,
		   json_string_new(&jhandle, "2", strlen("2")));

    json_object_add(&jhandle,
		    jobject, 
		    json_string_new(&jhandle, "array", strlen("array")),
		    jobjectXXX);
        
    json_dump(&jhandle, jobject);

    json_free(&jhandle);
  }
  
  return 0;
}
