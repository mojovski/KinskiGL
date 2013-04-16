#version 150 core
uniform int u_numTextures;
uniform sampler2D u_textureMap[16];
uniform struct
{
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    vec4 emission;
    float shinyness;
} u_material;
in vec3 v_normal;
in vec4 v_texCoord;
in vec3 v_eyeVec;
in vec3 v_lightDir;
out vec4 fragData;
vec3 normalFromHeightMap(sampler2D theMap, vec2 theCoords, float theStrength)
{
    float center = texture(theMap, theCoords).r ;	 //center bump map sample
    float U = textureOffset(theMap, theCoords, ivec2(-1, 0)).r ;	//U bump map sample
    float V = textureOffset(theMap, theCoords, ivec2(0, -1)).r ;	 //V bump map sample
    float dHdU = U - center;	 //create bump map U offset
    float dHdV = V - center;	 //create bump map V offset
    vec3 normal = vec3( -dHdU, dHdV, 0.05 / theStrength);	 //create the tangent space normal
    return normalize(normal);
}

void main()
{
    // sample color map
    vec4 texColors = texture(u_textureMap[0], v_texCoord.xy);
    vec4 night_color = u_material.ambient + u_material.diffuse * texture(u_textureMap[3], v_texCoord.xy);
    
    vec3 N = vec3(0, 0, 1);
    // sample normal map
    //N = normalize(texture(u_textureMap[1], v_texCoord.xy).xyz * 2.0 - 1.0);
    // sample bump map
    //N = normalFromHeightMap(u_textureMap[1], v_texCoord.xy, 0.3);
    vec3 L = normalize(-v_lightDir);
    vec3 E = normalize(v_eyeVec);
    vec3 R = reflect(-L, N);
    
    //Lambert term
    float nDotL = max(0.0, dot(N, L));

    float specIntesity = pow( max(dot(R, E), 0.0), u_material.shinyness);
    // sample spec map
    specIntesity *= (1 - texture(u_textureMap[2], v_texCoord.xy).x);
    
    vec4 spec = u_material.specular * specIntesity; spec.a = 0.0;
    vec4 day_color = texColors * (u_material.ambient + u_material.diffuse * vec4(vec3(nDotL), 1.0)) + spec;
    
    fragData = mix(night_color, day_color, nDotL);
}