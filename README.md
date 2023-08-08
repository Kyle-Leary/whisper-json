# whisper JSON parser

pure C JSON parsing library. 

## building

run
```bash
make
```
in the root directory to build the library, and
```bash
make test
```
to test the parser with some example JSON.

## usage

link against the static library libwjson.a created with make, and then include the /api headers
into your project.
