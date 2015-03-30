#version 410

uniform mat4 u_textureMatrix;

layout(std140) uniform MatrixBlock
{
  mat4 u_modelViewMatrix;
  mat4 u_modelViewProjectionMatrix;
  mat4 u_light_mvp[4];
  mat3 u_normalMatrix;
};

layout(location = 0) in vec4 a_vertex; 
layout(location = 2) in vec4 a_texCoord;
layout(location = 3) in vec4 a_color; 

out VertexData
{ 
  vec4 color;
  vec2 texCoord;
} vertex_out;

void main() 
{
  vertex_out.color = a_color;
  vertex_out.texCoord = (u_textureMatrix * a_texCoord).xy;
  gl_Position = u_modelViewProjectionMatrix * a_vertex; 
}
