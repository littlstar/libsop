#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sop/sop.h>

int
sop_parser_init(sop_parser_t *parser,
                sop_parser_options_t *options) {
  if (!parser) { return SOP_EMEM; }
  memset(parser, 0, sizeof(sop_parser_t));
  parser->options = options;
#define SET_CALLBACK_IF(cb) \
  parser->callbacks. cb = options->callbacks. cb ? options->callbacks. cb : 0;
  SET_CALLBACK_IF(on_material_transparency);
  SET_CALLBACK_IF(on_material_shininess);
  SET_CALLBACK_IF(on_material_specular);
  SET_CALLBACK_IF(on_material_ambient);
  SET_CALLBACK_IF(on_material_diffuse);
  SET_CALLBACK_IF(on_material_illum);
  SET_CALLBACK_IF(on_material_lib);
  SET_CALLBACK_IF(on_material_use);
  SET_CALLBACK_IF(on_material_new);
  SET_CALLBACK_IF(on_texture);
  SET_CALLBACK_IF(on_comment);
  SET_CALLBACK_IF(on_vertex);
  SET_CALLBACK_IF(on_normal);
  SET_CALLBACK_IF(on_smooth);
  SET_CALLBACK_IF(on_face);
#undef SET_CALLBACK_IF
  return SOP_EOK;
}

