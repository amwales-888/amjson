# c-json
Simple JSON parser in C

This project attempts to provide a JSON parser that is able to 
successfully parse correct JSON formatted data as described in the 
JSON grammar available at https://www.json.org/

As per RFC 8259 section (9) we set a maximum depth when parsing 
elements, this is configurable and a default compile time constant 
has been provided.

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
      --dump - Output JSON representation of data
```

You will be able to parse VERY large files with this very quickly.
Running on a VM, 4 Core E3-1230 V2 @ 3.30GHz 4GB RAM + 1GB Swap I
was able to parse a file a little over 487MB in 1.8 seconds and
generated 75000003 objects (32 Bytes each). Passing the same file
through the default json parser in python 2.7.9 took 18 seconds
before it was killed by the OOM killer.

The parser was tested using the JSONTestSuite project found at
https://github.com/nst/JSONTestSuite all tests passed with no
crashes.