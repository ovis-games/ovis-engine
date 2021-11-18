uniform mat4 u_ScreenToClipSpace;

in vec3 a_Position;
in vec4 a_Color;

out vec4 vs_Color;

void main() {
  gl_Position = vec4(a_Position, 1.0) * u_ScreenToClipSpace;
  vs_Color = a_Color;
}