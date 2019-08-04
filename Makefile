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
CFLAGS=-I. -I./extras -O3 -Wall -Wextra -pedantic-errors -fomit-frame-pointer -std=c89
DEPS=amjson.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: amjson examples/example1 examples/example2 examples/example3 examples/example4

amjson: amjson.o extras/amjson_util.o extras/amjson_dump.o extras/amjson_file.o extras/amjson_query.o extras/amjson_main.o
	$(CC) -o $@ $^ $(C99CFLAGS)

examples/example1: amjson.o examples/example1.o
	$(CC) -o $@ $^ $(CFLAGS)

examples/example2: amjson.o extras/amjson_dump.o  examples/example2.o
	$(CC) -o $@ $^ $(CFLAGS)

examples/example3: amjson.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o examples/example3.o
	$(CC) -o $@ $^ $(CFLAGS)

examples/example4: amjson.o extras/amjson_dump.o extras/amjson_query.o extras/amjson_util.o extras/amjson_mod.o examples/example4.o 
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f amjson amjson.o extras/amjson_util.o extras/amjson_dump.o extras/amjson_file.o \
              extras/amjson_query.o extras/amjson_mod.o extras/amjson_main.o examples/example1 \
              examples/example1.o examples/example2 examples/example2.o examples/example3 \
              examples/example3.o examples/example4 examples/example4.o \
              tests/performance/genjson.o tests/performance/genjson \
              tests/performance/result

.PHONY: test

test: amjson
	@tests/JSON_checker/run.sh tests/JSON_checker/test ./amjson
	@tests/JSONTestSuite/run.sh tests/JSONTestSuite/test_parsing ./amjson

.PHONY: perf

tests/performance/genjson: tests/performance/genjson.o
	$(CC) -o $@ $^ $(CFLAGS)


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
