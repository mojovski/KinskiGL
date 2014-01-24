// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2013, Fabian Schmidt <crocdialer@googlemail.com>
//
// It is distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __

#include "KinskiGL.h"
#include "Shader.h"

#define STRINGIFY(A) #A
#define GLSL(version, shader)  "#version " #version "\n" #shader

namespace kinski { namespace gl {
    
    Shader createShaderFromFile(const std::string &vertPath,
                                const std::string &fragPath,
                                const std::string &geomPath)
    {
        Shader ret;
        std::string vertSrc, fragSrc, geomSrc;
        vertSrc = readFile(vertPath);
        fragSrc = readFile(fragPath);
        
        if (!geomPath.empty()) geomSrc = readFile(geomPath);
        
        try {
            ret.loadFromData(vertSrc.c_str(), fragSrc.c_str(), geomSrc.empty() ? NULL : geomSrc.c_str());
        }
        catch (Exception &e)
        {
            LOG_ERROR<<e.what();
        }
        return ret;
    }
    
    Shader createShader(ShaderType type)
    {
        const char *phong_normalmap_vertSrc, *phong_normalmap_fragSrc;
        const char *point_color_fragSrc, *point_texture_fragSrc, *point_sphere_fragSrc;
        
        std::string glsl_header = "#version 150 core\n";
        std::string glsl_define_explicit_layout = "#define GL_ARB_explicit_attrib_location 1\n";
        
        std::string material_block = STRINGIFY(
        struct Material\n
        {
          vec4 diffuse;
          vec4 ambient;
          vec4 specular;
          vec4 emission;
          float shinyness;
        };);
        
        std::string light_block = STRINGIFY(
        uniform int u_numLights;
        struct Lightsource
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
            // spot params
            vec3 spotDirection;
            float spotCosCutoff;
            float spotExponent;
        };);
        
        std::string shade_function_block = STRINGIFY(
        vec4 shade(in Lightsource light, in Material mat, in vec3 normal, in vec3 eyeVec,
                   in vec4 base_color)
        {
          vec3 lightDir = light.type > 0 ? (light.position - eyeVec) : -light.position;
          vec3 L = normalize(lightDir);
          vec3 E = normalize(-eyeVec);
          vec3 R = reflect(-L, normal);
          
          // ambient term
          vec4 ambient = mat.ambient * light.ambient;
            
          float att = 1.0;
          float nDotL = dot(normal, L);
          // point + spot
          if (light.type > 0)
          {
              float dist = length(lightDir);
              att = 1.0 / (light.constantAttenuation +
                           light.linearAttenuation * dist +
                           light.quadraticAttenuation * dist * dist);
              
              // spot
              if(light.type > 1)
              {
                  float spotEffect = dot(normalize(light.spotDirection), -L);
                  if (spotEffect < light.spotCosCutoff)
                  {
                      att = 0.0;//return vec4(0, 0, 0, 1);
                      base_color * ambient;
                  }
                  
                  spotEffect = pow(spotEffect, light.spotExponent);
                  att *= spotEffect;
              }
          }
          nDotL = max(0.0, nDotL);
          
          float specIntesity = clamp(pow( max(dot(R, E), 0.0), mat.shinyness), 0.0, 1.0);
          vec4 diffuse = att * mat.diffuse * light.diffuse * vec4(vec3(nDotL), 1.0);
          vec4 spec = att * mat.specular * light.specular * specIntesity; spec.a = 0.0;
          return base_color * (ambient + diffuse) + spec;
        });
        
        std::string vertex_shader_phong = STRINGIFY(
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat3 u_normalMatrix;
        uniform mat4 u_textureMatrix;
        
        in vec4 a_vertex;
        in vec3 a_normal;
        in vec4 a_texCoord;
        
        out VertexData{
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
        });
        
