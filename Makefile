## -------------------------------------------------------------------- *
##
## Copyright 2012 Angelo Masci
## 
## Permission is hereby granted, free of charge, to any person obtaining a
## copy of this software and associated documentation files (the 
## "Software"), to deal in the Software without restriction, including 
## without limitation the rights to use, copy, modify, merge, publish, 
## distribute, sublicense, and/or sell copies of the Software, and to permit 
## persons to whom the Software is furnished to do so, subject to the 
## following conditions:
## 
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
## 
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
## IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
## CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
## OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR 
## THE USE OR OTHER DEALINGS IN THE SOFTWARE.
## 
## --------------------------------------------------------------------

CC=gcc
CFLAGS=-I. -I./extras -O3 -Wall -Wextra -fomit-frame-pointer -march=native -mtune=native -std=c89
C99CFLAGS=-I. -I./extras -O3 -Wall -Wextra -fomit-frame-pointer -march=native -mtune=native -D_GNU_SOURCE -std=c99 

all: amjson examples/example1 examples/example2 examples/example3 examples/example4 examples/example5

amjson.o: amjson.c amjson.h
	$(CC) -c -o amjson.o amjson.c $(CFLAGS)

extras/amjson_util.o: extras/amjson_util.c extras/amjson_util.h  amjson.h
	$(CC) -c -o extras/amjson_util.o extras/amjson_util.c $(CFLAGS)

extras/amjson_dump.o: extras/amjson_dump.c extras/amjson_dump.h amjson.h
	$(CC) -c -o extras/amjson_dump.o extras/amjson_dump.c $(CFLAGS)

extras/amjson_file.o: extras/amjson_file.c extras/amjson_file.h amjson.h
	$(CC) -c -o extras/amjson_file.o extras/amjson_file.c $(CFLAGS)

extras/amjson_query.o: extras/amjson_query.c extras/amjson_query.h extras/amjson_util.h amjson.h
	$(CC) -c -o extras/amjson_query.o extras/amjson_query.c $(CFLAGS)

extras/amjson_main.o: extras/amjson_main.c amjson.h extras/amjson_file.h extras/amjson_dump.h extras/amjson_query.h extras/amjson_util.h
	$(CC) -c -o extras/amjson_main.o extras/amjson_main.c $(C99CFLAGS)

amjson: amjson.o extras/amjson_util.o extras/amjson_dump.o extras/amjson_file.o extras/amjson_query.o extras/amjson_main.o
	$(CC) -o amjson amjson.o extras/amjson_util.o extras/amjson_dump.o extras/amjson_file.o extras/amjson_query.o extras/amjson_main.o $(CFLAGS)

examples/example1.o: amjson.o examples/example1.c
	$(CC) -c -o examples/example1.o examples/example1.c $(CFLAGS)

examples/example1: amjson.o examples/example1.o
	$(CC) -o examples/example1 amjson.o examples/example1.o $(CFLAGS)

examples/example2.o: amjson.o examples/example2.c 
	$(CC) -c -o examples/example2.o examples/example2.c $(CFLAGS)

examples/example2: amjson.o examples/example2.o extras/amjson_dump.o
	$(CC) -o examples/example2 amjson.o examples/example2.o extras/amjson_dump.o $(CFLAGS)

examples/example3.o: amjson.o examples/example3.c
	$(CC) -c -o examples/example3.o examples/example3.c $(CFLAGS)

examples/example3: amjson.o examples/example3.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o
	$(CC) -o examples/example3 amjson.o examples/example3.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o $(CFLAGS)

examples/example4.o: amjson.o examples/example4.c
	$(CC) -c -o examples/example4.o examples/example4.c $(CFLAGS)

examples/example4: amjson.o examples/example4.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o extras/amjson_mod.o
	$(CC) -o examples/example4 amjson.o examples/example4.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o extras/amjson_mod.o $(CFLAGS)

examples/example5.o: amjson.o examples/example5.c
	$(CC) -c -o examples/example5.o examples/example5.c $(CFLAGS)

examples/example5: amjson.o examples/example5.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o extras/amjson_mod.o
	$(CC) -o examples/example5 amjson.o examples/example5.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o extras/amjson_mod.o $(CFLAGS)

.PHONY: clean

clean:
	rm -f amjson amjson.o extras/amjson_util.o extras/amjson_dump.o extras/amjson_file.o \
              extras/amjson_query.o extras/amjson_mod.o extras/amjson_main.o examples/example1 \
              examples/example1.o examples/example2 examples/example2.o examples/example3 \
              examples/example3.o examples/example4 examples/example4.o examples/example5 \
              examples/example5.o tests/performance/genjson.o tests/performance/genjson \
              tests/performance/result

.PHONY: test

test: amjson
	@tests/JSONTestSuite/run.sh tests/JSONTestSuite/test_parsing ./amjson
	@tests/JSON_checker/run.sh tests/JSON_checker/test ./amjson

.PHONY: perf

tests/performance/genjson: tests/performance/genjson.c
	$(CC) -o tests/performance/genjson tests/performance/genjson.c $(CFLAGS)

tests/performance/datasets:
	mkdir tests/performance/datasets

tests/performance/datasets/1mb.json: tests/performance/genjson tests/performance/datasets
	tests/performance/genjson tests/performance/datasets/1mb.json 1M

tests/performance/datasets/50mb.json: tests/performance/genjson tests/performance/datasets
	tests/performance/genjson tests/performance/datasets/50mb.json 50M

tests/performance/datasets/100mb.json: tests/performance/genjson tests/performance/datasets
	tests/performance/genjson tests/performance/datasets/100mb.json 100M

tests/performance/datasets/500mb.json: tests/performance/genjson tests/performance/datasets
	tests/performance/genjson tests/performance/datasets/500mb.json 500M

perf: amjson tests/performance/genjson tests/performance/datasets/1mb.json tests/performance/datasets/50mb.json tests/performance/datasets/100mb.json tests/performance/datasets/500mb.json
	@tests/performance/run.sh tests/performance ./amjson

## --------------------------------------------------------------------
## --------------------------------------------------------------------
