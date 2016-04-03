# Simple OBJ Parser (SOP)

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

* Clone the repository `git clone git@github.com:littlstar/sop.git`
* Change directory to project directory `cd sop`
* Build the library `make`
* Run tests `make test`
* Install into system `make install` (Uninstall with `make uninstall`)
* or copy static libary into your project with the headers found in
  `include/`

### Adding to your clib project

#### Installing library source into your project

```sh
$ clib install littlstar/sop --save
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

Consider this simple model describing a cube

```obj
## This is a comment
v -0.5 -0.5 +0.5
v +0.5 -0.5 +0.5
v -0.5 +0.5 +0.5
v +0.5 +0.5 +0.5
v -0.5 -0.5 -0.5
v +0.5 -0.5 -0.5
v -0.5 +0.5 -0.5
v +0.5 +0.5 -0.5
f 0 1 3
f 0 3 2
f 1 5 7
f 1 7 3
f 5 4 6
f 5 6 7
f 4 0 2
f 4 2 6
f 4 5 1
f 4 1 0
f 2 3 7
f 2 7 6
```

```c
#include <sop/sop.h>
#include <string.h>
#include <stdio.h>
#include <fs/fs.h>

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
  const char *src = fs_read("cube.obj");
  sop_parser_t parser;
  sop_parser_options_t options = {
    .callbacks = {
      .oncomment = oncomment,
      .ontexture = ontexture,
      .onvertex = onvertex,
      .onface = onface
    }
  };

  assert(SOP_EOK == sop_parser_init(&parser, &options));
  assert(SOP_EOK == sop_parser_execute(&parser, src, strlen(src)));

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
  int faces[3][3];
  int vf[3], vtf[3], vnf[3]; // vertex, texture, & normal faces
  if (line.data) {
    memcpy(faces, line.data, sizeof(faces));
    memcpy(vf, faces[0], sizeof(faces[0]));
    memcpy(vtf, faces[1], sizeof(faces[1]));
    memcpy(vnf, faces[2], sizeof(faces[2]));
    // do something face data
  }
  return SOP_EOK;
}
```

## License

MIT