        std::string frag_shader_phong = STRINGIFY(
        uniform Material u_material;
        uniform Lightsource u_lights[16];
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[16];
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
            // accumulate all texture maps
            vec4 texColors = vec4(1);
            for(int i = 0; i < u_numTextures; i++)
            {
                texColors *= texture(u_textureMap[i], vertex_in.texCoord.st);
            }
            vec3 normal = normalize(vertex_in.normal);
         
            // calculate shading for all lights
            vec4 shade_color = vec4(0);
            //for(int i = 0; i < u_numLights; i++)// loop causes trouble on nvidia osx 10.8
            
            if(u_numLights > 0)
                shade_color += shade(u_lights[0], u_material, normal, vertex_in.eyeVec, texColors);
            
            if(u_numLights > 1)
                shade_color += shade(u_lights[1], u_material, normal, vertex_in.eyeVec, texColors);

            if(u_numLights > 2)
                shade_color += shade(u_lights[2], u_material, normal, vertex_in.eyeVec, texColors);

            if(u_numLights > 3)
                shade_color += shade(u_lights[3], u_material, normal, vertex_in.eyeVec, texColors);
            
            fragData = shade_color;
        });
        
        std::string vertex_shader_gouraud = STRINGIFY(
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat3 u_normalMatrix;
        uniform mat4 u_textureMatrix;
        uniform Material u_material;
        uniform Lightsource u_lights[16];
                                                  
        in vec4 a_vertex;
        in vec3 a_normal;
        in vec4 a_texCoord;
        
        out VertexData
        {
            vec4 color;
            vec4 texCoord;
        } vertex_out;
        
        void main()
        {
            vertex_out.texCoord = u_textureMatrix * a_texCoord;
            vec3 normal = normalize(u_normalMatrix * a_normal);
            vec3 eyeVec = (u_modelViewMatrix * a_vertex).xyz;
            vec4 shade_color = vec4(0);
            //for(int i = 0; i < u_numLights; i++)
            {
                shade_color += shade(u_lights[0], u_material, normal, eyeVec, vec4(1));
                shade_color += shade(u_lights[1], u_material, normal, eyeVec, vec4(1));
            }
            vertex_out.color = shade_color;
            gl_Position = u_modelViewProjectionMatrix * a_vertex;
        });
        
        std::string frag_shader_gouraud = STRINGIFY(
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[16];

        in VertexData
        {
            vec4 color;
            vec4 texCoord;
        } vertex_in;

        out vec4 fragData;
        void main()
        {
          // accumulate all texture maps
          vec4 texColors = vec4(1);
          for(int i = 0; i < u_numTextures; i++)
          {
              texColors *= texture(u_textureMap[i], vertex_in.texCoord.st);
          }
          fragData = vertex_in.color * texColors;
        });
        
#ifdef KINSKI_GLES
        const char *unlitVertSrc = STRINGIFY(
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat4 u_textureMatrix;
        attribute vec4 a_vertex;
        attribute vec4 a_texCoord;
        attribute vec4 a_color;//
        varying lowp vec4 v_color;//
        varying lowp vec4 v_texCoord;
        void main()
        {
            v_color = a_color;
            v_texCoord =  u_textureMatrix * a_texCoord;
            gl_Position = u_modelViewProjectionMatrix * a_vertex;
        });
        
        const char *unlitFragSrc = STRINGIFY(
        precision mediump float;
        precision lowp int;
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[8];
        uniform struct
        {
            vec4 diffuse;
            vec4 ambient;
            vec4 specular;
            vec4 emission;
            float shinyness;
        } u_material;
        varying vec4 v_color;//
        varying vec4 v_texCoord;
        void main()
        {
            vec4 texColors = v_color;//
            if(u_numTextures > 0) texColors *= texture2D(u_textureMap[0], v_texCoord.st);
            if(u_numTextures > 1) texColors *= texture2D(u_textureMap[1], v_texCoord.st);
            if(u_numTextures > 2) texColors *= texture2D(u_textureMap[2], v_texCoord.st);
            if(u_numTextures > 3) texColors *= texture2D(u_textureMap[3], v_texCoord.st);
            gl_FragColor = u_material.diffuse * texColors;
        });
        
        const char *phongVertSrc = STRINGIFY(
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat3 u_normalMatrix;
        uniform mat4 u_textureMatrix;
        attribute vec4 a_vertex;
        attribute vec4 a_texCoord;
        attribute vec3 a_normal;
        varying lowp vec4 v_texCoord;
        varying mediump vec3 v_normal;
        varying mediump vec3 v_eyeVec;
        void main()
        {
            v_normal = normalize(u_normalMatrix * a_normal);
            v_texCoord =  u_textureMatrix * a_texCoord;
            v_eyeVec = - (u_modelViewMatrix * a_vertex).xyz;
            gl_Position = u_modelViewProjectionMatrix * a_vertex;
        });
        
