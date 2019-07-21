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
CFLAGS=-I. -O3 -Wall -Wextra -pedantic-errors -fomit-frame-pointer
DEPS=json.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: json example1 example2 example3 example4

json: json.o json_util.o json_dump.o json_file.o json_query.o json_main.o
	$(CC) -o $@ $^ $(CFLAGS)

example1: json.o example1.o
	$(CC) -o $@ $^ $(CFLAGS)

example2: json.o json_dump.o  example2.o
	$(CC) -o $@ $^ $(CFLAGS)

example3: json.o json_dump.o json_query.o json_util.o example3.o
	$(CC) -o $@ $^ $(CFLAGS)

example4: json.o json_dump.o json_query.o json_util.o json_mod.o example4.o 
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f json json.o json_util.o json_dump.o json_file.o json_query.o json_mod.o \
              json_main.o example1 example1.o example2 example2.o example3 example3.o example4 example4.o

## --------------------------------------------------------------------
## --------------------------------------------------------------------
