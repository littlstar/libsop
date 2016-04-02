#version 400

precision mediump float;
uniform vec3 uColor;
out vec4 fragColor;

void
main(void) {
  fragColor = vec4(uColor, 1);
}
