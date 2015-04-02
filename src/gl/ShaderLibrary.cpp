/* Generated file, do not edit! */

#include "ShaderLibrary.h"


char const* const gouraud_vert = 
   "#version 410\n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "};\n"
   "struct Lightsource\n"
   "{\n"
   "  vec3 position; \n"
   "  int type; \n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec3 spotDirection; \n"
   "  float spotCosCutoff; \n"
   "  float spotExponent; \n"
   "  float constantAttenuation; \n"
   "  float linearAttenuation; \n"
   "  float quadraticAttenuation; \n"
   "};\n"
   "vec4 shade(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec, in vec4 base_color) \n"
   "{\n"
   "  vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : -light.position;\n"
   "  vec3 L = normalize(lightDir); \n"
   "  vec3 E = normalize(-eyeVec); \n"
   "  vec3 R = reflect(-L, normal); \n"
   "  vec4 ambient = mat.ambient * light.ambient; \n"
   "  float att = 1.0; \n"
   "  float nDotL = dot(normal, L); \n"
   "  \n"
   "  if (light.type > 0) \n"
   "  {\n"
   "    float dist = length(lightDir); \n"
   "    att = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist); \n"
   "    if(light.type > 1) \n"
   "    {\n"
   "      float spotEffect = dot(normalize(light.spotDirection), -L); \n"
   "      if (spotEffect < light.spotCosCutoff) \n"
   "      {\n"
   "        att = 0.0;\n"
   "        base_color * ambient; \n"
   "      }\n"
   "      spotEffect = pow(spotEffect, light.spotExponent); \n"
   "      att *= spotEffect; \n"
   "    }\n"
   "  } \n"
   "  nDotL = max(0.0, nDotL); \n"
   "  float specIntesity = clamp(pow( max(dot(R, E), 0.0), mat.shinyness), 0.0, 1.0); \n"
   "  vec4 diffuse = att * mat.diffuse * light.diffuse * vec4(vec3(nDotL), 1.0); \n"
   "  vec4 spec = att * mat.specular * light.specular * specIntesity; spec.a = 0.0; \n"
   "  return base_color * (ambient + diffuse) + spec; \n"
   "}\n"
   "uniform mat4 u_modelViewMatrix; \n"
   "uniform mat4 u_modelViewProjectionMatrix; \n"
   "uniform mat3 u_normalMatrix; \n"
   "uniform mat4 u_textureMatrix;\n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "layout(std140) uniform LightBlock\n"
   "{\n"
   "  int u_numLights;\n"
   "  Lightsource u_lights[16];\n"
   "}; \n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 1) in vec3 a_normal; \n"
   "layout(location = 2) in vec4 a_texCoord; \n"
   "out VertexData \n"
   "{\n"
   "  vec4 color;\n"
   "  vec4 texCoord; \n"
   "} vertex_out;\n"
   "void main() \n"
   "{\n"
   "  vertex_out.texCoord = u_textureMatrix * a_texCoord;\n"
   "  vec3 normal = normalize(u_normalMatrix * a_normal); \n"
   "  vec3 eyeVec = (u_modelViewMatrix * a_vertex).xyz;\n"
   "  vec4 shade_color = vec4(0);\n"
   "  if(u_numLights > 0)\n"
   "    shade_color += shade(u_lights[0], u_material, normal, eyeVec, vec4(1)); \n"
   "  \n"
   "  if(u_numLights > 1)\n"
   "    shade_color += shade(u_lights[1], u_material, normal, eyeVec, vec4(1));\n"
   "  \n"
   "  vertex_out.color = shade_color; \n"
   "  gl_Position = u_modelViewProjectionMatrix * a_vertex; \n"
   "}\n"
;