        const char *phongVertSrc_skin = STRINGIFY(
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat3 u_normalMatrix;
        uniform mat4 u_textureMatrix;
        uniform mat4 u_bones[18];
        attribute vec4 a_vertex;
        attribute vec4 a_texCoord;
        attribute vec3 a_normal;
        attribute vec4 a_boneIds;
        attribute vec4 a_boneWeights;
        varying vec4 v_texCoord;
        varying vec3 v_normal;
        varying vec3 v_eyeVec;
        void main()
        {
            vec4 newVertex = vec4(0.0);
            vec4 newNormal = vec4(0.0);
            for (int i = 0; i < 4; i++)
            {
                newVertex += u_bones[int(floor(a_boneIds[i]))] * a_vertex * a_boneWeights[i];
                newNormal += u_bones[int(floor(a_boneIds[i]))] * vec4(a_normal, 0.0) * a_boneWeights[i];
            }
            v_normal = normalize(u_normalMatrix * newNormal.xyz);
            v_texCoord =  u_textureMatrix * a_texCoord;
            v_eyeVec = - (u_modelViewMatrix * newVertex).xyz;
            gl_Position = u_modelViewProjectionMatrix * vec4(newVertex.xyz, 1.0);
        });
        
        const char *phongFragSrc = STRINGIFY(
        precision mediump float;
        precision lowp int;
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[8];
        uniform vec3 u_lightDir;
        uniform struct
        {
            vec4 diffuse;
            vec4 ambient;
            vec4 specular;
            vec4 emission;
            float shinyness;
        } u_material;
        varying vec3 v_normal;
        varying vec4 v_texCoord;
        varying vec3 v_eyeVec;
        void main()
        {
            vec4 texColors = vec4(1);
            if(u_numTextures > 0) texColors *= texture2D(u_textureMap[0], v_texCoord.st);
            if(u_numTextures > 1) texColors *= texture2D(u_textureMap[1], v_texCoord.st);
            if(u_numTextures > 2) texColors *= texture2D(u_textureMap[2], v_texCoord.st);
            if(u_numTextures > 3) texColors *= texture2D(u_textureMap[3], v_texCoord.st);
            vec3 N = normalize(v_normal);
            vec3 L = normalize(-u_lightDir);
            vec3 E = normalize(v_eyeVec);
		    vec3 R = reflect(-L, N);
            float nDotL = max(0.0, dot(N, L));
            float specIntesity = pow( max(dot(R, E), 0.0), u_material.shinyness);
            vec4 spec = u_material.specular * specIntesity; spec.a = 0.0;
            gl_FragColor = texColors * (u_material.ambient + u_material.diffuse * vec4(vec3(nDotL), 1.0)) + spec;
        });

#else
        const char *unlitVertSrc = GLSL(150 core,
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat4 u_textureMatrix;
        in vec4 a_vertex;
        in vec4 a_texCoord;
        in vec4 a_color;
        out VertexData{
           vec4 color;
           vec2 texCoord;
        } vertex_out;
        void main()
        {
           vertex_out.color = a_color;
           vertex_out.texCoord =  (u_textureMatrix * a_texCoord).xy;
           gl_Position = u_modelViewProjectionMatrix * a_vertex;
        });
        
        const char *unlitFragSrc = GLSL(150 core,
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[16];
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
           vec2 texCoord;
        } vertex_in;
                  
