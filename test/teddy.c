#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <sop/sop.h>
#include <ok/ok.h>
#include <fs/fs.h>

#include "test.h"

static int
on_comment(const sop_parser_state_t *state,
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
    .on_vertex = on_vertex,
    .on_face = on_face,
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
  TestState.counters.comments = 0;
  TestState.counters.vertices = 0;
  TestState.counters.faces = 0;
  memset(&TestState, 0, sizeof(TestState));
}

TEST(teddy) {
  ResetTestState();

  const char *src = fs_read("fixtures/teddy.obj");

  assert(SOP_EOK == sop_parser_init(&parser, &options));
  ok("teddy: sop_parser_init");
  assert(SOP_EOK == sop_parser_execute(&parser, src, strlen(src)));
  ok("teddy: sop_parser_exec");

  assert(0 == TestState.counters.comments);
  ok("teddy: comments parsed");

  assert(1598 == TestState.counters.vertices);
  ok("teddy: vertices parsed");

  assert(3192 == TestState.counters.faces);
  ok("teddy: faces parsed");
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
            "teddy: .on_comment: comment line %d has length set",
            TestState.counters.comments);
    ok(message);
  }
  return SOP_EOK;
}

static int
on_vertex(const sop_parser_state_t *state,
          const sop_parser_line_state_t line) {
  // (x y z) - we ignore w component
  TestState.counters.vertices++;
  if (line.data) {
    assert(line.length);
  }

  return SOP_EOK;
}

static int
on_face(const sop_parser_state_t *state,
        const sop_parser_line_state_t line) {
  TestState.counters.faces++;
  if (line.data) {
    assert(line.length);
  }
  return SOP_EOK;
}