char const* const phong_vert = 
   "#version 410\n"
   "uniform mat4 u_modelViewMatrix; \n"
   "uniform mat4 u_modelViewProjectionMatrix; \n"
   "uniform mat3 u_normalMatrix; \n"
   "uniform mat4 u_shadow_matrices[4];\n"
   "uniform mat4 u_textureMatrix;\n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 1) in vec3 a_normal; \n"
   "layout(location = 2) in vec4 a_texCoord; \n"
   "out VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec4 texCoord; \n"
   "  vec3 normal; \n"
   "  vec3 eyeVec;\n"
   "  vec4 lightspace_pos[4];\n"
   "} vertex_out; \n"
   "void main()\n"
   "{\n"
   "  vertex_out.normal = normalize(u_normalMatrix * a_normal); \n"
   "  vertex_out.texCoord = u_textureMatrix * a_texCoord; \n"
   "  vertex_out.eyeVec = (u_modelViewMatrix * a_vertex).xyz;\n"
   "  for(int i = 0; i < 4; i++)\n"
   "  {\n"
   "    vertex_out.lightspace_pos[i] = u_shadow_matrices[i] * a_vertex;\n"
   "  }\n"
   "  gl_Position = u_modelViewProjectionMatrix * a_vertex; \n"
   "}\n"
;

char const* const phong_normalmap_vert = 
   "#version 410\n"
   "uniform mat4 u_modelViewMatrix; \n"
   "uniform mat4 u_modelViewProjectionMatrix; \n"
   "uniform mat3 u_normalMatrix; \n"
   "uniform mat4 u_textureMatrix; \n"
   "struct Lightsource\n"
   "{\n"
   "  vec3 position; \n"
   "  int type; \n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec3 spotDirection; \n"
   "  float spotCosCutoff; \n"
   "  float spotExponent; \n"
   "  float constantAttenuation; \n"
   "  float linearAttenuation; \n"
   "  float quadraticAttenuation; \n"
   "}; \n"
   "layout(std140) uniform LightBlock\n"
   "{\n"
   "  int u_numLights;\n"
   "  Lightsource u_lights[16];\n"
   "};\n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 1) in vec3 a_normal; \n"
   "layout(location = 2) in vec4 a_texCoord; \n"
   "layout(location = 5) in vec3 a_tangent; \n"
   "out VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec4 texCoord; \n"
   "  vec3 eyeVec; \n"
   "  vec3 lightDir[16]; \n"
   "} vertex_out; \n"
   "void main()\n"
   "{\n"
   "  vec3 n = normalize(u_normalMatrix * a_normal); \n"
   "  vec3 t = normalize (u_normalMatrix * a_tangent); \n"
   "  vec3 b = cross(n, t); \n"
   "  mat3 tbnMatrix = mat3(t, b, n);\n"
   "  vec3 eye = (u_modelViewMatrix * a_vertex).xyz; \n"
   "  vertex_out.texCoord = u_textureMatrix * a_texCoord; \n"
   "  vertex_out.eyeVec = tbnMatrix * eye; \n"
   "  for(int i = 0; i < u_numLights; i++)\n"
   "  {\n"
   "    vertex_out.lightDir[i] = tbnMatrix * vec3(1, 0, 0);\n"
   "  }\n"
   "  gl_Position = u_modelViewProjectionMatrix * a_vertex; \n"
   "}\n"
;

char const* const phong_skin_vert = 
   "#version 410\n"
   "uniform mat4 u_modelViewMatrix; \n"
   "uniform mat4 u_modelViewProjectionMatrix; \n"
   "uniform mat3 u_normalMatrix; \n"
   "uniform mat4 u_shadow_matrices[4];\n"
   "uniform mat4 u_textureMatrix; \n"
   "uniform mat4 u_bones[110]; \n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 1) in vec3 a_normal; \n"
   "layout(location = 2) in vec4 a_texCoord; \n"
   "layout(location = 6) in ivec4 a_boneIds; \n"
   "layout(location = 7) in vec4 a_boneWeights; \n"
   "out VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec4 texCoord; \n"
   "  vec3 normal; \n"
   "  vec3 eyeVec;\n"
   "  vec4 lightspace_pos[4];\n"
   "} vertex_out; \n"
   "void main()\n"
   "{\n"
   "  vec4 newVertex = vec4(0); \n"
   "  vec4 newNormal = vec4(0); \n"
   "  \n"
   "  for (int i = 0; i < 4; i++)\n"
   "  {\n"
   "    newVertex += u_bones[a_boneIds[i]] * a_vertex * a_boneWeights[i]; \n"
   "    newNormal += u_bones[a_boneIds[i]] * vec4(a_normal, 0.0) * a_boneWeights[i]; \n"
   "  }\n"
   "  newVertex = vec4(newVertex.xyz, 1.0);\n"
   "  vertex_out.normal = normalize(u_normalMatrix * newNormal.xyz); \n"
   "  vertex_out.texCoord = u_textureMatrix * a_texCoord; \n"
   "  vertex_out.eyeVec = (u_modelViewMatrix * newVertex).xyz; \n"
   "  for(int i = 0; i < 4; i++)\n"
   "  {\n"
   "    vertex_out.lightspace_pos[i] = u_shadow_matrices[i] * newVertex;\n"
   "  }\n"
   "  gl_Position = u_modelViewProjectionMatrix * newVertex;\n"
   "}\n"
