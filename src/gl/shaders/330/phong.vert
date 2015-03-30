#version 410

layout(std140) uniform MatrixBlock
{
  mat4 u_modelViewMatrix;
  mat4 u_modelViewProjectionMatrix;
  mat4 u_light_mvp[4];
  mat3 u_normalMatrix;
}; 
uniform mat4 u_textureMatrix; 

layout(location = 0) in vec4 a_vertex; 
layout(location = 1) in vec3 a_normal; 
layout(location = 2) in vec4 a_texCoord; 

out VertexData
{
  vec4 color; 
  vec4 texCoord; 
  vec3 normal; 
  vec3 eyeVec; 
} vertex_out; 

void main()
{
  vertex_out.normal = normalize(u_normalMatrix * a_normal); 
  vertex_out.texCoord = u_textureMatrix * a_texCoord; 
  vertex_out.eyeVec = (u_modelViewMatrix * a_vertex).xyz; 
  gl_Position = u_modelViewProjectionMatrix * a_vertex; 
}