        out vec4 fragData;
        void main()
        {
           vec4 texColors = vertex_in.color;
           for(int i = 0; i < u_numTextures; i++)
           {
               texColors *= texture(u_textureMap[i], vertex_in.texCoord.st);
           }
            //if(texColors.a == 0.0) discard;
           fragData = u_material.diffuse * texColors;
        });
        
        const char *phongVertSrc_skin = GLSL(150 core,
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat3 u_normalMatrix;
        uniform mat4 u_textureMatrix;
        uniform mat4 u_bones[110];
                                    
        in vec4 a_vertex;
        in vec4 a_texCoord;
        in vec3 a_normal;
        in ivec4 a_boneIds;
        in vec4 a_boneWeights;
        out VertexData{
            vec4 color;
            vec4 texCoord;
            vec3 normal;
            vec3 eyeVec;
        } vertex_out;
                  
        void main()
        {
            vec4 newVertex = vec4(0);
            vec4 newNormal = vec4(0);
            for (int i = 0; i < 4; i++)
            {
                newVertex += u_bones[a_boneIds[i]] * a_vertex * a_boneWeights[i];
                newNormal += u_bones[a_boneIds[i]] * vec4(a_normal, 0.0) * a_boneWeights[i];
            }
            vertex_out.normal = normalize(u_normalMatrix * newNormal.xyz);
            vertex_out.texCoord =  u_textureMatrix * a_texCoord;
            vertex_out.eyeVec = (u_modelViewMatrix * newVertex).xyz;
            gl_Position = u_modelViewProjectionMatrix * vec4(newVertex.xyz, 1.0);
        });
        
        phong_normalmap_vertSrc = GLSL(150 core,
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform mat3 u_normalMatrix;
        uniform mat4 u_textureMatrix;
        uniform vec3 u_lightDir;
                                                   
        in vec4 a_vertex;
        in vec4 a_texCoord;
        in vec3 a_normal;
        in vec3 a_tangent;
                                                   
        out VertexData{
            vec4 color;
            vec4 texCoord;
            vec3 normal;
            vec3 eyeVec;
            vec3 lightDir;
        } vertex_out;
                                                   
        void main()
        {
           vertex_out.normal = normalize(u_normalMatrix * a_normal);
           vec3 t = normalize (u_normalMatrix * a_tangent);
           vec3 b = cross(vertex_out.normal, t);
           mat3 tbnMatrix = mat3(t,b, vertex_out.normal);
           vertex_out.eyeVec = tbnMatrix * normalize(- (u_modelViewMatrix * a_vertex).xyz);
           vertex_out.lightDir = tbnMatrix * u_lightDir;
           vertex_out.texCoord =  u_textureMatrix * a_texCoord;
           gl_Position = u_modelViewProjectionMatrix * a_vertex;
        });
        
        phong_normalmap_fragSrc = GLSL(150 core,
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
        in VertexData{
            vec4 color;
            vec4 texCoord;
            vec3 normal;
            vec3 eyeVec;
            vec3 lightDir;
        } vertex_in;
        out vec4 fragData;
        vec3 normalFromHeightMap(sampler2D theMap, vec2 theCoords, float theStrength)
        {
            float center = texture(theMap, theCoords).r ;	 //center bump map sample
            float U = texture(theMap, theCoords + vec2( 0.005, 0)).r ;	//U bump map sample
            float V = texture(theMap, theCoords + vec2(0, 0.005)).r ;	 //V bump map sample
            float dHdU = U - center;	 //create bump map U offset
            float dHdV = V - center;	 //create bump map V offset
            vec3 normal = vec3( -dHdU, dHdV, 0.05 / theStrength);	 //create the tangent space normal
            return normalize(normal);
        }
        
        void main()
        {
            //vec2 texCoord = v_texCoord.xy;
            vec4 texColors = texture(u_textureMap[0], vertex_in.texCoord.xy);
            vec3 N;
            // sample normal map
            //N = texture(u_textureMap[1], v_texCoord.xy).xyz * 2.0 - 1.0;
            // sample bump map
            N = normalFromHeightMap(u_textureMap[1], vertex_in.texCoord.xy, 0.8);
            vec3 L = normalize(-vertex_in.lightDir);
            vec3 E = normalize(vertex_in.eyeVec);
		    vec3 R = reflect(-L, N);
            float nDotL = max(0.0, dot(N, L));
            float specIntesity = pow( max(dot(R, E), 0.0), u_material.shinyness);
            vec4 spec = u_material.specular * specIntesity; spec.a = 0.0;
            fragData = texColors * (u_material.ambient + u_material.diffuse * vec4(vec3(nDotL), 1.0)) + spec;
        });
#endif
        
#ifdef KINSKI_GLES
        const char *point_vertSrc =
        STRINGIFY(
        uniform mat4 u_modelViewProjectionMatrix;
        uniform float u_pointSize;
        attribute vec4 a_vertex;
        attribute vec4 a_color;
        attribute float a_pointSize;
        varying lowp vec4 v_color;
        void main()
        {
           gl_Position = u_modelViewProjectionMatrix * a_vertex;
           gl_PointSize = a_pointSize;
           v_color = a_color;
        });
        
        const char *point_fragSrc =
        STRINGIFY(
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
        varying vec4 v_color;
        void main(){
        vec4 texColors = v_color;
        for(int i = 0; i < u_numTextures; i++)
        {
            texColors *= texture2D(u_textureMap[i], gl_PointCoord);
        }
        gl_FragColor = u_material.diffuse * texColors;
        });
