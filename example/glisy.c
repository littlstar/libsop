#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glfw-shell/glfw-shell.h>
#include <glisy/glisy.h>
#include <sop/sop.h>
#include <fs/fs.h>

#define WINDOW_NAME "Model from OBJ"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640

#define INITIAL_VERTEX_LENGTH 66
#define INITIAL_FACE_LENGTH 132

#define dtor(d) d * (M_PI / 180)
#define min(a, b) (a < b ? a : b < a ? b : a)
#define max(a, b) (a > b ? a : b > a ? b : a)

#define LoadShader(path) fs_read(path)
#define CreateProgram(vertex, fragment) ({ \
  GlisyProgram program; \
  GlisyShader vertexShader; \
  GlisyShader fragmentShader; \
  glisyProgramInit(&program); \
  glisyShaderInit(&vertexShader, GL_VERTEX_SHADER, LoadShader(vertex)); \
  glisyShaderInit(&fragmentShader, GL_FRAGMENT_SHADER, LoadShader(fragment)); \
  glisyProgramAttachShader(&program, &vertexShader); \
  glisyProgramAttachShader(&program, &fragmentShader); \
  glisyProgramLink(&program); \
  glisyProgramBind(&program); \
  (program); \
})

typedef struct Camera Camera;
struct Camera {
  GlisyUniform uProjection;
  GlisyUniform uView;

  mat4 projection;
  mat4 transform;
  mat4 view;

  vec3 position;
  vec3 target;
  vec3 up;

  struct {
    vec3 direction;
    vec3 right;
    vec3 up;
  } orientation;

  float aspect;
  float near;
  float far;
  float fov;
};

void
UpdateCameraProjectionMatrix(Camera *camera) {
  camera->projection = mat4_perspective(camera->fov,
                                        camera->aspect,
                                        camera->near,
                                        camera->far);
  glisyUniformSet(&camera->uProjection,
                    &camera->projection,
                    sizeof(camera->projection));
  glisyUniformBind(&camera->uProjection, 0);
}

void
UpdateCameraLookAt(Camera *camera) {
  vec3 target = camera->target;
  vec3 position = vec3_transform_mat4(camera->position,
                                      camera->transform);
  camera->view = mat4_lookAt(position, target, camera->up);
  glisyUniformBind(&camera->uView, 0);
  mat4_identity(camera->transform);
}

void
UpdateCamera(Camera *camera) {
  camera->orientation.direction =
    vec3_normalize(vec3_subtract(camera->position, camera->target));

  camera->orientation.right =
    vec3_normalize(vec3_cross(camera->up, camera->orientation.direction));

  glisyUniformSet(&camera->uView,
                    &camera->view,
                    sizeof(camera->view));

  UpdateCameraProjectionMatrix(camera);
  UpdateCameraLookAt(camera);
}

void
InitializeCamera(Camera *camera, int width, int height) {
  camera->position = vec3(0, 0, 0);
  camera->target = vec3(0, 0, 0);
  camera->up = vec3(0, 1, 0);

  camera->aspect = width / height;
  camera->near = 1.0f;
  camera->far = 100.0f;
  camera->fov = 45.0f;

  mat4_identity(camera->projection);
  mat4_identity(camera->transform);
  mat4_identity(camera->view);

  glisyUniformInit(&camera->uProjection,
                     "uProjection",
                     GLISY_UNIFORM_MATRIX, 4);

  glisyUniformInit(&camera->uView,
                     "uView",
                     GLISY_UNIFORM_MATRIX, 4);

  UpdateCamera(camera);
}

/**
 * sop callbacks
 */

static int
ontexture(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);

static int
oncomment(const sop_parser_state_t *state,
          const sop_parser_line_state_t line);

static int
onvertex(const sop_parser_state_t *state,
         const sop_parser_line_state_t line);

static int
onnormal(const sop_parser_state_t *state,
         const sop_parser_line_state_t line);

