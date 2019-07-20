# c-json
Simple JSON parser in C

This project attempts to provide an RFC 8259 compliant JSON parser.
You can browse the JSON grammar at https://www.json.org/

As per RFC 8259 section (9) we set a maximum depth when parsing 
elements, this is configurable and a default compile time constant 
of 1024 has been provided.

This library is usable with just 2 files being required json.h and 
json.c ALL other files are optional.

The parser is supplied with a sample commmand line utility that
is generated when you make this project.

```
    Usage: ./json filepath
           ./json filepath query
           ./json filepath --dump

    filepath - Path to file or '-' to read from stdin
       query - Path to JSON object to display
       	       eg. "uk.people[10].name"
      --dump - Output JSON representation of data
```

You will be able to parse VERY large JSON files with c-json very
quickly. Running on a VM, 4 Core E3-1230 V2 @ 3.30GHz 8GB
RAM + 1GB Swap I tested a 487MB and 1.2GB file using python as
a comparison.

| Parser            | 487MB | 1.2GB   |
| ----------------- | ----- | ------- |
| c-json            | 2s    | 5s      |
| Python 2.7.9 json | 26s   | 56s OOM |

The parser was tested using the JSONTestSuite project found at
https://github.com/nst/JSONTestSuite all tests passed with no
crashes.

The implementation stores an object representation of each element
of the json data with a pool. The pool is either allocated by the
caller and passed in or size is requested left to the parser to
allocate and reallocate as required. We keep reallocations to a
minimum and use a simplistic approach to resizing.

Each object in the pool is 12bytes in length, in the case of strings
and numbers we points back to the original JSON data using offset
from the start of the buffer. The original JSON data is never
modified and never copied in we simply store offset and length into
the original data.

Parser objects in the pool are chained together using offset from
the start of the pool. Since all offsets are to objects within the
pool and are allocated in sequential order, it is trivial to grow
the pool by simplying reallocating it without affecting any
references.

The parser is implemented using a single character lookahead
recursive decent parser. It is possible that large ammounts of
stack space may be used if the JSON being parsed is very deep,
however as per the RFC 8259 section (9) we can mitigate any
problems that can occur due to very deep nesting.

The functions provided outside of json.c include mechanisms to
dump any JSON object, a query mechanism to select objects from
the parsed data and a mechanism to memory map a file to be parsed.

A directory 'data' is included that contains some simple JSON
datasets.
