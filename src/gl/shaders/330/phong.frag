#version 410

struct Material
{
  vec4 diffuse; 
  vec4 ambient; 
  vec4 specular; 
  vec4 emission; 
  float shinyness;
  float point_size; 
  struct
  {
    float constant; 
    float linear; 
    float quadratic; 
  } point_attenuation;
};

struct Lightsource
{
  vec3 position; 
  vec4 diffuse; 
  vec4 ambient; 
  vec4 specular; 
  vec3 spotDirection; 
  float spotCosCutoff; 
  float spotExponent; 
  float constantAttenuation; 
  float linearAttenuation; 
  float quadraticAttenuation; 
  int type; 
};

vec4 shade(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec, in vec4 base_color)
{
  vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : -light.position; 
  vec3 L = normalize(lightDir); 
  vec3 E = normalize(-eyeVec); 
  vec3 R = reflect(-L, normal); 
  vec4 ambient = mat.ambient * light.ambient; 
  float att = 1.0; 
  float nDotL = dot(normal, L); 
  
  if (light.type > 0)
  {
    float dist = length(lightDir); 
    att = 1.0 / (light.constantAttenuation + light.linearAttenuation * dist + light.quadraticAttenuation * dist * dist); 
    
    if(light.type > 1)
    {
      float spotEffect = dot(normalize(light.spotDirection), -L); 
      
      if (spotEffect < light.spotCosCutoff)
      {
        att = 0.0;
        base_color * ambient; 
      }
      spotEffect = pow(spotEffect, light.spotExponent); 
      att *= spotEffect; 
    }
  }
  nDotL = max(0.0, nDotL); 
  float specIntesity = clamp(pow( max(dot(R, E), 0.0), mat.shinyness), 0.0, 1.0); 
  vec4 diffuse = att * mat.diffuse * light.diffuse * vec4(vec3(nDotL), 1.0); 
  vec4 spec = att * mat.specular * light.specular * specIntesity; 
  spec.a = 0.0; 
  return base_color * (ambient + diffuse) + spec; 
}

//uniform Material u_material;
layout(std140) uniform MaterialBlock
{
  Material u_material;
};

layout(std140) uniform LightBlock
{
  int u_numLights;
  Lightsource u_lights[16];
};

uniform int u_numTextures; 
uniform sampler2D u_sampler_2D[4]; 

in VertexData
{
  vec4 color;
  vec4 texCoord;
  vec3 normal; 
  vec3 eyeVec; 
} vertex_in;

out vec4 fragData; 

void main() 
{
  vec4 texColors = vec4(1); 
  
  for(int i = 0; i < u_numTextures; i++) 
  {
    texColors *= texture(u_sampler_2D[i], vertex_in.texCoord.st); 
  } 
  vec3 normal = normalize(vertex_in.normal); 
  vec4 shade_color = vec4(0); 
  
  if(u_numLights > 0) 
    shade_color += shade(u_lights[0], u_material, normal, vertex_in.eyeVec, texColors); 
  
  if(u_numLights > 1)
    shade_color += shade(u_lights[1], u_material, normal, vertex_in.eyeVec, texColors); 

  if(u_numLights > 2)
    shade_color += shade(u_lights[2], u_material, normal, vertex_in.eyeVec, texColors); 
  
  if(u_numLights > 3) shade_color += shade(u_lights[3], u_material, normal, vertex_in.eyeVec, texColors); 
  
  fragData = shade_color; 
}
