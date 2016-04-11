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
on_texture(const sop_parser_state_t *state,
           const sop_parser_line_state_t line);

static int
on_normal(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);

static int
on_face(const sop_parser_state_t *state,
        const sop_parser_line_state_t line);

static int
on_smooth(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);

static int
on_material_new(const sop_parser_state_t *state,
                const sop_parser_line_state_t line);

static int
on_material_use(const sop_parser_state_t *state,
                const sop_parser_line_state_t line);

static int
on_material_lib(const sop_parser_state_t *state,
                const sop_parser_line_state_t line);

static sop_parser_t parser;
static sop_parser_options_t options = {
  .callbacks = {
    .on_material_new = on_material_new,
    .on_material_use = on_material_use,
    .on_material_lib = on_material_lib,
    .on_texture = on_texture,
    .on_comment = on_comment,
    .on_normal = on_normal,
    .on_vertex = on_vertex,
    .on_smooth = on_smooth,
    .on_face = on_face,
  }
};

static struct {
  struct {
    int materialsused;
    int materiallibs;
    int materials;
    int comments;
    int vertices;
    int textures;
    int normals;
    int faces;
  } counters;
  int smooth;
} TestState;

static void ResetTestState(void) {
  memset(&TestState, 0, sizeof(TestState));
}

TEST(material) {
  ResetTestState();

  const char *src = fs_read("fixtures/cube.obj");

  assert(SOP_EOK == sop_parser_init(&parser, &options));
  ok("material: sop_parser_init");
  assert(SOP_EOK == sop_parser_execute(&parser, src, strlen(src)));
  ok("material: sop_parser_exec");

  assert(0 == TestState.counters.comments);
  ok("material: comments parsed");

  assert(8 == TestState.counters.vertices);
  ok("material: vertices parsed");

  assert(14 == TestState.counters.textures);
  ok("material: textures parsed");

  assert(6 == TestState.counters.normals);
  ok("material: normals parsed");

  assert(12 == TestState.counters.faces);
  ok("material: faces parsed");

  assert(6 == TestState.counters.materials);
  ok("material: material parsed");

  assert(6 == TestState.counters.materialsused);
  ok("material: materials used parsed");
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
            "material: .on_comment: comment line %d has length set",
            TestState.counters.comments);
    ok(message);
  }
  return SOP_EOK;
}

static int
on_vertex(const sop_parser_state_t *state,
          const sop_parser_line_state_t line) {
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

static int
on_texture(const sop_parser_state_t *state,
           const sop_parser_line_state_t line) {
  TestState.counters.textures++;
  if (line.data && line.length) {
    assert(line.length);
  }
  return SOP_EOK;
}

static int
on_normal(const sop_parser_state_t *state,
           const sop_parser_line_state_t line) {
  TestState.counters.normals++;
  if (line.data && line.length) {
    assert(line.length);
  }
  return SOP_EOK;
}

static int
on_material_new(const sop_parser_state_t *state,
                const sop_parser_line_state_t line) {
  TestState.counters.materials++;
  if (line.data && line.length) {
    assert(line.length);
  }
  return SOP_EOK;
}

static int
on_material_use(const sop_parser_state_t *state,
                const sop_parser_line_state_t line) {
  TestState.counters.materialsused++;
  if (line.data && line.length) {
    assert(line.length);
  }
  return SOP_EOK;
}

static int
on_material_lib(const sop_parser_state_t *state,
                const sop_parser_line_state_t line) {
  TestState.counters.materiallibs++;
  if (line.data && line.length) {
    assert(line.length);
    const char *src = fs_read((char *) line.data);
    assert(SOP_EOK == sop_parser_execute(&parser, src, strlen(src)));
    ok("material: lib loaded");
  }
  return SOP_EOK;
}

static int
on_smooth(const sop_parser_state_t *state,
          const sop_parser_line_state_t line) {
  int toggle = *(int *) line.data;
  assert(0 == toggle);
  return SOP_EOK;
}
