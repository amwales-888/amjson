/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

#include "amjson.h"
#include "extras/amjson_query.h"
#include "extras/amjson_dump.h"

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int main(int argc __attribute__((unused)),
	 char **argv __attribute__((unused))) {

  struct jhandle jhandle;
  char *amjson = "{ \"name\" : \"bob\" }";
  
  if (amjson_alloc(&jhandle, (void *)0, 32) == 0) {
    if (amjson_decode(&jhandle, amjson, strlen(amjson)) == 0) {
      struct jobject *jobject;
     
      if ((jobject = amjson_query(&jhandle, JOBJECT_ROOT(&jhandle), "name"))) {
	amjson_dump(&jhandle, jobject, 1, (char *)0, 0);
      }
    }
    amjson_free(&jhandle);
  }
  
  return 0;
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
