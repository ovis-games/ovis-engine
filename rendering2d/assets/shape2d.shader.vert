in vec2 a_Position;
in vec4 a_Color;

out vec4 vs_Color;

void main() {
  gl_Position = vec4(a_Position, 0.0, 1.0);
  vs_Color = a_Color;
}