static int
onface(const sop_parser_state_t *state,
       const sop_parser_line_state_t line);

/**
 * simple model
 */

typedef struct Model Model;
struct Model {
  // vao
  float *vertices;
  float *normals;
  unsigned int *normalIndices;
  unsigned int *textures;
  unsigned int *faces;
  unsigned int *uvs;

  int verticesLength;
  int normalsLength;
  int facesLength;
  int uvsLength;

  // glisy
  GlisyGeometry geometry;
  GlisyUniform uModel;

  // gl
  mat4 transform;
  mat4 rotation;
  vec3 position;
};

static void
InitializeModel(Model *model, const char *objfile);

static void
UpdateModel(Model *model);

static void
DrawModel(Model *model);

void
RotateModel(Model *model, float radians, vec3 axis);

int
main(int argc, const char **argv) {
  const char *file = argv[1];

  // glfw
  GLFWwindow *window;

  // glisy
  GlisyProgram program;

  // objects
  Camera camera;
  Model model;

  if (!file) {
    fprintf(stderr, "e: Missing input file\nusage: %s <input.obj>\n", argv[0]);
    return 1;
  }

  // init
  GLFW_SHELL_CONTEXT_INIT(3, 2);
  GLFW_SHELL_WINDOW_INIT(window, WINDOW_WIDTH, WINDOW_HEIGHT);
  program = CreateProgram("shader/vertex.glsl", "shader/fragment.glsl");
  InitializeCamera(&camera, WINDOW_WIDTH, WINDOW_HEIGHT);
  InitializeModel(&model, file);

  // move camera behind model
  //camera.fov = 20;
  camera.position = vec3(0, 0, -2);

  GLFW_SHELL_RENDER(window, {
    const float time = glfwGetTime();
    const float angle = time * 20.0f;
    const float radians = dtor(angle);
    const vec3 rotation = vec3(0, 1, 0);
    (void) mat4_rotate(camera.transform,
                       radians,
                       rotation);

    // handle resize
    camera.aspect = width / height;

    // update camera orientation
    UpdateCamera(&camera);

    // rotate model at radians angle in opposite direction
    RotateModel(&model, radians, vec3_negate(rotation));

    // render model
    DrawModel(&model);
  });

  return 0;
}

static int
oncomment(const sop_parser_state_t *state,
          const sop_parser_line_state_t line) {
  if (line.length && line.data) {
    //printf("%s\n", (char *) line.data);
  }
  return SOP_EOK;
}

static int
onvertex(const sop_parser_state_t *state,
         const sop_parser_line_state_t line) {
  Model *model = (Model *) state->data;
  float vertex[3];

  if (line.data && line.length) {
    memcpy(vertex, line.data, sizeof(vertex));
  }

  if (model && (!model->verticesLength || !model->vertices)) {
    model->vertices = (float *) malloc(sizeof(float) * INITIAL_VERTEX_LENGTH);
    model->verticesLength = 0;
  } else if (model && model->vertices && model->verticesLength >= INITIAL_VERTEX_LENGTH) {
    float *ptr = realloc(model->vertices, sizeof(float) * (model->verticesLength + 3));
    if (ptr) {
      model->vertices = ptr;
    } else {
      free(model->vertices);
      model->vertices = 0;
      return SOP_EMEM;
    }
  }

  if (model && model->vertices) {
    model->vertices[model->verticesLength++] = vertex[0];
    model->vertices[model->verticesLength++] = vertex[1];
    model->vertices[model->verticesLength++] = vertex[2];
  }

  return SOP_EOK;
}