;

char const* const points_vert = 
   "#version 410\n"
   "uniform mat4 u_modelViewMatrix; \n"
   "uniform mat4 u_modelViewProjectionMatrix; \n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "}; \n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 3) in vec4 a_color; \n"
   "layout(location = 4) in float a_pointSize; \n"
   "out VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec3 eyeVec;\n"
   "  float point_size;\n"
   "} vertex_out;\n"
   "void main()\n"
   "{\n"
   "  vertex_out.color = a_color; \n"
   "  vertex_out.eyeVec = -(u_modelViewMatrix * a_vertex).xyz; \n"
   "  float d = length(vertex_out.eyeVec); \n"
   "  float attenuation = 1.0 / (u_material.point_vals[1] + u_material.point_vals[2] * d + u_material.point_vals[3] * (d * d)); \n"
   "  gl_PointSize = vertex_out.point_size = max(a_pointSize, u_material.point_vals[0]) * attenuation; \n"
   "  gl_Position = u_modelViewProjectionMatrix * a_vertex; \n"
   "}\n"
;

char const* const unlit_vert = 
   "#version 410\n"
   "uniform mat4 u_modelViewProjectionMatrix;\n"
   "uniform mat4 u_textureMatrix;\n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 2) in vec4 a_texCoord;\n"
   "layout(location = 3) in vec4 a_color; \n"
   "out VertexData\n"
   "{ \n"
   "  vec4 color;\n"
   "  vec2 texCoord;\n"
   "} vertex_out;\n"
   "void main() \n"
   "{\n"
   "  vertex_out.color = a_color;\n"
   "  vertex_out.texCoord = (u_textureMatrix * a_texCoord).xy;\n"
   "  gl_Position = u_modelViewProjectionMatrix * a_vertex; \n"
   "}\n"
;

char const* const unlit_rect_vert = 
   "#version 410\n"
   "uniform mat4 u_modelViewProjectionMatrix;\n"
   "uniform mat4 u_textureMatrix;\n"
   "uniform vec2 u_texture_size = vec2(1.0);\n"
   "layout(location = 0) in vec4 a_vertex; \n"
   "layout(location = 2) in vec4 a_texCoord;\n"
   "layout(location = 3) in vec4 a_color; \n"
   "out VertexData\n"
   "{ \n"
   "  vec4 color;\n"
   "  vec2 texCoord;\n"
   "} vertex_out;\n"
   "void main() \n"
   "{\n"
   "  vertex_out.color = a_color;\n"
   "  vertex_out.texCoord = (u_textureMatrix * a_texCoord).xy * u_texture_size;\n"
   "  gl_Position = u_modelViewProjectionMatrix * a_vertex; \n"
   "}\n"
;

