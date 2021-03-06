// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2013, Fabian Schmidt <crocdialer@googlemail.com>
//
// It is distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __

#ifndef __gl__Material__
#define __gl__Material__

#include <boost/variant.hpp>
#include "KinskiGL.h"
#include "Shader.h"
#include "Buffer.h"
#include "Texture.h"

namespace kinski { namespace gl {
    
    class KINSKI_API Material
    {
    public:
        
        typedef std::shared_ptr<Material> Ptr;
        typedef std::shared_ptr<const Material> ConstPtr;
        typedef std::weak_ptr<Material> WeakPtr;
        
        typedef boost::variant<GLint, GLfloat, double, glm::vec2, glm::vec3, glm::vec4,
        glm::mat3, glm::mat4,
        std::vector<GLint>, std::vector<GLfloat>,
        std::vector<glm::vec2>, std::vector<glm::vec3>, std::vector<glm::vec4>,
        std::vector<glm::mat3>, std::vector<glm::mat4> > UniformValue;
        
        typedef std::map<std::string, UniformValue> UniformMap;

        static Ptr create(const Shader &theShader = Shader())
        {
            return Ptr(new Material(theShader));
        }
        
        Material(const Shader &theShader = Shader(), const UniformMap &theUniforms = UniformMap());

        bool dirty() const { return m_dirty; };
        
        void addTexture(const Texture &theTexture) {m_textures.push_back(theTexture);};
        
        inline void uniform(const std::string &theName, const UniformValue &theVal)
        { m_uniforms[theName] = theVal; };
        
        void update_uniform_buffer();
        
        Shader& shader() {return m_shader;};
        const Shader& shader() const {return m_shader;};
        void setShader(const Shader &theShader) { m_shader = theShader; };
        
        std::vector<Texture>& textures() {return m_textures;};
        const std::vector<Texture>& textures() const {return m_textures;};
        
        UniformMap& uniforms() {return m_uniforms;};
        const UniformMap& uniforms() const {return m_uniforms;};
        
        bool twoSided() const { return m_twoSided; };
        bool wireframe() const { return m_wireFrame; };
        bool blending() const { return m_blending; };
        GLenum blend_src() const { return m_blend_src; };
        GLenum blend_dst() const { return m_blend_dst; };
        GLenum blend_equation() const { return m_blend_equation; };
        
        void setTwoSided(bool b = true) { m_twoSided = b;};
        void setWireframe(bool b = true) { m_wireFrame = b;};
        void setBlending(bool b = true) { m_blending = b;};
        void set_blend_factors(GLenum src, GLenum dst){ m_blend_src = src; m_blend_dst = dst; };
        void set_blend_equation(GLenum equation){ m_blend_equation = equation;};
        
        void setDepthTest(bool b = true) { m_depthTest = b;};
        void setDepthWrite(bool b = true) { m_depthWrite = b;};
        
        bool opaque() const { return !m_blending && m_diffuse.a == 1.f ;};
        bool depthTest() const { return m_depthTest; };
        bool depthWrite() const { return m_depthWrite; };
        float pointSize() const { return m_pointSize; };
        
        const Color& diffuse() const { return m_diffuse; };
        const Color& ambient() const { return m_ambient; };
        const Color& specular() const { return m_specular; };
        const Color& emission() const { return m_emission; };
        const float shinyness() const { return m_shinyness; };
        
        void setDiffuse(const Color &theColor);
        void setAmbient(const Color &theColor);
        void setSpecular(const Color &theColor);
        void setEmission(const Color &theColor);
        void setShinyness(float s);
        void setPointSize(float sz);
        void setPointAttenuation(float constant, float linear, float quadratic);
        void setLineThickness(float t);
        
    private:
        
        Shader m_shader;
        
        UniformMap m_uniforms;
        gl::Buffer m_uniform_buffer;
        
        bool m_dirty;
        
        GLenum m_polygonMode;
        bool m_twoSided;
        bool m_wireFrame;
        bool m_depthTest;
        bool m_depthWrite;
        bool m_blending;
        GLenum m_blend_src, m_blend_dst, m_blend_equation;
        
        // those are available in shader
        Color m_diffuse;
        Color m_ambient;
        Color m_specular;
        Color m_emission;
        float m_shinyness;
        
        std::vector<Texture> m_textures;
        
        // point attributes
        float m_pointSize;
        struct PointAttenuation
        {
            float constant, linear, quadratic;
            PointAttenuation():constant(1.f), linear(0.f), quadratic(0.f){}
            PointAttenuation(float c, float l, float q):constant(c), linear(l), quadratic(q)
            {};
        } m_point_attenuation;
    };
    
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
   
}} // namespace

#endif 