static int
onface(const sop_parser_state_t *state,
       const sop_parser_line_state_t line) {
  Model *model = (Model *) state->data;
  int face[3][3];
  int vf[3];

  if (line.data && line.length) {
    memcpy(face, line.data, sizeof(face));
    vf[0] = face[0][0];
    vf[1] = face[0][1];
    vf[2] = face[0][2];
  }

  if (model && (!model->facesLength || !model->faces)) {
    model->faces = (unsigned int *) malloc(sizeof(unsigned int) * INITIAL_FACE_LENGTH);
    model->facesLength = 0;
  } else if (model && model->faces && model->facesLength >= INITIAL_FACE_LENGTH) {
    unsigned int *ptr = realloc(model->faces, sizeof(unsigned int) * (model->facesLength + 3));
    if (ptr) {
      model->faces = ptr;
    } else {
      return SOP_EMEM;
    }
  }

  if (model && model->faces) {
    {
      if (vf[0] > -1) {
        model->faces[model->facesLength++] = vf[0];
      }

      if (vf[1] > -1) {
        model->faces[model->facesLength++] = vf[1];
      }

      if (vf[2] > -1) {
        model->faces[model->facesLength++] = vf[2];
      }
    }
  }

  return SOP_EOK;
}

static void
InitializeModel(Model *model, const char *objfile) {
  char *src = fs_read(objfile);
  sop_parser_t parser;
  sop_parser_options_t options = {
    .callbacks = {
      .oncomment = oncomment,
      .onvertex = onvertex,
      .onface = onface,
    }
  };

  if (!src) {
    printf("file %s not found\n", objfile);
    exit(0);
  }

  model->verticesLength = 0;
  model->normalsLength = 0;
  model->facesLength = 0;
  model->uvsLength = 0;

  model->vertices = 0;
  model->normals = 0;
  model->faces = 0;
  model->uvs = 0;

  options.data = model;
  assert(SOP_EOK == sop_parser_init(&parser, &options));
  assert(SOP_EOK == sop_parser_execute(&parser, src, strlen(src)));

  printf("*vertex count = %d\n", model->verticesLength / 3);
  printf("*face count = %d\n", model->facesLength / 3);

  // init color
  GlisyColor color;
  glisyColorInit(&color, "blue", 0);

  // init uniforms
  GlisyUniform uColor;
  glisyUniformInit(&uColor, "uColor", GLISY_UNIFORM_VECTOR, 3);
  glisyUniformInit(&model->uModel, "uModel", GLISY_UNIFORM_MATRIX, 4);

  // set uniforms
  glisyUniformSet(&uColor, &vec3(color.r, color.g, color.b), sizeof(vec3));
  glisyUniformBind(&uColor, 0);

  model->position = vec3(0, 0, 0);
  GLuint size = sizeof(float) * model->verticesLength;

  GlisyVAOAttribute vPosition = {
    .buffer = {
      .data = (void *) model->vertices,
      .type = GL_FLOAT,
      .size = size,
      .usage = GL_STATIC_DRAW,
      .offset = 0,
      .stride = 0,
      .dimension = 3,
    }
  };

  // init matrices
  mat4_identity(model->transform);
  mat4_identity(model->rotation);

  // init vao attributes
  glisyGeometryInit(&model->geometry);
  glisyGeometryAttr(&model->geometry, "vPosition", &vPosition);
  glisyGeometryFaces(&model->geometry,
                       GL_UNSIGNED_INT,
                       model->facesLength,
                       (void *) model->faces);

  // update geometry with attributes and faces
  glisyGeometryUpdate(&model->geometry);
}

void
UpdateModel(Model *model) {
  mat4 mat;
  mat4_identity(mat);
  mat = mat4_multiply(mat, model->rotation);
  glisyUniformSet(&model->uModel, &mat, sizeof(mat));
  glisyUniformBind(&model->uModel, 0);
}

void
DrawModel(Model *model) {
  UpdateModel(model);
  glisyGeometryBind(&model->geometry, 0);
  glisyGeometryDraw(&model->geometry, GL_LINE_STRIP, 0, model->facesLength);
  glisyGeometryUnbind(&model->geometry);
}

void
RotateModel(Model *model, float radians, vec3 axis) {
  (void) mat4_rotate(model->rotation, radians, axis);
}
