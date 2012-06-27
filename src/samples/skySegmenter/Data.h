#ifndef _DATA_H_IS_INCLUDED
#define _DATA_H_IS_INCLUDED

#include "kinskiGL/KinskiGL.h"

const char *g_vertShaderSrc = 
"#version 150 core\n\
\n\
uniform mat4 u_modelViewProjectionMatrix;\n\
uniform mat3 u_normalMatrix;\n\
uniform mat4 u_textureMatrix;\n\
\n\
in vec4 a_position;\n\
in vec3 a_normal;\n\
in vec4 a_texCoord;\n\
\n\
out vec4 v_texCoord;\n\
\n\
void main()\n\
{\n\
vec3 eyeNormal = normalize(u_normalMatrix * a_normal);\n\
\n\
v_texCoord =  u_textureMatrix * a_texCoord;\n\
\n\
gl_Position = u_modelViewProjectionMatrix * a_position;\n\
}";

const char *g_fragShaderSrc =
"#version 150 core\n\
\n\
uniform sampler2D   u_textureMap;\n\
uniform vec4   u_lightColor;\n\
in vec4        v_texCoord;\n\
\n\
out vec4 fragData;\n\
\n\
void main()\n\
{\n\
vec4 tex = texture(u_textureMap, v_texCoord.xy);\n\
vec4 invert = vec4(vec3(1.0) - tex.rgb, tex.a);\n\
\n\
fragData = tex ;\n\
\
}";

#endif //_DATA_H_IS_INCLUDED