#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <sop/sop.h>
#include <ok/ok.h>

#include "test.h"

static int
on_comment(const sop_parser_state_t *state,
           const sop_parser_line_state_t line);
static int
on_texture(const sop_parser_state_t *state,
           const sop_parser_line_state_t line);
static int
on_vertex(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);

static int
on_face(const sop_parser_state_t *state,
        const sop_parser_line_state_t line);

static sop_parser_t parser;
static sop_parser_options_t options = {
  .callbacks = {
    .on_comment = on_comment,
    .on_texture = on_texture,
    .on_vertex = on_vertex,
    .on_face = on_face,
  }
};

static struct {
  struct {
    int comments;
    int textures;
    int vertices;
    int faces;
  } counters;
} TestState;

static void ResetTestState(void) {
  memset(&TestState, 0, sizeof(TestState));
}

TEST(simple) {
  ResetTestState();

  const char *src = ""
    "## This is a comment \n"
    "v -0.5 -0.5 +0.5 \n"
    "v +0.5 -0.5 +0.5 \n"
    "v -0.5 +0.5 +0.5 \n"
    "v +0.5 +0.5 +0.5 \n"
    "v -0.5 -0.5 -0.5 \n"
    "v +0.5 -0.5 -0.5 \n"
    "v -0.5 +0.5 -0.5 \n"
    "v +0.5 +0.5 -0.5 \n"
    "vt 0.539062 0.812500 0.000000\n"
    "vt 0.546875 0.812500 0.000000\n"
    "vt 0.554687 0.812500 0.000000\n"
    "vt 0.562500 0.812500 0.000000\n"
    "vt 0.570312 0.812500 0.000000\n"
    "vt 0.578125 0.812500 0.000000\n"
    "vt 0.585937 0.812500 0.000000\n"
    "vt 0.593750 0.812500 0.000000\n"
    "f 0 1 3 \n"
    "f 0 3 2 \n"
    "f 1 5 7 \n"
    "f 1 7 3 \n"
    "f 5 4 6 \n"
    "f 5 6 7 \n"
    "f 4 0 2 \n"
    "f 4 2 6 \n"
    "f 4 5 1 \n"
    "f 4 1 0 \n"
    "f 2 3 7 \n"
    "f 2 7 6 \n"
    "";

  assert(SOP_EOK == sop_parser_init(&parser, &options));
  ok("simple: sop_parser_init");
  assert(SOP_EOK == sop_parser_execute(&parser, src, strlen(src)));
  ok("simple: sop_parser_exec");

  assert(1 == TestState.counters.comments);
  ok("simple: comments parsed");

  assert(8 == TestState.counters.vertices);
  ok("simple: vertices parsed");

  assert(8 == TestState.counters.textures);
  ok("simple: textures parsed");

  assert(12 == TestState.counters.faces);
  ok("simple: faces parsed");

  ok_done();
  return 0;
}

static int
on_comment(const sop_parser_state_t *state,
           const sop_parser_line_state_t line) {
  char message[BUFSIZ];
  TestState.counters.comments++;
  if (line.data && strlen((char *) line.data)) {
    assert(line.length);
    sprintf(message,
            "simple: .oncomment: comment line %d has length set",
            TestState.counters.comments);
    ok(message);
  }
  return SOP_EOK;
}

static int
on_texture(const sop_parser_state_t *state,
           const sop_parser_line_state_t line) {
  char message[BUFSIZ];
  TestState.counters.textures++;
  if (line.data && line.length) {
    assert(line.length);
    sprintf(message,
            "simple: .ontexture: texture line %d has length set",
            TestState.counters.textures);
    ok(message);
  }
  return SOP_EOK;
}

static int
on_vertex(const sop_parser_state_t *state,
          const sop_parser_line_state_t line) {
  // (x y z) - we ignore w component
  float vertex[3];
  char message[BUFSIZ];
  TestState.counters.vertices++;
  if (line.data) {
    assert(line.length);
    sprintf(message,
            "simple: .on_vertex: vertex line %d has length set",
            TestState.counters.vertices);
    ok(message);
    memcpy(vertex, line.data, sizeof(vertex));
    assert(0 != vertex[0]);
    assert(0 != vertex[1]);
    assert(0 != vertex[2]);
  }

  return SOP_EOK;
}

static int
on_face(const sop_parser_state_t *state,
        const sop_parser_line_state_t line) {
  TestState.counters.faces++;
  int faces[3][3];
  char message[BUFSIZ];
  if (line.data) {
    assert(line.length);
    sprintf(message,
            "simple: .on_face: face line %d has length set",
            TestState.counters.faces);
    ok(message);
    memcpy(faces, line.data, sizeof(faces));
    assert(faces[0][0] >= 0 && faces[0][0] <= 7);
    assert(faces[0][1] >= 0 && faces[0][1] <= 7);
    assert(faces[0][2] >= 0 && faces[0][2] <= 7);

    assert(-1 == faces[1][0]);
    assert(-1 == faces[1][1]);
    assert(-1 == faces[1][2]);
    assert(-1 == faces[2][0]);
    assert(-1 == faces[2][1]);
    assert(-1 == faces[2][2]);
  }
  return SOP_EOK;
}
