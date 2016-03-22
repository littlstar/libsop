#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <sop/sop.h>
#include <ok/ok.h>

#include "test.h"

static int
oncomment(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);
static int
onvertex(const sop_parser_state_t *state,
         const sop_parser_line_state_t line);

static int
onface(const sop_parser_state_t *state,
       const sop_parser_line_state_t line);

static sop_parser_t parser;
static sop_parser_options_t options = {
  .callbacks = {
    .oncomment = oncomment,
    .onvertex = onvertex,
    .onface = onface,
  }
};

static struct {
  struct {
    int comments;
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

  assert(12 == TestState.counters.faces);
  ok("simple: faces parsed");

  ok_done();
  return 0;
}

static int
oncomment(const sop_parser_state_t *state,
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
onvertex(const sop_parser_state_t *state,
         const sop_parser_line_state_t line) {
  // (x y z) - we ignore w component
  float vertex[3];
  char message[BUFSIZ];
  TestState.counters.vertices++;
  if (line.data) {
    assert(line.length);
    sprintf(message,
            "simple: .onvertex: vertex line %d has length set",
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
onface(const sop_parser_state_t *state,
       const sop_parser_line_state_t line) {
  TestState.counters.faces++;
  int faces[3][3];
  char message[BUFSIZ];
  if (line.data) {
    assert(line.length);
    sprintf(message,
            "simple: .onface: face line %d has length set",
            TestState.counters.faces);
    ok(message);
    memcpy(faces, line.data, sizeof(faces));
    assert(faces[0][0] >= 0 && faces[0][0] <= 7);
    assert(faces[1][0] >= 0 && faces[1][0] <= 7);
    assert(faces[2][0] >= 0 && faces[2][0] <= 7);

    assert(-1 == faces[0][1]);
    assert(-1 == faces[0][2]);
    assert(-1 == faces[1][1]);
    assert(-1 == faces[1][2]);
    assert(-1 == faces[2][1]);
    assert(-1 == faces[2][2]);
  }
  return SOP_EOK;
}
