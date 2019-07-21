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

    struct jobject *array;

    array = json_array_new(&jhandle,
			   json_string_new(&jhandle, "0", strlen("0")),
			   json_string_new(&jhandle, "1", strlen("1")),
			   json_object_new(&jhandle,
					   json_string_new(&jhandle, "name", strlen("name")),
					   json_string_new(&jhandle, "bob", strlen("bob")),
					   (void *)0),
			   json_string_new(&jhandle, "2", strlen("2")),
			   json_string_new(&jhandle, "3", strlen("3")),
			   json_string_new(&jhandle, "4", strlen("4")),
			   (void *)0);

    json_array_add(&jhandle,
		   array, 
		   json_string_new(&jhandle, "5", strlen("5")));

    json_array_add(&jhandle,
		   array, 
		   json_object_new(&jhandle,
				   json_string_new(&jhandle, "name", strlen("name")),
				   json_string_new(&jhandle, "dave", strlen("dave")),
				   (void *)0));    
    json_dump(&jhandle, array);

    json_free(&jhandle);
  }
  
  return 0;
}
