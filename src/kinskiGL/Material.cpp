//
//  Material.cpp
//  kinskiGL
//
//  Created by Fabian on 10/12/12.
//
//

#include "Material.h"

using namespace std;

namespace kinski { namespace gl {

    class InsertUniformVisitor : public boost::static_visitor<>
    {
    private:
        gl::Shader &m_shader;
        const std::string &m_uniform;
        
    public:
        
        InsertUniformVisitor(gl::Shader &theShader, const std::string &theUniform)
        :m_shader(theShader), m_uniform(theUniform){};
        
        template <typename T>
        void operator()( T &value ) const
        {
            m_shader.uniform(m_uniform, value);
        }
    };
    
    Material::Material(const Shader &theShader, const UniformMap &theUniforms):
    m_shader(theShader),
    m_uniforms(theUniforms),
    m_diffuse(glm::vec4(1)),
    m_ambient(glm::vec4(1)),
    m_specular(glm::vec4(1)),
    m_emission(glm::vec4(0)),
    m_polygonMode(GL_FRONT),
    m_wireFrame(false),
    m_depthTest(true),
    m_depthWrite(true),
    m_blending(false), m_blendSrc(GL_SRC_ALPHA), m_blendDst(GL_ONE_MINUS_SRC_ALPHA)
    {
        m_uniforms["u_material.diffuse"] = m_diffuse;
        m_uniforms["u_material.ambient"] = m_ambient;
        m_uniforms["u_material.specular"] = m_specular;
        m_uniforms["u_material.emmission"] = m_emission;
    }
    
    void Material::apply() const
    {
        m_shader.bind();
        
        char buf[512];
        
        // wireframe ?
        glPolygonMode(GL_FRONT_AND_BACK, m_wireFrame ? GL_LINE : GL_FILL);
        
        // read write depth buffer ?
        if(m_depthTest) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        
        if(m_depthWrite) glDepthMask(GL_TRUE);
        else glDepthMask(GL_FALSE);
        
        if(m_blending)
        {
            glEnable(GL_BLEND);
            glBlendFunc(m_blendSrc, m_blendDst);
        }
        else
            glDisable(GL_BLEND);
        
        // enable texturing
        if(!m_textures.empty()) glEnable(GL_TEXTURE_2D);
        
        // texture matrix from first texture, if any
        m_shader.uniform("u_textureMatrix",
                         m_textures.empty() ? glm::mat4() : m_textures.front().getTextureMatrix());
        
        // add texturemaps
        for(int i=0;i<m_textures.size();i++)
        {
            m_textures[i].bind(i);
            sprintf(buf, "u_textureMap[%d]", i);
            m_shader.uniform(buf, i);
        }
        
        // set all other uniform values
        for (UniformMap::const_iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            boost::apply_visitor(InsertUniformVisitor(m_shader, it->first), it->second);
        }
    }
    
    void Material::setDiffuse(const glm::vec4 &theColor)
    {
        m_diffuse = glm::clamp(theColor, glm::vec4(0), glm::vec4(1));
        m_uniforms["u_material.diffuse"] = m_diffuse;
    }
    
    void Material::setAmbient(const glm::vec4 &theColor)
    {
        m_ambient = glm::clamp(theColor, glm::vec4(0), glm::vec4(1));
        m_uniforms["u_material.ambient"] = m_ambient;
    }
    
    void Material::setSpecular(const glm::vec4 &theColor)
    {
        m_specular = glm::clamp(theColor, glm::vec4(0), glm::vec4(1));
        m_uniforms["u_material.specular"] = m_specular;
    }
    
    void Material::setEmission(const glm::vec4 &theColor)
    {
        m_emission = glm::clamp(theColor, glm::vec4(0), glm::vec4(1));
        m_uniforms["u_material.emmission"] = m_emission;
    }

}}// namespace