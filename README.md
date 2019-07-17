# c-json
Simple JSON parser in C

This project attempts to provide a JSON parser that is able to 
successfully parse correct JSON formatted data as described in the 
JSON grammar available at https://www.json.org/

As per RFC 8259 section (9) we set a maximum depth when parsing 
elements, this is configurable and a default compile time constant 
of 1024 has been provided.

This library is usable with just 2 files being required json.h and 
json.c ALL other files are optional.

The parser is supplied with a sample utility that is generated
when you make this project.

```
    Usage: ./json <filepath>
           ./json <filepath> <query>
           ./json <filepath> --dump

    filepath - Path to file or '-' to read from stdin
       query - Path to JSON object to display
       	       eg. "uk.people[10].name"
      --dump - Output JSON representation of data
```

You will be able to parse VERY large JSON files with this very
quickly. Running on a VM, 4 Core E3-1230 V2 @ 3.30GHz 4GB
RAM + 1GB Swap I was able to parse a file a little over 487MB
in 1.8 seconds and generated 75000003 objects (32 Bytes each).
Passing the same file through the default json parser in Python
2.7.9 took 18 seconds before it was killed by the OOM killer.

The parser was tested using the JSONTestSuite project found at
https://github.com/nst/JSONTestSuite all tests passed with no
crashes.

The implementation stores an object representation of each element
of the json data with a pool. The pool is either allocated by the
caller and passed in or left to the parser to allocate as required.

Each object in the pool is 32bits in length and points back to the
original JSON data, objects are chained together using offset from
the start of the pool. Since all offsets are to objects within the
pool and are allocated in sequential order, it is trivial to
reallocate the pool without affecting any references.

The parser is implemented using a single character lookahead
recursive decent parser. It is possible that large ammounts of
stack space may be used if the JSON being parsed is very deep,
however as per the RFC 8259 section (9) we can mitigate any
problems that can occur due to very deep nesting.

The functions provided outside of json.c include mechanisms to
dump any JSON object, a query mechanism to select objects from
the parsed data and a mechanism to memory map a file to be parsed.

