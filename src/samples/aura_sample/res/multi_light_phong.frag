#version 150 core

uniform int u_numTextures;
uniform int u_numLights;
uniform sampler2D u_textureMap[16];
uniform struct Lightsource
{
    // 0: Directional
    // 1: Point
    // 2: Spot
    int type;
    // position in eyecoords
    vec3 position;
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    // attenuation
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
} u_lights[8];

uniform struct Material
{
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    vec4 emission;
    float shinyness;
} u_material;
                                
in VertexData{
    vec4 color;
    vec4 texCoord;
    vec3 normal;
    vec3 eyeVec;
} vertex_in;
out vec4 fragData;
                                
vec4 shade_phong(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec,
                 in vec4 base_color)
{
    vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : light.position;
    vec3 L = normalize(lightDir);
    vec3 E = normalize(-eyeVec);
    vec3 R = reflect(-L, normal);
    
    float att = 1.0;
    float nDotL = dot(normal, L);
    if (light.type > 0 && nDotL > 0.0)
    {
        float dist = length(lightDir);
        att = 1.0 / (light.constantAttenuation +
                     light.linearAttenuation * dist +
                     light.quadraticAttenuation * dist * dist);
    }
    nDotL = max(0.0, nDotL);
    
    float specIntesity = pow( max(dot(R, E), 0.0), mat.shinyness);
    vec4 ambient = mat.ambient * light.ambient;
    vec4 diffuse = mat.diffuse * light.diffuse;
    vec4 spec = mat.specular * light.specular * specIntesity; spec.a = 0.0;
    return base_color * att * (ambient + diffuse * vec4(vec3(nDotL), 1.0)) + spec;
}
                                
void main()
{
    // accumulate all texture maps
    vec4 texColors = vec4(1);
    for(int i = 0; i < u_numTextures; i++)
    {
        texColors *= texture(u_textureMap[i], vertex_in.texCoord.st);
    }
    vec3 N = normalize(vertex_in.normal);
    
    // calculate shading for all lights
    vec4 shade_color = vec4(0);
    for(int i = 0; i < u_numLights; i++)
    {
        shade_color += shade_phong(u_lights[i], u_material, N, vertex_in.eyeVec, texColors);
    }
    fragData = shade_color;
}