char const* const lines_2D_geom = 
   "#version 330\n"
   "layout(lines) in;\n"
   "layout (triangle_strip, max_vertices = 4) out; \n"
   "uniform float u_line_thickness; \n"
   "uniform vec2 u_window_size; \n"
   "in VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec2 texCoord; \n"
   "} vertex_in[2]; \n"
   "out VertexData \n"
   "{ \n"
   "  vec4 color; \n"
   "  vec2 texCoord; \n"
   "} vertex_out; \n"
   "vec2 screen_space(vec4 vertex) \n"
   "{\n"
   "  return vertex.xy / vertex.w; \n"
   "} \n"
   "void main() \n"
   "{\n"
   "  vec2 p0 = screen_space(gl_in[0].gl_Position); \n"
   "  vec2 p1 = screen_space(gl_in[1].gl_Position); \n"
   "  vec2 v0 = normalize(p1 - p0); \n"
   "  vec2 n0 = vec2(-v0.y, v0.x); \n"
   "  vec2 bias = n0 * u_line_thickness / u_window_size; \n"
   "  vertex_out.color = vertex_in[0].color; \n"
   "  vertex_out.texCoord = vec2(0, 1); \n"
   "  gl_Position = vec4(p0 + bias , 0, 1); \n"
   "  EmitVertex(); \n"
   "  \n"
   "  vertex_out.color = vertex_in[0].color; \n"
   "  vertex_out.texCoord = vec2(0, 0); \n"
   "  gl_Position = vec4(p0 - bias, 0, 1); \n"
   "  EmitVertex(); \n"
   "  \n"
   "  vertex_out.color = vertex_in[1].color; \n"
   "  vertex_out.texCoord = vec2(0, 1); \n"
   "  gl_Position = vec4(p1 + bias, 0, 1); \n"
   "  EmitVertex(); \n"
   "  vertex_out.color = vertex_in[1].color; \n"
   "  vertex_out.texCoord = vec2(0, 0); \n"
   "  gl_Position = vec4(p1 - bias, 0, 1); \n"
   "  EmitVertex(); \n"
   "  EndPrimitive(); \n"
   "}\n"
;

char const* const gouraud_frag = 
   "#version 410\n"
   "uniform int u_numTextures;\n"
   "uniform sampler2D u_sampler_2D[1]; \n"
   "in VertexData \n"
   "{\n"
   "  vec4 color;\n"
   "  vec4 texCoord; \n"
   "} vertex_in; \n"
   "out vec4 fragData; \n"
   "void main() \n"
   "{\n"
   "  vec4 texColors = vec4(1); \n"
   "  \n"
   "  if(u_numTextures > 0)\n"
   "  {\n"
   "    texColors *= texture(u_sampler_2D[0], vertex_in.texCoord.st); \n"
   "  } \n"
   "  fragData = vertex_in.color * texColors; \n"
   "}\n"
;

