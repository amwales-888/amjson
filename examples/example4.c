/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#include "json.h"
#include "json_mod.h"
#include "json_dump.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc __attribute__((unused)),
	 char **argv __attribute__((unused))) {

  struct jhandle jhandle;
  if (json_alloc(&jhandle, (void *)0, 32) == 0) {
    
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
				   json_string_new(&jhandle, "list", strlen("list")),
				   json_array_new(&jhandle,
						  json_string_new(&jhandle, "x", strlen("x")),
						  json_string_new(&jhandle, "y", strlen("y")),
						  (void *)0),
				   (void *)0));    
    json_dump(&jhandle, array, 1);

    json_free(&jhandle);
  }
  
  return 0;
}
