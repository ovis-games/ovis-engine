uniform mat4 u_Projection;

in vec2 a_Position;
in vec2 a_TextureCoordinates;
in vec4 a_Color;

out vec4 vs_Color;
out vec2 vs_TextureCoordinates;

void main() {
  gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
  vs_Color = a_Color;
  vs_TextureCoordinates = a_TextureCoordinates;
}