char const* const phong_frag = 
   "#version 410\n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "};\n"
   "struct Lightsource\n"
   "{\n"
   "  vec3 position; \n"
   "  int type; \n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec3 spotDirection; \n"
   "  float spotCosCutoff; \n"
   "  float spotExponent; \n"
   "  float constantAttenuation; \n"
   "  float linearAttenuation; \n"
   "  float quadraticAttenuation; \n"
   "};\n"
   "vec3 projected_coords(in vec4 the_lightspace_pos)\n"
   "{\n"
   "    vec3 proj_coords = the_lightspace_pos.xyz / the_lightspace_pos.w;\n"
   "    proj_coords = (vec3(1) + proj_coords) * 0.5;\n"
   "    return proj_coords;\n"
   "}\n"
   "vec4 shade(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec, in vec4 base_color,\n"
   "           float shade_factor)\n"
   "{\n"
   "  vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : -light.position; \n"
   "  vec3 L = normalize(lightDir); \n"
   "  vec3 E = normalize(-eyeVec); \n"
   "  vec3 R = reflect(-L, normal); \n"
   "  vec4 ambient = mat.ambient * light.ambient; \n"
   "  float att = 1.0; \n"
   "  float nDotL = dot(normal, L); \n"
   "  \n"
   "  if (light.type > 0)\n"
   "  {\n"
   "    float dist = length(lightDir); \n"
   "    att = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist); \n"
   "    \n"
   "    if(light.type > 1)\n"
   "    {\n"
   "      float spotEffect = dot(normalize(light.spotDirection), -L); \n"
   "      \n"
   "      if (spotEffect < light.spotCosCutoff)\n"
   "      {\n"
   "        att = 0.0;\n"
   "        base_color * ambient; \n"
   "      }\n"
   "      spotEffect = pow(spotEffect, light.spotExponent); \n"
   "      att *= spotEffect; \n"
   "    }\n"
   "  }\n"
   "  nDotL = max(0.0, nDotL); \n"
   "  float specIntesity = clamp(pow( max(dot(R, E), 0.0), mat.shinyness), 0.0, 1.0); \n"
   "  vec4 diffuse = mat.diffuse * light.diffuse * vec4(att * shade_factor * vec3(nDotL), 1.0); \n"
   "  vec4 spec = shade_factor * att * mat.specular * light.specular * specIntesity; \n"
   "  spec.a = 0.0; \n"
   "  return base_color * (ambient + diffuse) + spec; \n"
   "}\n"
   "//uniform Material u_material;\n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "layout(std140) uniform LightBlock\n"
   "{\n"
   "  int u_numLights;\n"
   "  Lightsource u_lights[16];\n"
   "};\n"
   "uniform int u_numTextures;\n"
   "uniform sampler2D u_sampler_2D[4];\n"
   "#define EPSILON 0.00001\n"
   "uniform sampler2D u_shadow_map[4];\n"
   "uniform vec2 u_shadow_map_size = vec2(1024);\n"
   "in VertexData\n"
   "{\n"
   "  vec4 color;\n"
   "  vec4 texCoord;\n"
   "  vec3 normal; \n"
   "  vec3 eyeVec; \n"
   "  vec4 lightspace_pos[4];\n"
   "} vertex_in;\n"
   "out vec4 fragData; \n"
   "float nrand( vec2 n ) \n"
   "{\n"
   "	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);\n"
   "}\n"
   "vec2 rot2d( vec2 p, float a ) \n"
   "{\n"
   "	vec2 sc = vec2(sin(a),cos(a));\n"
   "	return vec2( dot( p, vec2(sc.y, -sc.x) ), dot( p, sc.xy ) );\n"
   "}\n"
   "const int NUM_TAPS = 12;\n"
   "uniform float u_poisson_radius = 5.0;// * (0.5+0.5*sin(iGlobalTime));\n"
   "vec2 fTaps_Poisson[NUM_TAPS];\n"
   "	\n"
   "float shadow_factor(int shadow_index)\n"
   "{\n"
   "	\n"
   "  vec3 proj_coords = projected_coords(vertex_in.lightspace_pos[shadow_index]);\n"
   "  float rnd = 6.28 * nrand(proj_coords.xy);\n"
   "  float factor = 0.0;\n"
   "	vec4 basis = vec4( rot2d(vec2(1,0),rnd), rot2d(vec2(0,1),rnd) );\n"
   "	for (int i = 0; i < NUM_TAPS; i++)\n"
   "	{\n"
   "		vec2 ofs = fTaps_Poisson[i]; ofs = vec2(dot(ofs,basis.xz),dot(ofs,basis.yw) );\n"
   "		//vec2 ofs = rot2d( fTaps_Poisson[i], rnd );\n"
   "		vec2 texcoord = proj_coords.xy + u_poisson_radius * ofs / u_shadow_map_size;\n"
   "    float depth = texture(u_shadow_map[shadow_index], texcoord).x;\n"
   "    bool is_in_shadow = depth < (proj_coords.z - EPSILON);\n"
   "    factor += is_in_shadow ? 0 : 1;\n"
   "  }\n"
   "  \n"
   "  //float xOffset = 1.0/u_shadow_map_size.x;\n"
   "  //float yOffset = 1.0/u_shadow_map_size.y;\n"
   "  //for(int y = -2 ; y <= 2 ; y++) \n"
   "  //{\n"
   "  //  for (int x = -2 ; x <= 2 ; x++) \n"
   "  //  {\n"
   "  //    vec2 offset = vec2(x * xOffset, y * yOffset);\n"
   "  //    float depth = texture(u_shadow_map[shadow_index], proj_coords.xy + offset).x;\n"
   "  //    bool is_in_shadow = depth < (proj_coords.z - EPSILON);\n"
   "  //    factor += is_in_shadow ? 0 : 1;\n"
   "  //  }\n"
   "  //}\n"
   "  //return (0.5 + (factor / 50.0));\n"
   "  return (0.5 + (factor / NUM_TAPS / 2.0));\n"
   "}\n"
   "void main() \n"
   "{\n"
   "	fTaps_Poisson[0]  = vec2(-.326,-.406);\n"
   "	fTaps_Poisson[1]  = vec2(-.840,-.074);\n"
   "	fTaps_Poisson[2]  = vec2(-.696, .457);\n"
   "	fTaps_Poisson[3]  = vec2(-.203, .621);\n"
   "	fTaps_Poisson[4]  = vec2( .962,-.195);\n"
   "	fTaps_Poisson[5]  = vec2( .473,-.480);\n"
   "	fTaps_Poisson[6]  = vec2( .519, .767);\n"
   "	fTaps_Poisson[7]  = vec2( .185,-.893);\n"
   "	fTaps_Poisson[8]  = vec2( .507, .064);\n"
   "	fTaps_Poisson[9]  = vec2( .896, .412);\n"
   "	fTaps_Poisson[10] = vec2(-.322,-.933);\n"
   "	fTaps_Poisson[11] = vec2(-.792,-.598);\n"
   "  vec4 texColors = vec4(1); \n"
   "  \n"
   "  for(int i = 0; i < u_numTextures; i++) \n"
   "    texColors *= texture(u_sampler_2D[i], vertex_in.texCoord.st); \n"
   "  \n"
   "  vec3 normal = normalize(vertex_in.normal); \n"
   "  vec4 shade_color = vec4(0); \n"
   "  \n"
   "  for(int i = 0; i < 2; i++)\n"
   "    shade_color += shade(u_lights[i], u_material, normal, vertex_in.eyeVec, texColors, shadow_factor(i));\n"
   "  fragData = shade_color; \n"
   "}\n"