#else
        const char *point_vertSrc = GLSL(150 core,
        uniform mat4 u_modelViewMatrix;
        uniform mat4 u_modelViewProjectionMatrix;
        uniform float u_pointSize;
        uniform struct{
            float constant;
            float linear;
            float quadratic;
        } u_point_attenuation;
                  
        in vec4 a_vertex;
        in float a_pointSize;
        in vec4 a_color;
        out vec4 v_color;
        out vec3 v_eyeVec;
        void main()
        {
            v_color = a_color;
            v_eyeVec = -(u_modelViewMatrix * a_vertex).xyz;
            float d = length(v_eyeVec);
            float attenuation = 1.0 / (u_point_attenuation.constant +
                                       u_point_attenuation.linear * d +
                                       u_point_attenuation.quadratic * (d * d));
            gl_PointSize = max(a_pointSize, u_pointSize) * attenuation;
            gl_Position = u_modelViewProjectionMatrix * a_vertex;
        });
        
        point_color_fragSrc = GLSL(150 core,
                                   
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
        in vec4 v_color;
        out vec4 fragData;
        void main()
        {
            vec4 texColors = v_color;
            fragData = u_material.diffuse * texColors;
        });
        
        point_texture_fragSrc = GLSL(150 core,
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[8];
        uniform struct{
        vec4 diffuse;
        vec4 ambient;
        vec4 specular;
        vec4 emission;
        } u_material;
        in vec4 v_color;
        out vec4 fragData;
        void main()
        {
            vec4 texColors = v_color; // workaround for ATI gl_PointCoord bug
            for(int i = 0; i < u_numTextures; i++)
            {
                texColors *= texture(u_textureMap[i], gl_PointCoord.xy);
            }
            fragData = u_material.diffuse * texColors;
        });
        
        // pixel shader for rendering points as shaded spheres
        point_sphere_fragSrc = STRINGIFY(
        uniform float u_pointRadius;  // point size in world space
        uniform vec3 u_lightDir;
        uniform int u_numTextures;
        uniform sampler2D u_textureMap[8];
        uniform Lightsource u_lights[16];
        uniform Material u_material;
        in vec4 v_color;
        in vec3 v_eyeVec;        // position of center in eye space
        out vec4 fragData;
        void main()
        {
            vec4 texColors = v_color;// workaround for ATI gl_PointCoord bug
            
            for(int i = 0; i < u_numTextures; i++)
            {
                texColors *= texture(u_textureMap[i], gl_PointCoord);
            }
            
            // calculate normal from texture coordinates
            vec3 N;
            N.xy = gl_PointCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
            float mag = dot(N.xy, N.xy);
            if (mag > 1.0) discard;   // kill pixels outside circle
            N.z = sqrt(1.0 - mag);
            
            // point on surface of sphere in eye space
            vec3 spherePosEye = v_eyeVec + N * u_pointRadius;
            
            vec3 L = normalize(-u_lightDir);
            vec3 E = normalize(v_eyeVec);
            //vec3 R = reflect(-L, N);
            float nDotL = max(0.0, dot(N, L));
            
            vec3 v = normalize(-spherePosEye);
            vec3 h = normalize(-u_lightDir + v);
            float specIntesity = pow( max(dot(N, h), 0.0), u_material.shinyness);
            vec4 spec = u_material.specular * specIntesity; spec.a = 0.0;
            fragData = texColors * (u_material.ambient + u_material.diffuse * vec4(vec3(nDotL), 1.0)) + spec;
            
//            vec4 shade_color = vec4(0);
//            for(int i = 0; i < u_numLights; i++)
//            {
//                shade_color += shade(u_lights[i], u_material, N, -normalize(spherePosEye), texColors);
//            }
//            
//            fragData = shade_color;
        });
        
        const char *line_geom_src = GLSL(150 core,
        
        // ------------------ Geometry Shader --------------------------------
        layout(lines) in;
        layout (triangle_strip, max_vertices = 4) out;
        
        uniform float u_line_thickness;
        uniform vec2 u_window_size;
        
        in VertexData {
            vec4 color;
            vec2 texCoord;
        } vertex_in[2];
        
        out VertexData {
            vec4 color;
            vec2 texCoord;
        } vertex_out;
        
        vec2 screen_space(vec4 vertex)
        {
            return vertex.xy / vertex.w;// * u_window_size;
        }
                                         
        void main()
        {
            // get the four vertices passed to the shader:
            vec2 p0 = screen_space(gl_in[0].gl_Position);	// start of current segment
            vec2 p1 = screen_space(gl_in[1].gl_Position);	// end of current segment
            
//            vec2 area = vec2(1.2) ;//* u_window_size;
//            if( p0.x < -area.x || p0.x > area.x ) return;
//            if( p0.y < -area.y || p0.y > area.y ) return;
//            if( p1.x < -area.x || p1.x > area.x ) return;
//            if( p1.y < -area.y || p1.y > area.y ) return;
            
            // determine the direction of the segment
            vec2 v0 = normalize(p1 - p0);
            
            // determine the normal
            vec2 n0 = vec2(-v0.y, v0.x);
            
            // generate the triangle strip
            vec2 bias = n0 * u_line_thickness / u_window_size;
            
            vertex_out.color = vertex_in[0].color;
            vertex_out.texCoord = vec2(0, 1);
            gl_Position = vec4(p0 + bias , 0, 1);
            EmitVertex();
            
            vertex_out.color = vertex_in[0].color;
            vertex_out.texCoord = vec2(0, 0);
            gl_Position = vec4(p0 - bias, 0, 1);
            EmitVertex();
            
            vertex_out.color = vertex_in[1].color;
            vertex_out.texCoord = vec2(0, 1);
            gl_Position = vec4(p1 + bias, 0, 1);
            EmitVertex();
            
            vertex_out.color = vertex_in[1].color;
            vertex_out.texCoord = vec2(0, 0);
            gl_Position = vec4(p1 - bias, 0, 1);
            EmitVertex();
            
            EndPrimitive();
        });
        
