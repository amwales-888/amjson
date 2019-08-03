/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#include "amjson.h"
#include "extras/amjson_mod.h"
#include "extras/amjson_dump.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc __attribute__((unused)),
	 char **argv __attribute__((unused))) {

  struct jhandle jhandle;
  if (amjson_alloc(&jhandle, (void *)0, 32) == 0) {
    
    struct jobject *array;

    array = amjson_array_new(&jhandle,
			   amjson_string_new(&jhandle, "0", strlen("0")),
			   amjson_string_new(&jhandle, "1", strlen("1")),
			   amjson_object_new(&jhandle,
					   amjson_string_new(&jhandle, "name", strlen("name")),
					   amjson_string_new(&jhandle, "bob", strlen("bob")),
					   (void *)0),
			   amjson_string_new(&jhandle, "2", strlen("2")),
			   amjson_string_new(&jhandle, "3", strlen("3")),
			   amjson_string_new(&jhandle, "4", strlen("4")),
			   (void *)0);

    amjson_array_add(&jhandle,
		   array, 
		   amjson_string_new(&jhandle, "5", strlen("5")));

    amjson_array_add(&jhandle,
		   array, 
		   amjson_object_new(&jhandle,
				   amjson_string_new(&jhandle, "list", strlen("list")),
				   amjson_array_new(&jhandle,
						  amjson_string_new(&jhandle, "x", strlen("x")),
						  amjson_string_new(&jhandle, "y", strlen("y")),
						  (void *)0),
				   (void *)0));    
    amjson_dump(&jhandle, array, 1);

    amjson_free(&jhandle);
  }
  
  return 0;
}