;

char const* const phong_normalmap_frag = 
   "#version 410\n"
   "uniform int u_numTextures; \n"
   "uniform sampler2D u_sampler_2D[4]; \n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "};\n"
   "struct Lightsource\n"
   "{\n"
   "  vec3 position; \n"
   "  int type; \n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec3 spotDirection; \n"
   "  float spotCosCutoff; \n"
   "  float spotExponent; \n"
   "  float constantAttenuation; \n"
   "  float linearAttenuation; \n"
   "  float quadraticAttenuation; \n"
   "}; \n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "layout(std140) uniform LightBlock\n"
   "{\n"
   "  int u_numLights;\n"
   "  Lightsource u_lights[16];\n"
   "};\n"
   "vec4 shade(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec, in vec3 lightDir,\n"
   "    in vec4 base_color)\n"
   "{\n"
   "//  vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : -light.position; \n"
   "  vec3 L = normalize(lightDir); \n"
   "  vec3 E = normalize(-eyeVec); \n"
   "  vec3 R = reflect(-L, normal); \n"
   "  vec4 ambient = mat.ambient * light.ambient; \n"
   "  float att = 1.0; \n"
   "  float nDotL = dot(normal, L); \n"
   "  \n"
   "  if (light.type > 0)\n"
   "  {\n"
   "    float dist = length(lightDir); \n"
   "    att = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist); \n"
   "    \n"
   "    if(light.type > 1)\n"
   "    {\n"
   "      float spotEffect = dot(normalize(light.spotDirection), -L); \n"
   "      \n"
   "      if (spotEffect < light.spotCosCutoff)\n"
   "      {\n"
   "        att = 0.0;\n"
   "        base_color * ambient; \n"
   "      }\n"
   "      spotEffect = pow(spotEffect, light.spotExponent); \n"
   "      att *= spotEffect; \n"
   "    }\n"
   "  }\n"
   "  nDotL = max(0.0, nDotL); \n"
   "  float specIntesity = clamp(pow( max(dot(R, E), 0.0), mat.shinyness), 0.0, 1.0); \n"
   "  vec4 diffuse = att * mat.diffuse * light.diffuse * vec4(vec3(nDotL), 1.0); \n"
   "  vec4 spec = att * mat.specular * light.specular * specIntesity; \n"
   "  spec.a = 0.0; \n"
   "  return base_color * (ambient + diffuse) + spec; \n"
   "}\n"
   "in VertexData\n"
   "{ \n"
   "  vec4 color; \n"
   "  vec4 texCoord; \n"
   "  vec3 eyeVec; \n"
   "  vec3 lightDir[16]; \n"
   "} vertex_in; \n"
   "out vec4 fragData; \n"
   "vec3 normalFromHeightMap(sampler2D theMap, vec2 theCoords, float theStrength) \n"
   "{\n"
   "  float center = texture(theMap, theCoords).r;\n"
   "  float U = texture(theMap, theCoords + vec2( 0.005, 0)).r;\n"
   "  float V = texture(theMap, theCoords + vec2(0, 0.005)).r;\n"
   "  float dHdU = U - center; \n"
   "  float dHdV = V - center; \n"
   "  vec3 normal = vec3( -dHdU, dHdV, 0.05 / theStrength); \n"
   "  return normalize(normal);\n"
   "}\n"
   "void main()\n"
   "{\n"
   "  vec4 texColors = vec4(1);//vertex_in.color;//texture(u_sampler_2D[0], vertex_in.texCoord.xy); \n"
   "  vec3 normal; \n"
   "  //normal = normalFromHeightMap(u_sampler_2D[1], vertex_in.texCoord.xy, 0.8); \n"
   "  normal = normalize(2 * (texture(u_sampler_2D[1], vertex_in.texCoord.xy).xyz - vec3(0.5))); \n"
   "  vec4 shade_color = vec4(0); \n"
   "  \n"
   "  for(int i = 0; i < u_numLights; i++)\n"
   "  {\n"
   "    shade_color += shade(u_lights[i], u_material, normal, vertex_in.eyeVec,\n"
   "        vertex_in.lightDir[i], texColors);\n"
   "  }\n"
   "  fragData = shade_color; \n"
   "}\n"