int
sop_parser_execute(sop_parser_t *parser,
                   const char *source,
                   size_t length) {
  // handle poor state and input
  if (!parser) {
    return SOP_EMEM;
  } else if (!source || 0 == length) {
    return SOP_EINVALID_SOURCE;
  }

  // sop state
  sop_parser_line_state_t line;
  sop_parser_state_t state;
  sop_enum_t type = SOP_NULL;

  // setup sate pointers
  state.line = &line;
  state.data = parser->options->data;

  // source state
  size_t bufsize = 0;
  char buffer[BUFSIZ];
  char prev = 0;
  char ch0 = 0;
  char ch1 = 0;
  int lineno = 0;
  int colno = 0;

  // init buffer
  memset(buffer, 0, BUFSIZ);

#define RESET_LINE_STATE {   \
  memset(buffer, 0, BUFSIZ); \
  lineno++;                  \
  bufsize = 0;               \
  colno = 0;                 \
}

  for (int i = 0; i < length; ++i) {
    ch0 = source[i];
    ch1 = source[i + 1];

    if (i > 0) {
      prev = source[i - 1];
    }

    if ((' ' == ch0 && '\n' == prev)) {
      RESET_LINE_STATE;
      continue;
    }

    if (' ' == ch0 && 0 == colno) {
      ch0 = ch1;
      ch1 = source[++i];
    }

#define CALL_CALLBACK_IF(cb, ...) {              \
  int rc = SOP_EOK;                              \
  if (parser->callbacks. cb) {                   \
    rc = parser->callbacks. cb(__VA_ARGS__);     \
    if (rc != SOP_EOK) return rc;                \
  }                                              \
}
    // we've reached the end of the line and now need
    // to notify the consumer with a callback, state error,
    // or continue if there is nothing to do
    if ('\n' == ch0) {
      if (!bufsize) {
        RESET_LINE_STATE;
        continue;
      }
      line.data = 0;
      line.type = type;
      line.length = bufsize;
      line.data = (void *) buffer;
      switch (type) {
        // continue until something meaningful
        case SOP_NULL: break;

        // handle comments
        case SOP_COMMENT: {
          line.data = (void *) buffer;
          CALL_CALLBACK_IF(on_comment, &state, line);
          break;
        }

        // handle directives
        case SOP_DIRECTIVE_VERTEX_TEXTURE: {
          float vertex[4];
          sscanf(buffer, "%f %f %f %f",
              &vertex[0], &vertex[1], &vertex[2], &vertex[3]);
          line.data = vertex;
          CALL_CALLBACK_IF(on_texture, &state, line);
          break;
        }

        case SOP_DIRECTIVE_VERTEX_NORMAL: {
          float vertex[4];
          sscanf(buffer, "%f %f %f %f",
              &vertex[0], &vertex[1], &vertex[2], &vertex[3]);
          line.data = vertex;
          CALL_CALLBACK_IF(on_normal, &state, line);
          break;
        }

        case SOP_DIRECTIVE_VERTEX: {
          float vertex[4];
          sscanf(buffer, "%f %f %f %f",
              &vertex[0], &vertex[1], &vertex[2], &vertex[3]);
          line.data = vertex;
          CALL_CALLBACK_IF(on_vertex, &state, line);
          break;
        }

        case SOP_DIRECTIVE_FACE: {
          int maxfaces = 3;
          int faces[3][maxfaces];

          // vertex faces
          int *vf = faces[0];
          int x = 0;
          // vertex texture faces
          int *vtf = faces[1];
          int y = 0;
          // vertex normal faces
          int *vnf = faces[2];
          int z = 0;

          for (int i = 0; i < maxfaces; i++) {
            vf[i] = -1;
            vtf[i] = -1;
            vnf[i] = -1;
          }

          // current char buffer
          size_t size = 0;
          char buf[BUFSIZ];
          memset(buf, 0, BUFSIZ);

          // current face scope
          enum { READ = 0, VERTEX, TEXTURE, NORMAL };
          int scope = VERTEX;

          for (int j = 0; j < bufsize; ++j) {
            char c = buffer[j];

            // skip white space at beginning of line
            if (0 == size && ' ' == c) {
              continue;
            }

            if (j == bufsize - 1) {
              buf[size++] = c;
              goto read_face_data;
            }

            switch (c) {
              case '/':
              case '\t':
              case '\n':
              case ' ':
                goto read_face_data;

              default:
                buf[size++] = c;
                continue;
            }

read_face_data:
            switch (scope) {
              case VERTEX:
                sscanf(buf, "%d", &vf[x++]);
                break;

              case TEXTURE:
                sscanf(buf, "%d", &vtf[y++]);
                break;

              case NORMAL:
                sscanf(buf, "%d", &vnf[z++]);
                break;
            }

            switch (c) {
              case '/':
                scope++;
                break;

              case '\n':
              case '\t':
              case ' ':
                scope = VERTEX;
                break;
            }

            memset(buf, 0, BUFSIZ);
            size = 0;
          }

          line.data = faces;
          CALL_CALLBACK_IF(on_face, &state, line)
          break;
        }

        case SOP_DIRECTIVE_USE_MTL: {
          line.data = (void *) buffer;
          CALL_CALLBACK_IF(on_material_use, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MTL_LIB: {
          line.data = (void *) buffer;
          CALL_CALLBACK_IF(on_material_lib, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_NEW: {
          line.data = (void *) buffer;
          CALL_CALLBACK_IF(on_material_new, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_AMBIENT_COLOR: {
          float color[4];
          sscanf(buffer, "%f %f %f %f",
                 &color[0], &color[1], &color[2], &color[3]);
          line.data = color;
          CALL_CALLBACK_IF(on_material_ambient, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_DIFFUSE_COLOR: {
          float color[4];
          sscanf(buffer, "%f %f %f %f",
                 &color[0], &color[1], &color[2], &color[3]);
          line.data = color;
          CALL_CALLBACK_IF(on_material_diffuse, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_SPECULAR_COLOR: {
          float color[4];
          sscanf(buffer, "%f %f %f %f",
                 &color[0], &color[1], &color[2], &color[3]);
          line.data = color;
          CALL_CALLBACK_IF(on_material_specular, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_ILLUM: {
          unsigned int illum = 0;
          sscanf(buffer, "%d", &illum);
          line.data = &illum;
          CALL_CALLBACK_IF(on_material_illum, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_SHININESS: {
          unsigned int shininess = 0;
          sscanf(buffer, "%d", &shininess);
          line.data = &shininess;
          CALL_CALLBACK_IF(on_material_shininess, &state, line);
          break;
        }

        case SOP_DIRECTIVE_MATERIAL_TRANSPARENCY: {
          unsigned int transparency = 0;
          sscanf(buffer, "%d", &transparency);
          line.data = &transparency;
          CALL_CALLBACK_IF(on_material_transparency, &state, line);
          break;
        }

        case SOP_DIRECTIVE_SMOOTH: {
          int on = 1;
          int off = 0;
          if (strcmp(buffer, "on") >= 0) {
            line.data = (int *) &on;
          } else if (strcmp(buffer, "off") >= 0) {
            line.data = (int *) &off;
          } else {
            return SOP_OOB;
          }
          CALL_CALLBACK_IF(on_smooth, &state, line);
          break;
        }

        // notify of memory errors
        case SOP_EMEM:
          return SOP_EMEM;

        // out of bounds if we get here for some reason
        default:
          return SOP_OOB;
      }

      RESET_LINE_STATE;
      continue;
    }

    if (0 == colno) {
      // v, vt, vn
      if ('v' == ch0) {
        if (' '== ch1) {
          type = SOP_DIRECTIVE_VERTEX;
          line.directive = "v";
        } else if ('t' == ch1) {
          type = SOP_DIRECTIVE_VERTEX_TEXTURE;
          (void) i++;
          line.directive = "vt";
        } else if ('n' == ch1) {
          type = SOP_DIRECTIVE_VERTEX_NORMAL;
          (void) i++;
          line.directive = "vn";
        } else {
          type = SOP_NULL;
        }
      } else if ('f' == ch0) {
        type = SOP_DIRECTIVE_FACE;
        line.directive = "f";
      } else if ('#' == ch0) {
        type = SOP_COMMENT;
        line.directive = "#";
      } else if ('u' == ch0) {
        type = SOP_DIRECTIVE_USE_MTL;
        line.directive = "usemtl";
        i += 5; // 5 == strlen("usemtl") - 1;
      } else if ('m' == ch0) {
        type = SOP_DIRECTIVE_MTL_LIB;
        line.directive = "mtllib";
        i += 5; // 5 == strlen("mtllib") - 1;
      } else if ('n' == ch0) {
        type = SOP_DIRECTIVE_MATERIAL_NEW;
        line.directive = "newmtl";
        i += 5; // 5 == strlen("newmtl") - 1;
      } else if ('i' == ch0) {
        type = SOP_DIRECTIVE_MATERIAL_ILLUM;
        line.directive = "illum";
        i += 4; // 4 == strlen("illum") - 1;
      } else if ('d' == ch0 || ('T' == ch0 && 'r' == ch1)) {
        type = SOP_DIRECTIVE_MATERIAL_TRANSPARENCY;
        if ('d' == ch0) {
          line.directive = "d";
        } else {
          line.directive = "Tr";
          (void) i++;
        }
      } else if ('N' == ch0 && 's' == ch1) {
        type = SOP_DIRECTIVE_MATERIAL_SHININESS;
        line.directive = "Ns";
        (void) i++;
      } else if ('K' == ch0) {
        if ('a' == ch1) {
          type = SOP_DIRECTIVE_MATERIAL_AMBIENT_COLOR;
          line.directive = "Ka";
        } else if ('d' == ch1) {
          type = SOP_DIRECTIVE_MATERIAL_DIFFUSE_COLOR;
          line.directive = "Kd";
        } else if ('s' == ch1) {
          type = SOP_DIRECTIVE_MATERIAL_SPECULAR_COLOR;
          line.directive = "Ks";
        } else {
          type = SOP_NULL;
          continue;
        }

        (void) i++;
      } else if ('s' == ch0) {
        type = SOP_DIRECTIVE_SMOOTH;
        line.directive = "s";
      } else {
        switch (ch0) {
          case '\n':
            RESET_LINE_STATE;
            continue;
        }

        type = SOP_NULL;
      }
    } else {
      if (0 == bufsize && ' ' == ch0) {
        continue;
      }
      buffer[bufsize++] = ch0;
    }

    colno++;
  }
  return SOP_EOK;
#undef RESET_LINE_STATE
#undef CALL_CALLBACK_IF
}
