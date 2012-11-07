#version 150 core

uniform int u_numTextures;
uniform sampler2D u_textureMap[];

uniform struct
{
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    vec4 emission;
    
} u_material;

in vec4 v_texCoord;
out vec4 fragData;

vec4 jet(in float val)
{
    return vec4(min(4.0 * val - 1.5, -4.0 * val + 4.5),
                min(4.0 * val - 0.5, -4.0 * val + 3.5),
                min(4.0 * val + 0.5, -4.0 * val + 2.5),
                1.0);
}

float gray(in vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec4 color = texture(u_textureMap[0], v_texCoord.xy);
    float confidence = texture(u_textureMap[1], v_texCoord.xy).r;
    
    //confidence = confidence > 0.25 ? confidence : 0.0 ;
    
    fragData = mix(color, jet(confidence), confidence) ;
}