;

char const* const points_frag = 
   "#version 410\n"
   "uniform int u_numTextures;\n"
   "uniform sampler2D u_sampler_2D[1]; \n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "}; \n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "in VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec3 eyeVec;\n"
   "  float point_size;\n"
   "} vertex_in; \n"
   "out vec4 fragData; \n"
   "void main() \n"
   "{\n"
   "  vec4 texColors = vertex_in.color; \n"
   "  \n"
   "  if(u_numTextures > 0)\n"
   "  {\n"
   "    texColors *= texture(u_sampler_2D[0], gl_PointCoord.xy); \n"
   "  } \n"
   "  fragData = u_material.diffuse * texColors; \n"
   "}\n"
;

char const* const points_sphere_frag = 
   "#version 410\n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "};\n"
   "struct Lightsource\n"
   "{\n"
   "  vec3 position; \n"
   "  int type; \n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec3 spotDirection; \n"
   "  float spotCosCutoff; \n"
   "  float spotExponent; \n"
   "  float constantAttenuation; \n"
   "  float linearAttenuation; \n"
   "  float quadraticAttenuation; \n"
   "};\n"
   "vec4 shade(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec, in vec4 base_color)\n"
   "{\n"
   "  vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : -light.position; \n"
   "  vec3 L = normalize(lightDir); \n"
   "  vec3 E = normalize(-eyeVec); \n"
   "  vec3 R = reflect(-L, normal); \n"
   "  vec4 ambient = mat.ambient * light.ambient; \n"
   "  float att = 1.0; \n"
   "  float nDotL = dot(normal, L); \n"
   "  \n"
   "  if (light.type > 0)\n"
   "  {\n"
   "    float dist = length(lightDir); \n"
   "    att = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist); \n"
   "    \n"
   "    if(light.type > 1)\n"
   "    {\n"
   "      float spotEffect = dot(normalize(light.spotDirection), -L); \n"
   "      \n"
   "      if (spotEffect < light.spotCosCutoff)\n"
   "      {\n"
   "        att = 0.0;\n"
   "        base_color * ambient; \n"
   "      }\n"
   "      spotEffect = pow(spotEffect, light.spotExponent); \n"
   "      att *= spotEffect; \n"
   "    }\n"
   "  }\n"
   "  nDotL = max(0.0, nDotL); \n"
   "  float specIntesity = clamp(pow( max(dot(R, E), 0.0), mat.shinyness), 0.0, 1.0); \n"
   "  vec4 diffuse = att * mat.diffuse * light.diffuse * vec4(vec3(nDotL), 1.0); \n"
   "  vec4 spec = att * mat.specular * light.specular * specIntesity; \n"
   "  spec.a = 0.0; \n"
   "  return base_color * (ambient + diffuse) + spec; \n"
   "}\n"
   "uniform float u_pointRadius; \n"
   "uniform vec3 u_lightDir; \n"
   "uniform int u_numTextures; \n"
   "uniform sampler2D u_sampler_2D[8];\n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "layout(std140) uniform LightBlock\n"
   "{\n"
   "  int u_numLights;\n"
   "  Lightsource u_lights[16];\n"
   "}; \n"
   "in VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec3 eyeVec;\n"
   "  float point_size;\n"
   "} vertex_in; \n"
   "out vec4 fragData; \n"
   "void main() \n"
   "{\n"
   "  vec4 texColors = vertex_in.color; \n"
   "  \n"
   "  for(int i = 0; i < u_numTextures; i++) \n"
   "  { \n"
   "    texColors *= texture(u_sampler_2D[i], gl_PointCoord); \n"
   "  }\n"
   "  vec3 normal; \n"
   "  normal.xy = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0); \n"
   "  float mag = dot(normal.xy, normal.xy); \n"
   "  \n"
   "  if (mag > 1.0) discard; \n"
   "  \n"
   "  normal.z = sqrt(1.0 - mag); \n"
   "  vec3 spherePosEye = -(vertex_in.eyeVec + normal * vertex_in.point_size / 2.0); \n"
   "  vec4 shade_color = vec4(0); \n"
   "  \n"
   "  if(u_numLights > 0) \n"
   "    shade_color += shade(u_lights[0], u_material, normal, spherePosEye, texColors); \n"
   "  \n"
   "  if(u_numLights > 1)\n"
   "    shade_color += shade(u_lights[1], u_material, normal, spherePosEye, texColors);\n"
   "  if(u_numLights > 2) \n"
   "    shade_color += shade(u_lights[2], u_material, normal, spherePosEye, texColors); \n"
   "  \n"
   "  if(u_numLights > 3)\n"
   "    shade_color += shade(u_lights[3], u_material, normal, spherePosEye, texColors);\n"
   "  fragData = shade_color;//texColors * (u_material.diffuse * vec4(vec3(nDotL), 1.0)) + spec; \n"
   "}\n"
