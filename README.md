# amjson
Simple JSON parser in C conforming to the C89 standard.

This project provide an RFC 8259 compliant JSON parser.
You can browse the JSON grammar at https://www.json.org/

This JSON parser is usable with just 2 C files being required 
amjson.h and amjson.c ALL other files are optional. Simply adding 
the the files to your project and using the three provided APIs.

```
int amjson_alloc(struct jhandle *jhandle, 
                 struct jobject *ptr, joff_t count);

int amjson_decode(struct jhandle *jhandle, 
                  char *buf, jsize_t len);
	
void amjson_free(struct jhandle *jhandle);
```

Once a JSON buffer has been parsed a DOM is created and can be
manipulated with the provided C Macros.

A number of examples are provided and can be found in the 'examples' 
directory.

The project examples can be built on linux using GNU make and GCC

```
    make
```

The parser is supplied with a example command line utility called
'amjson' that is generated when you make the examples.

```
    Usage: ./amjson filepath
           ./amjson filepath query
           ./amjson filepath --dump

      filepath      - Path to file or '-' to read from stdin
      query         - Path to JSON object to display
       	              eg. "uk.people[10].name"
      --dump        - Output minified JSON representation of data
      --dump-pretty - Output pretty printed JSON representation of data
      --benchmark   - Output parsing statistics
```

With this parser you will be able to parse VERY large JSON files
quickly. The commandline utility will use mmap() to map the file 
data into the users address space, no changes to the data are
made and the only memory required is for the DOM.

The parser was verfied agains the spcification using the JSONTestSuite
project which can be found at https://github.com/nst/JSONTestSuite all 
tests pass with no crashes. The parser was also verfied against the
tests found in https://json.org/JSON_checker/test.zip

You can verify this by using GNU make

```
    make test
```

If you want to test performance on your system using GNU make.
You will require upto 1GB of disk space for the benchmarks to run.

```
    make perf
```

Support functions are provided in the 'extras' directory.
They include functionality to, query the DOM, create a new DOM, dump
the DOM and manipulate the DOM.

##### Q. Why would I want to use this JSON parser?
You have small memory requirements, the internal representation 
for the DOM are configurable. They can be as low as 3 bytes!
Using 12 bytes gives you access to buffers upto 4GB in size.

The generation of the DOM can use a user provided buffer with
no reallocations. Alernatively you can leave allocations upto the
parser by providing as hint, all allocations are kept to a minimum.

You want to use C or C++, this parser can be easily integrated
into your C application or C++ application.

##### Q. Can I use this in an embedded system?
It's dependant on your stack space, since the parser is a single
character lookahead recursive descent parser it will require stack
space to call recursive functions, typically embedded systems have
small stacks.

As per RFC 8259 section (9) we set a maximum depth when parsing 
elements, this is configurable at compilation time, a default 
constant of 64 has been provided.

Using the compile time options it should be possile to choose
a relatively small stack depth and choose a small DOM 
representation size, YMWV.

