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
        
        ret.loadFromData(vertSrc.c_str(), fragSrc.c_str(), geomSrc.empty() ? NULL : geomSrc.c_str());
        return ret;
    }
    
    Shader createShader(ShaderType type)
    {
#ifdef KINSKI_GLES
        const char *unlitVertSrc =
        "uniform mat4 u_modelViewProjectionMatrix;\n"
        "uniform mat4 u_textureMatrix;\n"
        "attribute vec4 a_vertex;\n"
        "attribute vec4 a_texCoord;\n"
        "varying lowp vec4 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    v_texCoord =  u_textureMatrix * a_texCoord;\n"
        "    gl_Position = u_modelViewProjectionMatrix * a_vertex;\n"
        "}\n";
        
        const char *unlitFragSrc =
        "precision mediump float;\n"
        "precision lowp int;\n"
        "uniform int u_numTextures;\n"
        "uniform sampler2D u_textureMap[4];\n"
        "uniform struct\n"
        "{\n"
        "    vec4 diffuse;\n"
        "    vec4 ambient;\n"
        "    vec4 specular;\n"
        "    vec4 emission;\n"
        "    float shinyness;\n"
        "} u_material;\n"
        "varying vec4 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    vec4 texColors = vec4(1.0);\n"
        "    if(u_numTextures > 0) texColors *= texture2D(u_textureMap[0], v_texCoord.st);\n"
        "    if(u_numTextures > 1) texColors *= texture2D(u_textureMap[1], v_texCoord.st);\n"
        "    if(u_numTextures > 2) texColors *= texture2D(u_textureMap[2], v_texCoord.st);\n"
        "    if(u_numTextures > 3) texColors *= texture2D(u_textureMap[3], v_texCoord.st);\n"
        "    gl_FragColor = u_material.diffuse * texColors;\n"
        "}\n";
        
        const char *phongVertSrc =
        "uniform mat4 u_modelViewMatrix;\n"
        "uniform mat4 u_modelViewProjectionMatrix;\n"
        "uniform mat3 u_normalMatrix;\n"
        "uniform mat4 u_textureMatrix;\n"
        "attribute vec4 a_vertex;\n"
        "attribute vec4 a_texCoord;\n"
        "attribute vec3 a_normal;\n"
        "varying vec4 v_texCoord;\n"
        "varying vec3 v_normal;\n"
        "varying vec3 v_eyeVec;\n"
        "void main()\n"
        "{\n"
        "    v_normal = normalize(u_normalMatrix * a_normal);\n"
        "    v_texCoord =  u_textureMatrix * a_texCoord;\n"
        "    v_eyeVec = - (u_modelViewMatrix * a_vertex).xyz;\n"
        "    gl_Position = u_modelViewProjectionMatrix * a_vertex;\n"
        "}\n";
        
        const char *phongVertSrc_skin =
        "uniform mat4 u_modelViewMatrix;\n"
        "uniform mat4 u_modelViewProjectionMatrix;\n"
        "uniform mat3 u_normalMatrix;\n"
        "uniform mat4 u_textureMatrix;\n"
        "uniform mat4 u_bones[25];\n"
        "attribute vec4 a_vertex;\n"
        "attribute vec4 a_texCoord;\n"
        "attribute vec3 a_normal;\n"
        "attribute vec4 a_boneIds;\n"
        "attribute vec4 a_boneWeights;\n"
        "varying vec4 v_texCoord;\n"
        "varying vec3 v_normal;\n"
        "varying vec3 v_eyeVec;\n"
        "void main()\n"
        "{\n"
        "    vec4 newVertex = vec4(0.0);\n"
        "    vec4 newNormal = vec4(0.0);\n"
        "    for (int i = 0; i < 4; i++)\n"
        "    {\n"
        "        newVertex += u_bones[(int)a_boneIds[i]] * a_vertex * a_boneWeights[i];\n"
        "        newNormal += u_bones[(int)a_boneIds[i]] * vec4(a_normal, 0.0) * a_boneWeights[i];\n"
        "    }\n"
        "    v_normal = normalize(u_normalMatrix * newNormal.xyz);\n"
        "    v_texCoord =  u_textureMatrix * a_texCoord;\n"
        "    v_eyeVec = - (u_modelViewMatrix * newVertex).xyz;\n"
        "    gl_Position = u_modelViewProjectionMatrix * vec4(newVertex.xyz, 1.0);\n"
        "}\n";
        
        const char *phongFragSrc =
        "uniform int u_numTextures;\n"
        "uniform sampler2D u_textureMap[4];\n"
        "uniform vec3 u_lightDir;\n"
        "uniform struct\n"
        "{\n"
        "    vec4 diffuse;\n"
        "    vec4 ambient;\n"
        "    vec4 specular;\n"
        "    vec4 emission;\n"
        "    float shinyness;\n"
        "} u_material;\n"
        "varying vec3 v_normal;\n"
        "varying vec4 v_texCoord;\n"
        "varying vec3 v_eyeVec;\n"
        "void main()\n"
        "{\n"
        "    vec4 texColors = vec4(1);\n"
        "    if(u_numTextures > 0) texColors *= texture2D(u_textureMap[0], v_texCoord.st);\n"
        "    if(u_numTextures > 1) texColors *= texture2D(u_textureMap[1], v_texCoord.st);\n"
        "    if(u_numTextures > 2) texColors *= texture2D(u_textureMap[2], v_texCoord.st);\n"
        "    if(u_numTextures > 3) texColors *= texture2D(u_textureMap[3], v_texCoord.st);\n"
        "    vec3 N = normalize(v_normal);\n"
        "    vec3 L = normalize(-u_lightDir);\n"
        "    vec3 E = normalize(v_eyeVec);\n"
		"    vec3 R = reflect(-L, N);\n"
        "    float nDotL = max(0.0, dot(N, L));\n"
        "    float specIntesity = pow( max(dot(R, E), 0.0), u_material.shinyness);\n"
        "    vec4 spec = u_material.specular * specIntesity; spec.a = 0.0;\n"
        "    gl_FragColor = u_material.diffuse * texColors * vec4(vec3(nDotL), 1.0) ;\n"
        "}\n";
#else
        const char *unlitVertSrc =
        "#version 150 core\n"
        "uniform mat4 u_modelViewProjectionMatrix;\n"
        "uniform mat4 u_textureMatrix;\n"
        "in vec4 a_vertex;\n"
        "in vec4 a_texCoord;\n"
        "in vec4 a_color;\n"//
        "out vec4 v_color;\n"//
        "out vec4 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    v_color = a_color;\n"
        "    v_texCoord =  u_textureMatrix * a_texCoord;\n"
        "    gl_Position = u_modelViewProjectionMatrix * a_vertex;\n"
        "}\n";
        
        const char *unlitFragSrc =
        "#version 150 core\n"
        "uniform int u_numTextures;\n"
        "uniform sampler2D u_textureMap[16];\n"
        "uniform struct{\n"
        "    vec4 diffuse;\n"
        "    vec4 ambient;\n"
        "    vec4 specular;\n"
        "    vec4 emission;\n"
        "} u_material;\n"
        "in vec4 v_color;\n"//
        "in vec4 v_texCoord;\n"//
        "out vec4 fragData;\n"
        "void main()\n"
        "{\n"
        "    vec4 texColors = v_color;\n"//
        "    for(int i = 0; i < u_numTextures; i++)\n"
        "    {\n"
        "        texColors *= texture(u_textureMap[i], v_texCoord.st);\n"
        "    }\n"
        "    fragData = u_material.diffuse * texColors;\n"
        "}\n";
        
        const char *phongVertSrc =
        "#version 150 core\n"
        "uniform mat4 u_modelViewMatrix;\n"
        "uniform mat4 u_modelViewProjectionMatrix;\n"
        "uniform mat3 u_normalMatrix;\n"
        "uniform mat4 u_textureMatrix;\n"
        "in vec4 a_vertex;\n"
        "in vec4 a_texCoord;\n"
        "in vec3 a_normal;\n"
        "out vec4 v_texCoord;\n"
        "out vec3 v_normal;\n"
        "out vec3 v_eyeVec;\n"
        "void main()\n"
        "{\n"
        "    v_normal = normalize(u_normalMatrix * a_normal);\n"
        "    v_texCoord =  u_textureMatrix * a_texCoord;\n"
        "    v_eyeVec = - (u_modelViewMatrix * a_vertex).xyz;\n"
        "    gl_Position = u_modelViewProjectionMatrix * a_vertex;\n"
        "}\n";
        
        const char *phongVertSrc_skin =
        "#version 150 core\n"
        "uniform mat4 u_modelViewMatrix;\n"
        "uniform mat4 u_modelViewProjectionMatrix;\n"
        "uniform mat3 u_normalMatrix;\n"
        "uniform mat4 u_textureMatrix;\n"
        "uniform mat4 u_bones[25];\n"
        "in vec4 a_vertex;\n"
        "in vec4 a_texCoord;\n"
        "in vec3 a_normal;\n"
        "in ivec4 a_boneIds;\n"
        "in vec4 a_boneWeights;\n"
        "out vec4 v_texCoord;\n"
        "out vec3 v_normal;\n"
        "out vec3 v_eyeVec;\n"
        "void main()\n"
        "{\n"
        "    vec4 newVertex = vec4(0);\n"
        "    vec4 newNormal = vec4(0);\n"
        "    for (int i = 0; i < 4; i++)\n"
        "    {\n"
        "        newVertex += u_bones[a_boneIds[i]] * a_vertex * a_boneWeights[i];\n"
        "        newNormal += u_bones[a_boneIds[i]] * vec4(a_normal, 0.0) * a_boneWeights[i];\n"
        "    }\n"
        "    v_normal = normalize(u_normalMatrix * newNormal.xyz);\n"
        "    v_texCoord =  u_textureMatrix * a_texCoord;\n"
        "    v_eyeVec = - (u_modelViewMatrix * newVertex).xyz;\n"
        "    gl_Position = u_modelViewProjectionMatrix * vec4(newVertex.xyz, 1.0);\n"
        "}\n";
        
        const char *phongFragSrc =
        "#version 150 core\n"
        "uniform int u_numTextures;\n"
        "uniform sampler2D u_textureMap[16];\n"
        "uniform vec3 u_lightDir;\n"
        "uniform struct\n"
        "{\n"
        "    vec4 diffuse;\n"
        "    vec4 ambient;\n"
        "    vec4 specular;\n"
        "    vec4 emission;\n"
        "    float shinyness;\n"
        "} u_material;\n"
        "in vec3 v_normal;\n"
        "in vec4 v_texCoord;\n"
        "in vec3 v_eyeVec;\n"
        "out vec4 fragData;\n"
        "void main()\n"
        "{\n"
        "    vec4 texColors = vec4(1);\n"
        "    for(int i = 0; i < u_numTextures; i++)\n"
        "    {\n"
        "        texColors *= texture(u_textureMap[i], v_texCoord.st);\n"
        "    }\n"
        "    vec3 N = normalize(v_normal);\n"
        "    vec3 L = normalize(-u_lightDir);\n"
        "    vec3 E = normalize(v_eyeVec);\n"
		"    vec3 R = reflect(-L, N);\n"
        "    float nDotL = max(0.0, dot(N, L));\n"
        "    float specIntesity = pow( max(dot(R, E), 0.0), u_material.shinyness);\n"
        "    vec4 spec = u_material.specular * specIntesity; spec.a = 0.0;\n"
        "    fragData = u_material.diffuse * texColors * vec4(vec3(nDotL), 1.0f) ;\n"
        "}\n";
#endif
        
        Shader ret;
        switch (type)
        {
            case SHADER_UNLIT:
                ret.loadFromData(unlitVertSrc, unlitFragSrc);
                break;
                
            case SHADER_PHONG:
                ret.loadFromData(phongVertSrc, phongFragSrc);
                break;
                
            case SHADER_PHONG_SKIN:
                ret.loadFromData(phongVertSrc_skin, phongFragSrc);
                break;
                
            default:
                break;
        }
        KINSKI_CHECK_GL_ERRORS();
        
        return ret;
    }
    
}} //namespace