;

char const* const unlit_frag = 
   "#version 410\n"
   "uniform int u_numTextures;\n"
   "uniform sampler2D u_sampler_2D[1];\n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "};\n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "in VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec2 texCoord;\n"
   "} vertex_in; \n"
   "out vec4 fragData;\n"
   "void main() \n"
   "{\n"
   "  vec4 texColors = vertex_in.color;\n"
   "  if(u_numTextures > 0) \n"
   "    texColors *= texture(u_sampler_2D[0], vertex_in.texCoord.st); \n"
   "  fragData = u_material.diffuse * texColors; \n"
   "}\n"
;

char const* const unlit_rect_frag = 
   "#version 410\n"
   "uniform int u_numTextures;\n"
   "uniform sampler2DRect u_sampler_2Drect[1];\n"
   "struct Material\n"
   "{\n"
   "  vec4 diffuse; \n"
   "  vec4 ambient; \n"
   "  vec4 specular; \n"
   "  vec4 emission; \n"
   "  vec4 point_vals;// (size, constant_att, linear_att, quad_att) \n"
   "  float shinyness;\n"
   "};\n"
   "layout(std140) uniform MaterialBlock\n"
   "{\n"
   "  Material u_material;\n"
   "};\n"
   "in VertexData\n"
   "{\n"
   "  vec4 color; \n"
   "  vec2 texCoord;\n"
   "} vertex_in; \n"
   "out vec4 fragData;\n"
   "void main() \n"
   "{\n"
   "  vec4 texColors = vertex_in.color;\n"
   "  if(u_numTextures > 0) \n"
   "    texColors *= texture(u_sampler_2Drect[0], vertex_in.texCoord.st); \n"
   "  fragData = u_material.diffuse * texColors; \n"
   "}\n"
;