#endif
        
        std::string vert_src, geom_src, frag_src;
                                        
        Shader ret;
        switch (type)
        {
            case SHADER_UNLIT:
                ret.loadFromData(unlitVertSrc, unlitFragSrc);
                break;
            
            case SHADER_GOURAUD:
                vert_src =  glsl_header +
                material_block +
                light_block +
                shade_function_block +
                vertex_shader_gouraud;
                
                frag_src =  glsl_header +
                frag_shader_gouraud;
                ret.loadFromData(vert_src.c_str(), frag_src.c_str());
                break;
                
            case SHADER_PHONG:
                vert_src =  glsl_header +
                            //glsl_define_explicit_layout +
                            light_block +
                            vertex_shader_phong;
                
                frag_src =  glsl_header +
                            material_block +
                            light_block +
                            shade_function_block +
                            frag_shader_phong;
                ret.loadFromData(vert_src.c_str(), frag_src.c_str());
                break;
            
            case SHADER_PHONG_NORMALMAP:
                ret.loadFromData(phong_normalmap_vertSrc, phong_normalmap_fragSrc);
                break;
                
            case SHADER_PHONG_SKIN:
                vert_src = glsl_header + vertex_shader_phong;
                frag_src =  glsl_header +
                material_block +
                light_block +
                shade_function_block +
                frag_shader_phong;
                ret.loadFromData(phongVertSrc_skin, frag_src.c_str());
                break;
            
            case SHADER_LINES:
                ret.loadFromData(unlitVertSrc, unlitFragSrc, line_geom_src);
                break;
                
            case SHADER_POINTS_TEXTURE:
                ret.loadFromData(point_vertSrc, point_texture_fragSrc);
                break;
                
            case SHADER_POINTS_COLOR:
                ret.loadFromData(point_vertSrc, point_color_fragSrc);
                break;
                
            case SHADER_POINTS_SPHERE:
                frag_src = glsl_header +
                material_block +
                light_block +
                shade_function_block +
                point_sphere_fragSrc;
                ret.loadFromData(point_vertSrc, frag_src.c_str());
                break;
                
            default:
                break;
        }
        KINSKI_CHECK_GL_ERRORS();
        return ret;
    }
    
}} //namespace
