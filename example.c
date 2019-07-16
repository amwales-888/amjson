#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "json.h"


static double TimeSpecToSeconds(struct timespec* ts)
{
  return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

int main(int argc, char **argv) {

  struct timespec start;
  struct timespec end;
  double elapsedSeconds;
  struct jhandle_s jhandle;
  char *filepath;

  if ((argc < 2) || (argc > 3)) {
    fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
    fprintf(stderr, "       %s <filepath> <query>\n", argv[0]);
    fprintf(stderr, "       %s <filepath> --dump\n", argv[0]);
    return 1;
  }

  filepath = argv[1];
  clock_gettime(CLOCK_MONOTONIC, &start); 

  if (json_alloc(&jhandle, (void *)0, 8 * 1024 * 1024) == 0) {    
    if (json_file_decode(&jhandle, filepath) == 0) {      
      printf("JSON valid [%d items]\n", jhandle.used);

      if (argc == 3) {  
	if (strcmp(argv[2],"--dump") == 0) {
	  json_dump(&jhandle, (void *)0);
	} else {
	  char *query = argv[2];
	  struct jobject_s *jobject = json_query(&jhandle, JOBJECT_ROOT(&jhandle), query);
	  if (jobject) {
	    json_dump(&jhandle, jobject);
	  } else {
	    printf("'%s' not found\n", query);
	    return 1;
	  }
	}
      }
    } else {
      printf("JSON invalid\n");
      return 1;
    }

    json_free(&jhandle);
  } else {
    printf("JSON alloc failed\n");
    return 1;
  }

  clock_gettime(CLOCK_MONOTONIC, &end);
  elapsedSeconds = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);
  printf("Ellapsed time seconds:%f\n", elapsedSeconds);
  
  return 0;
}
