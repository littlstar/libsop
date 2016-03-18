# libsop

Simple OBJ File Format Parser written in C99.
This library makes it easy to parse OBJ files for quickly loading
simple models in OpenGL. The parser provides a callback interfaces giving the
consumer the responsibility of storing Geometric Vertices (v), Texture
Vertices (vt), Vertext Normals (vn), and Face Indices (f). This library
can be built from source or consumed with
[clib](https://github.com/clibs/clib).

## Installation

### Building from source

Building from source is simple:

* Clone the repository `git clone git@github.com:littlstar/libsop.git`
* Change directory to project directory `cd libsop`
* Build the library `make`
* Run tests `make test`
* Install into system `make install` (Uninstall with `make uninstall`)
* or copy static libary into your project with the headers found in
  `include/`

### Adding to your clib project

#### Installing library source into your project

```sh
$ clib install littlstar/libsop --save
```

#### Adding source to your project

##### In a Makefile

Append to the source explicitly:

```Makefile
SRC += $(wildcard deps/sop/*.c)
```

or everything found in `deps/`:

```Makefile
SRC += $(wildcard deps/*/*.c)
```

Append include path to `CFLAGS` explicitly:

```Makefile
CFLAGS += -Ideps
```

##### From the command line

```
$ gcc -Ideps deps/sop/*.c *.c
```

### Usage/Example

```c
#include <sop/sop.h>

static int
oncomment(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);
static int
onvertex(const sop_parser_state_t *state,
         const sop_parser_line_state_t line);

static int
onface(const sop_parser_state_t *state,
       const sop_parser_line_state_t line);

int main(void) {
  const char *src = OBJ(
    ## This is a comment \n
    v -0.5 -0.5 +0.5 \n
    v +0.5 -0.5 +0.5 \n
    v -0.5 +0.5 +0.5 \n
    v +0.5 +0.5 +0.5 \n
    v -0.5 -0.5 -0.5 \n
    v +0.5 -0.5 -0.5 \n
    v -0.5 +0.5 -0.5 \n
    v +0.5 +0.5 -0.5 \n
    f 0 1 3 \n
    f 0 3 2 \n
    f 1 5 7 \n
    f 1 7 3 \n
    f 5 4 6 \n
    f 5 6 7 \n
    f 4 0 2 \n
    f 4 2 6 \n
    f 4 5 1 \n
    f 4 1 0 \n
    f 2 3 7 \n
    f 2 7 6 \n
  );

  return 0;
}

static int
oncomment(const sop_parser_state_t *state,
          const sop_parser_line_state_t line) {
  char message[BUFSIZ];
  TestState.counters.comments++;
  if (line.data && strlen((char *) line.data)) {
    // do something with comment
  }
  return SOP_EOK;
}

static int
onvertex(const sop_parser_state_t *state,
         const sop_parser_line_state_t line) {
  // (x y z) - we ignore w component
  float vertex[3];
  if (line.data) {
    memcpy(vertex, line.data, sizeof(vertex));
    // do something vertex data
  }

  return SOP_EOK;
}

static int
onface(const sop_parser_state_t *state,
       const sop_parser_line_state_t line) {
  unsigned int faces[3];
  if (line.data) {
    memcpy(faces, line.data, sizeof(vertex));
    // do something face data
  }
  return SOP_EOK;
}
```

## License

MIT
