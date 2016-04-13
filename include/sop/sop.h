#ifndef LIBSOP_H
#define LIBSOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * SOP types.
 */

typedef enum sop_enum sop_enum_t;
typedef struct sop_parser sop_parser_t;
typedef struct sop_parser_state sop_parser_state_t;
typedef struct sop_parser_options sop_parser_options_t;
typedef struct sop_parser_line_state sop_parser_line_state_t;

/**
 * This function pointer typedef defines the signature for a line callback
 */

typedef int (* sop_parser_line_cb) (const sop_parser_state_t *state,
                                    const sop_parser_line_state_t);

/**
 * SOP enum values.
 */

enum sop_enum {
  // Out of bounds state/value
  SOP_OOB = -1,

  // represents a empty state/value
  SOP_NULL = 0,

  /**
   * SOP errors
   */

  SOP_EOK,
  SOP_EMEM,
  SOP_EINVALID_OPTIONS,
  SOP_EINVALID_SOURCE,

  // represents a comment value
  SOP_COMMENT,

  /**
   * OBJ directive types
   *  See: (http://www.martinreddy.net/gfx/3d/OBJ.spec)
   *
   * Vertex data:
   *   - (v) geometric vertices (x y z w)
   *   - (vt) texture vertices (u v w)
   *   - (vn) vertex normals (x y z w)
   *   - (f) face (x/u/i y/v/j z/w/k)
   *   - (s) smooth shading (on/off)
   *   - (usemtl) material name (name)
   *   - (mtllib) material library (name)
   */

  SOP_DIRECTIVE_VERTEX,
  SOP_DIRECTIVE_VERTEX_TEXTURE,
  SOP_DIRECTIVE_VERTEX_NORMAL,
  SOP_DIRECTIVE_FACE,
  SOP_DIRECTIVE_SMOOTH,
  SOP_DIRECTIVE_USE_MTL,
  SOP_DIRECTIVE_MTL_LIB,

  /**
   * Material lib types.
   *   See: (http://web.cse.ohio-state.edu/~hwshen/581/Site/Lab3_files/Labhelp_Obj_parser.htm)
   *
   * Material data:
   *   - (newmtl) Start a definition of a new material
   *   - (Ka) ambient color (r g b a)
   *   - (Kd) diffuse color (r g b a)
   *   - (Ks) specular color (r g b a)
   *   - (illum) Define the illumination model:
   *     - [0] no illumination
   *     - [1] a flat material with no specular highlights
   *     - [2] denotes the presence of specular highlights
   *   - (Ns) shininess of the material (shininess)
   *   - (d or Tr) the transparency of the material (transparency)
   */

  SOP_DIRECTIVE_MATERIAL_NEW,
  SOP_DIRECTIVE_MATERIAL_AMBIENT_COLOR,
  SOP_DIRECTIVE_MATERIAL_DIFFUSE_COLOR,
  SOP_DIRECTIVE_MATERIAL_SPECULAR_COLOR,
  SOP_DIRECTIVE_MATERIAL_ILLUM,
  SOP_DIRECTIVE_MATERIAL_SHININESS,
  SOP_DIRECTIVE_MATERIAL_TRANSPARENCY,
};

/**
 * The associated callback fields defined in the SOP options
 * structure and the parser structure instance. We do this to avoid
 * defining a struct or typedef.
 */

#define SOP_PARSER_CALLBACK_FIELDS             \
  sop_parser_line_cb on_material_transparency; \
  sop_parser_line_cb on_material_shininess;    \
  sop_parser_line_cb on_material_specular;     \
  sop_parser_line_cb on_material_ambient;      \
  sop_parser_line_cb on_material_diffuse;      \
  sop_parser_line_cb on_material_illum;        \
  sop_parser_line_cb on_material_lib;          \
  sop_parser_line_cb on_material_use;          \
  sop_parser_line_cb on_material_new;          \
  sop_parser_line_cb on_texture;               \
  sop_parser_line_cb on_comment;               \
  sop_parser_line_cb on_vertex;                \
  sop_parser_line_cb on_normal;                \
  sop_parser_line_cb on_smooth;                \
  sop_parser_line_cb on_face;                  \

/**
 * This structure represents the options available for initializing the
 * parser with state and callback function pointers.
 */

struct sop_parser_options {
  // user pointer given to sop_parser_state
  void *data;

  // user defined callbacks
  struct { SOP_PARSER_CALLBACK_FIELDS } callbacks;
};

/**
 * This structure reprents the current parsed line
 * in the OBJ file source when the parsers is executed
 * against a given source.
 */

struct sop_parser_line_state {
  // the directive type
  sop_enum_t type;

  // line directive
  char *directive;

  // current line number of the source
  int lineno;

  // line data after directive
  void *data;

  // line data length after directive
  size_t length;
};

/**
 * This structure represents the current parser state.
 */

struct sop_parser_state {
  // user data pointer from sop_parser_options
  void *data;

  // pointer to the current state of the currently parsed line
  sop_parser_line_state_t *line;
};

/**
 * OBJ file format parser structure. This structure holds the state of the
 * parser and pointers to callback functions that should handle input from
 * OBJ content being parsed.
 */

struct sop_parser {
  // parser state that is updated when parsing an
  // OBJ file source
  sop_parser_state_t *state;

  // pointer to parser options
  sop_parser_options_t *options;

  // user defined callbacks given from sop_parser_options
  struct { SOP_PARSER_CALLBACK_FIELDS } callbacks;
};

/**
 * Initializes SOP parser with options.
 */

int
sop_parser_init(sop_parser_t *parser,
                sop_parser_options_t *options);

/**
 * Execute SOP parser for a given source.
 */

int
sop_parser_execute(sop_parser_t *,
                   const char *source,
                   size_t length);

#ifdef __cplusplus
}
#endif
#endif
