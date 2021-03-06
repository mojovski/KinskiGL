// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2013, Fabian Schmidt <crocdialer@googlemail.com>
//
// It is distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __

#include "KinskiGL.h"
#include "Material.h"
#include "Shader.h"
#include "ShaderLibrary.h"
#include "Texture.h"
#include "Camera.h"
#include "Mesh.h"
#include "Font.h"
#include "Scene.h"
#include "Fbo.h"

using namespace glm;
using namespace std;

// maximum matrix-stack size
#define MAX_MATRIX_STACK_SIZE 100

// how many string meshes are buffered at max
#define STRING_MESH_BUFFER_SIZE 300

namespace kinski { namespace gl {
    
    struct string_mesh_container
    {
        std::string text;
        MeshPtr mesh;
        uint64_t counter;
        string_mesh_container():counter(0){};
        string_mesh_container(const std::string &t, const MeshPtr &m):text(t), mesh(m), counter(0){}
        bool operator<(const string_mesh_container &other) const {return counter < other.counter;}
    };
    
///////////////////////////////////////////////////////////////////////////////
    
    class Impl
    {
    private:
        
        glm::vec2 g_viewportDim;
        std::stack<glm::mat4> g_projectionMatrixStack;
        std::stack<glm::mat4> g_modelViewMatrixStack;
        std::map<std::string, string_mesh_container> g_string_mesh_map;
    };
    
    static glm::vec2 g_viewportDim;
    static std::stack<glm::mat4> g_projectionMatrixStack;
    static std::stack<glm::mat4> g_modelViewMatrixStack;
    static gl::MaterialPtr g_line_material;
    static std::map<std::string, string_mesh_container> g_string_mesh_map;
    
///////////////////////////////////////////////////////////////////////////////
    
    void pushMatrix(const Matrixtype type)
    {
        if(g_modelViewMatrixStack.size() > MAX_MATRIX_STACK_SIZE ||
           g_projectionMatrixStack.size() > MAX_MATRIX_STACK_SIZE)
        {
            throw Exception("Matrix stack overflow");
        }
        
        switch (type)
        {
            case PROJECTION_MATRIX:
                g_projectionMatrixStack.push(g_projectionMatrixStack.top());
                break;
            case MODEL_VIEW_MATRIX:
                g_modelViewMatrixStack.push(g_modelViewMatrixStack.top());
                break;
                
            default:
                break;
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void popMatrix(const Matrixtype type)
    {
        switch (type)
        {
            case PROJECTION_MATRIX:
                if(g_projectionMatrixStack.size() > 1) g_projectionMatrixStack.pop();
                break;
            case MODEL_VIEW_MATRIX:
                if(g_modelViewMatrixStack.size() > 1) g_modelViewMatrixStack.pop();
                break;
                
            default:
                break;
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void multMatrix(const Matrixtype type, const glm::mat4 &theMatrix)
    {
        switch (type)
        {
            case PROJECTION_MATRIX:
                g_projectionMatrixStack.top() *= theMatrix;
                break;
            case MODEL_VIEW_MATRIX:
                g_modelViewMatrixStack.top() *= theMatrix;
                break;
                
            default:
                break;
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void loadMatrix(const Matrixtype type, const glm::mat4 &theMatrix)
    {
        switch (type)
        {
            case PROJECTION_MATRIX:
                g_projectionMatrixStack.top() = theMatrix;
                break;
            case MODEL_VIEW_MATRIX:
                g_modelViewMatrixStack.top() = theMatrix;
                break;
                
            default:
                break;
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void loadIdentity(const Matrixtype type)
    {
        loadMatrix(type, glm::mat4());
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void getMatrix(const Matrixtype type, glm::mat4 &theMatrix)
    {
        switch (type)
        {
            case PROJECTION_MATRIX:
                theMatrix = g_projectionMatrixStack.top();
                break;
            case MODEL_VIEW_MATRIX:
                theMatrix = g_modelViewMatrixStack.top();
                break;
                
            default:
                break;
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void setMatrices( const CameraPtr &cam )
    {
        setProjection(cam);
        setModelView(cam);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void setModelView( const CameraPtr &cam )
    {
        loadMatrix(MODEL_VIEW_MATRIX, cam->getViewMatrix());
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void setProjection( const CameraPtr &cam )
    {
        loadMatrix(PROJECTION_MATRIX, cam->getProjectionMatrix());
    }

    ///////////////////////////////////////////////////////////////////////////////
    
    void setMatricesForWindow()
    {
        loadMatrix(gl::PROJECTION_MATRIX, glm::ortho(0.f, gl::windowDimension().x,
                                                     0.f, gl::windowDimension().y,
                                                     0.f, 1.f));
        loadMatrix(gl::MODEL_VIEW_MATRIX, mat4());
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    const glm::vec2& windowDimension(){ return g_viewportDim; }
    
///////////////////////////////////////////////////////////////////////////////
    
    void setWindowDimension(const glm::vec2 &theDim)
    {
        g_viewportDim = theDim;
        glViewport(0, 0, theDim.x, theDim.y);
        
        if(g_projectionMatrixStack.empty())
            g_projectionMatrixStack.push(mat4());
        
        if(g_modelViewMatrixStack.empty())
            g_modelViewMatrixStack.push(mat4());
    }

///////////////////////////////////////////////////////////////////////////////
    
    gl::Ray calculateRay(const CameraPtr &theCamera, uint32_t x, uint32_t y)
    {
        glm::vec3 cam_pos = theCamera->position();
        glm::vec3 lookAt = theCamera->lookAt(),
        side = theCamera->side(), up = theCamera->up();
        float near = theCamera->near();
        // bring click_pos to range -1, 1
        glm::vec2 offset (gl::windowDimension() / 2.0f);
        glm::vec2 click_2D(x, y);
        click_2D -= offset;
        click_2D /= offset;
        click_2D.y = - click_2D.y;
        glm::vec3 click_world_pos;
        
        if(PerspectiveCamera::Ptr cam = dynamic_pointer_cast<PerspectiveCamera>(theCamera) )
        {
            // convert fovy to radians
            float rad = glm::radians(cam->fov());
            float vLength = tan( rad / 2) * near;
            float hLength = vLength * cam->aspectRatio();
            
            click_world_pos = cam_pos + lookAt * near
            + side * hLength * click_2D.x
            + up * vLength * click_2D.y;
            
        }else if (OrthographicCamera::Ptr cam = dynamic_pointer_cast<OrthographicCamera>(theCamera))
        {
            click_world_pos = cam_pos + lookAt * near + side * click_2D.x + up  * click_2D.y;
        }
        LOG_TRACE<<"clicked_world: ("<<click_world_pos.x<<",  "<<click_world_pos.y<<",  "<<click_world_pos.z<<")";
        return Ray(click_world_pos, click_world_pos - cam_pos);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    gl::AABB calculateAABB(const std::vector<glm::vec3> &theVertices)
    {
        AABB ret = AABB(glm::vec3(numeric_limits<float>::max()),
                        glm::vec3(numeric_limits<float>::min()));
        
        for (const glm::vec3 &vertex : theVertices)
        {
            // X
            if(vertex.x < ret.min.x)
                ret.min.x = vertex.x;
            else if(vertex.x > ret.max.x)
                ret.max.x = vertex.x;
            // Y
            if(vertex.y < ret.min.y)
                ret.min.y = vertex.y;
            else if(vertex.y > ret.max.y)
                ret.max.y = vertex.y;
            // Z
            if(vertex.z < ret.min.z)
                ret.min.z = vertex.z;
            else if(vertex.z > ret.max.z)
                ret.max.z = vertex.z;
        }
        return ret;
    }

///////////////////////////////////////////////////////////////////////////////
    
    vec3 calculateCentroid(const vector<vec3> &theVertices)
    {
        if(theVertices.empty())
        {
            LOG_TRACE << "Called gl::calculateCentroid() on zero vertices, returned vec3(0, 0, 0)";
            return vec3(0);
        }
        vec3 sum(0);
        vector<vec3>::const_iterator it = theVertices.begin();
        for(;it != theVertices.end(); ++it)
        {
            sum += *it;
        }
        sum /= theVertices.size();
        return sum;
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    gl::MeshPtr createFrustumMesh(const CameraPtr &cam)
    {
        glm::mat4 inverse_projection = glm::inverse(cam->getProjectionMatrix());
        gl::GeometryPtr geom = Geometry::create();
        geom->setPrimitiveType(GL_LINE_STRIP);
        const glm::vec3 vertices[8] = {vec3(-1, -1, 1), vec3(1, -1, 1), vec3(1, 1, 1), vec3(-1, 1, 1),
            vec3(-1, -1, -1), vec3(1, -1, -1), vec3(1, 1, -1), vec3(-1, 1, -1)};
        const GLuint indices[] = {0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0, 3, 7, 6, 2, 1, 5};
        int num_indices = sizeof(indices) / sizeof(GLuint);
        
        for (int i = 0; i < 8; i++)
        {
            vec4 proj_v = inverse_projection * vec4(vertices[i], 1.f);
            geom->vertices().push_back(vec3(proj_v) / proj_v.w);
        }
        
        geom->appendIndices(indices, num_indices);
        geom->computeBoundingBox();
        gl::MaterialPtr mat = gl::Material::create();
        gl::MeshPtr m = gl::Mesh::create(geom, mat);
        m->setTransform(cam->transform());
        return m;
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void clearColor(const Color &theColor)
    {
        glClearColor(theColor.r, theColor.g, theColor.b, theColor.a);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawLine(const vec2 &a, const vec2 &b, const Color &theColor, float line_thickness)
    {
        static vector<vec3> thePoints;
        thePoints.clear();
        thePoints.push_back(vec3(a, 0));
        thePoints.push_back(vec3(b, 0));
        drawLines2D(thePoints, theColor, line_thickness);
    }
    
    void drawLines2D(const vector<vec3> &thePoints, const vec4 &theColor, float line_thickness)
    {
        ScopedMatrixPush pro(gl::PROJECTION_MATRIX), mod(gl::MODEL_VIEW_MATRIX);
        
        loadMatrix(gl::PROJECTION_MATRIX, glm::ortho(0.f, g_viewportDim[0],
                                                     0.f, g_viewportDim[1],
                                                     0.f, 1.f));
        loadMatrix(gl::MODEL_VIEW_MATRIX, mat4());
        drawLines(thePoints, theColor, line_thickness);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawLines(const vector<vec3> &thePoints, const Color &the_color,
                   float line_thickness)
    {
        static MaterialPtr material;
        if(!material)
        {
            material = gl::Material::create(gl::createShader(gl::SHADER_LINES_2D));
        }
        material->setDiffuse(the_color);
        material->setBlending(the_color.a < 1.f);
        
        drawLines(thePoints, material, line_thickness);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawLines(const vector<vec3> &thePoints, const MaterialPtr &the_material,
                   float line_thickness)
    {
        if(thePoints.empty()) return;
        static MeshPtr mesh;
        static MaterialPtr material;
        
        //create line mesh
        if(!mesh)
        {
            material = gl::Material::create(gl::createShader(gl::SHADER_LINES_2D));
            material->setBlending();
            material->setTwoSided();
            gl::GeometryPtr geom = Geometry::create();
            mesh = gl::Mesh::create(geom, material);
            
            //mesh->geometry()->setPrimitiveType(GL_LINES_ADJACENCY);
            mesh->geometry()->setPrimitiveType(GL_LINES);
        }
        
        mesh->material() = (the_material ? the_material : material);
        mesh->material()->uniform("u_window_size", windowDimension());
        mesh->material()->uniform("u_line_thickness", line_thickness);
        
        mesh->geometry()->appendVertices(thePoints);
        mesh->geometry()->colors().resize(thePoints.size(), mesh->material()->diffuse());
        mesh->geometry()->createGLBuffers();
        gl::drawMesh(mesh);
        mesh->geometry()->vertices().clear();
        mesh->geometry()->colors().clear();
        mesh->geometry()->indices().clear();
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawLineStrip(const vector<vec3> &thePoints, const vec4 &theColor, float line_thickness)
    {
        if(thePoints.empty()) return;
        static gl::MeshPtr mesh;
        
        //create line mesh
        if(!mesh)
        {
            gl::MaterialPtr mat = gl::Material::create();
            mat->setTwoSided();
            gl::GeometryPtr geom = Geometry::create();
            mesh = gl::Mesh::create(geom, mat);
            mesh->geometry()->setPrimitiveType(GL_LINE_STRIP);
        }
        mesh->material()->uniform("u_window_size", windowDimension());
        mesh->material()->uniform("u_line_thickness", line_thickness);
        mesh->geometry()->appendVertices(thePoints);
        mesh->geometry()->colors().resize(thePoints.size(), theColor);
        mesh->geometry()->createGLBuffers();
        gl::drawMesh(mesh);
        mesh->geometry()->vertices().clear();
        mesh->geometry()->colors().clear();
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawPoints(const gl::Buffer &the_point_buf, const MaterialPtr &theMaterial, GLsizei offset)
    {
        static MaterialPtr staticMat;
        static GLuint pointVAO = 0;
        
        //create shader
        if(!staticMat)
        {
            staticMat = gl::Material::create(gl::createShader(gl::SHADER_POINTS_TEXTURE));
            staticMat->setPointSize(2.f);
        }
        
        MaterialPtr activeMat = theMaterial ? theMaterial : staticMat;
        int stride = the_point_buf.stride() ? the_point_buf.stride() : sizeof(glm::vec3);
        
        activeMat->uniform("u_modelViewMatrix", g_modelViewMatrixStack.top());
        
        activeMat->uniform("u_modelViewProjectionMatrix",
                           g_projectionMatrixStack.top()
                           * g_modelViewMatrixStack.top());
        
        apply_material(activeMat);
        
        if(!pointVAO || (activeMat != staticMat) )
        {   
#ifndef KINSKI_NO_VAO
            if(!pointVAO) GL_SUFFIX(glGenVertexArrays)(1, &pointVAO);
            GL_SUFFIX(glBindVertexArray)(pointVAO);
#endif            
            glBindBuffer(the_point_buf.target(), the_point_buf.id());
            
            GLint vertexAttribLocation = activeMat->shader().getAttribLocation("a_vertex");
            glEnableVertexAttribArray(vertexAttribLocation);
            glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE,
                                  stride, BUFFER_OFFSET(offset));
            
            GLint point_size_attribLocation = activeMat->shader().getAttribLocation("a_pointSize");
            if(point_size_attribLocation >= 0) glVertexAttrib1f(point_size_attribLocation, 1.f);
            
#ifndef KINSKI_NO_VAO
            GL_SUFFIX(glBindVertexArray)(0);
#endif
        }
        
        glBindBuffer(the_point_buf.target(), the_point_buf.id());
        
#ifndef KINSKI_NO_VAO
        GL_SUFFIX(glBindVertexArray)(pointVAO);
#endif
        glDrawArrays(GL_POINTS, 0, the_point_buf.numBytes() / sizeof(glm::vec3));
#ifndef KINSKI_NO_VAO
        GL_SUFFIX(glBindVertexArray)(0);
#endif
        
        KINSKI_CHECK_GL_ERRORS();
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawPoints(const std::vector<glm::vec3> &thePoints, const Material::Ptr &theMaterial)
    {
        static gl::Buffer point_buf;
        if(!point_buf){point_buf = gl::Buffer(GL_ARRAY_BUFFER, GL_STREAM_DRAW);}
        point_buf.setData(thePoints);
        drawPoints(point_buf, theMaterial);
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawTexture(const gl::Texture &theTexture, const vec2 &theSize, const vec2 &theTopLeft)
    {
        static gl::MaterialPtr material;
        static Shader shader_2D, shader_2Drect;
        
        // empty texture
        if(!theTexture)
        {
            LOG_WARNING << "trying to draw an empty texture";
            return;
        }
        
        // create shaders
        if(!shader_2D || !shader_2Drect)
        {
            shader_2D = gl::createShader(gl::SHADER_UNLIT);
            shader_2Drect = gl::createShader(gl::SHADER_2D_RECT);
        }
        
        //create material, if not yet here
        if(!material)
        {
            try{ material = gl::Material::create(shader_2D); }
            catch (Exception &e){LOG_ERROR<<e.what();}
            material->setDepthTest(false);
            material->setDepthWrite(false);
            material->setBlending(true);
        }
        
        if(theTexture.getTarget() == GL_TEXTURE_2D){ material->setShader(shader_2D); }
        else if(theTexture.getTarget() == GL_TEXTURE_RECTANGLE)
        {
            material->setShader(shader_2Drect);
            material->uniform("u_texture_size", theTexture.getSize());
        }
        else
        {
            LOG_ERROR << "drawTexture: texture target not supported";
            return;
        }
        
        // add the texture to the material
        material->textures().clear();
        material->addTexture(theTexture);
        
        vec2 sz = theSize;
        // flip to OpenGL coords
        vec2 tl = vec2(theTopLeft.x, g_viewportDim[1] - theTopLeft.y);
        drawQuad(material, tl[0], tl[1], (tl+sz)[0], tl[1]-sz[1]);
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawQuad(const gl::Color &theColor,
                  const vec2 &theSize,
                  const vec2 &theTopLeft,
                  bool filled)
    {
        static gl::Material::Ptr material;
        
        //create material, if not yet here
        if(!material)
        {
            try{material = gl::Material::Ptr(new gl::Material);}
            catch (Exception &e){LOG_ERROR<<e.what();}
            material->setDepthTest(false);
            material->setDepthWrite(false);
            material->setBlending(true);
        }
        material->setDiffuse(theColor);
        
        vec2 sz = theSize;
        // flip to OpenGL coords
        vec2 tl = vec2(theTopLeft.x, g_viewportDim[1] - theTopLeft.y);
        drawQuad(material, tl[0], tl[1], (tl+sz)[0], tl[1]-sz[1]);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawQuad(const gl::MaterialPtr &theMaterial,
                  const vec2 &theSize,
                  const vec2 &theTl,
                  bool filled)
    {
        // flip to OpenGL coords
        vec2 tl = vec2(theTl.x, g_viewportDim[1] - theTl.y);
        drawQuad(theMaterial, tl[0], tl[1], (tl + theSize)[0], tl[1] - theSize[1], filled);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawQuad(const gl::MaterialPtr &theMaterial,
                  float x0, float y0, float x1, float y1, bool filled)
    {
        gl::ScopedMatrixPush model(MODEL_VIEW_MATRIX), projection(PROJECTION_MATRIX);
        
        // orthographic projection with a [0,1] coordinate space
        static MeshPtr quad_mesh;
        static mat4 projectionMatrix = ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        
        if(!quad_mesh)
        {
            gl::GeometryPtr geom = gl::Geometry::create();
            geom->vertices().push_back(glm::vec3(-0.5f, 0.5f, 0.f));
            geom->vertices().push_back(glm::vec3(-0.5f, -0.5f, 0.f));
            geom->vertices().push_back(glm::vec3(0.5f, -0.5f, 0.f));
            geom->vertices().push_back(glm::vec3(0.5f, 0.5f, 0.f));
            geom->texCoords().push_back(glm::vec2(0.f, 1.f));
            geom->texCoords().push_back(glm::vec2(0.f, 0.f));
            geom->texCoords().push_back(glm::vec2(1.f, 0.f));
            geom->texCoords().push_back(glm::vec2(1.f, 1.f));
            geom->colors().assign(4, glm::vec4(1.f));
            geom->normals().assign(4, glm::vec3(0, 0, 1));
            geom->computeBoundingBox();
            geom->computeTangents();
            quad_mesh = gl::Mesh::create(geom, Material::create());
            quad_mesh->setPosition(glm::vec3(0.5f, 0.5f , 0.f));
        }
        quad_mesh->geometry()->setPrimitiveType(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
        quad_mesh->material() = theMaterial;
        float scaleX = (x1 - x0) / g_viewportDim[0];
        float scaleY = (y0 - y1) / g_viewportDim[1];
        mat4 modelViewMatrix = glm::scale(mat4(), vec3(scaleX, scaleY, 1));
        modelViewMatrix[3] = vec4(x0 / g_viewportDim[0], y1 / g_viewportDim[1] , 0, 1);
        gl::loadMatrix(gl::PROJECTION_MATRIX, projectionMatrix);
        gl::loadMatrix(gl::MODEL_VIEW_MATRIX, modelViewMatrix * quad_mesh->transform());
        drawMesh(quad_mesh);
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawText2D(const std::string &theText, const gl::Font &theFont, const Color &the_color,
                    const glm::vec2 &theTopLeft)
    {
        gl::ScopedMatrixPush model(MODEL_VIEW_MATRIX), projection(PROJECTION_MATRIX);
        
        if(!theFont.glyph_texture()) return;
        mat4 projectionMatrix = ortho(0.0f, g_viewportDim[0], 0.0f, g_viewportDim[1], 0.0f, 1.0f);
        
        if(g_string_mesh_map.find(theText) == g_string_mesh_map.end())
        {
            g_string_mesh_map[theText] = string_mesh_container(theText,
                                                               theFont.create_mesh(theText, the_color));
        }
        string_mesh_container &item = g_string_mesh_map[theText];
        item.counter++;
        gl::MeshPtr m = item.mesh;
        m->material()->setDiffuse(the_color);
        m->material()->setDepthTest(false);
        m->setPosition(glm::vec3(theTopLeft.x, g_viewportDim[1] - theTopLeft.y -
                                 m->geometry()->boundingBox().height(), 0.f));
        gl::loadMatrix(gl::PROJECTION_MATRIX, projectionMatrix);
        gl::loadMatrix(gl::MODEL_VIEW_MATRIX, m->transform());
        drawMesh(m);
        //drawAxes(m);
        
        // free the less frequent used half of our buffered string-meshes
        if(g_string_mesh_map.size() >= STRING_MESH_BUFFER_SIZE)
        {
            LOG_TRACE<<"font-mesh buffersize: "<<STRING_MESH_BUFFER_SIZE<<" -> clearing ...";
            std::list<string_mesh_container> tmp_list;
            std::map<std::string, string_mesh_container>::iterator it = g_string_mesh_map.begin();
            for (; it != g_string_mesh_map.end(); ++it){tmp_list.push_back(it->second);}
            tmp_list.sort();
            
            std::list<string_mesh_container>::reverse_iterator list_it = tmp_list.rbegin();
            g_string_mesh_map.clear();
            
            for (uint32_t i = 0; i < tmp_list.size() / 2; i++, ++list_it)
            {
                g_string_mesh_map[list_it->text] = *list_it;
            }
        }
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawText3D(const std::string &theText, const gl::Font &theFont)
    {
        if(!theFont.glyph_texture()) return;
        
        if(g_string_mesh_map.find(theText) == g_string_mesh_map.end())
        {
            g_string_mesh_map[theText] = string_mesh_container(theText,
                                                               theFont.create_mesh(theText));
        }
        string_mesh_container &item = g_string_mesh_map[theText];
        item.counter++;
        gl::MeshPtr m = item.mesh;
        drawMesh(m);
        
        // free the less frequent used half of our buffered string-meshes
        if(g_string_mesh_map.size() >= STRING_MESH_BUFFER_SIZE)
        {
            std::list<string_mesh_container> tmp_list;
            std::map<std::string, string_mesh_container>::iterator it = g_string_mesh_map.begin();
            for (; it != g_string_mesh_map.end(); ++it){tmp_list.push_back(it->second);}
            tmp_list.sort();
            
            std::list<string_mesh_container>::reverse_iterator list_it = tmp_list.rbegin();
            g_string_mesh_map.clear();
            
            for (uint32_t i = 0; i < tmp_list.size() / 2; i++, ++list_it)
            {
                g_string_mesh_map[list_it->text] = *list_it;
            }
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawGrid(float width, float height, int numW, int numH)
    {
        static std::map<std::tuple<float,float,int,int>, MeshPtr> theMap;
        static vec4 colorGrey(.7, .7, .7, 1.0), colorRed(1.0, 0, 0 ,1.0), colorBlue(0, 0, 1.0, 1.0);
        
        // search for incoming key
        auto conf = std::make_tuple(width, height, numW, numH);

        if(theMap.find(conf) == theMap.end())
        {
            GeometryPtr geom = Geometry::create();
            geom->setPrimitiveType(GL_LINES);
            gl::MaterialPtr mat(new gl::Material);
            MeshPtr mesh (gl::Mesh::create(geom, mat));
            
            vector<vec3> &points = geom->vertices();
            vector<vec4> &colors = geom->colors();
            
            float stepX = width / numW, stepZ = height / numH;
            float w2 = width / 2.f, h2 = height / 2.f;
            
            glm::vec4 *color;
            for (int x = 0; x < numW + 1; x ++ )
            {
                if(x == 0) color = &colorBlue;
                else color = &colorGrey;
                
                // line Z
                points.push_back(vec3(- w2 + x * stepX, 0.f, -h2));
                points.push_back(vec3(- w2 + x * stepX, 0.f, h2));
                colors.push_back(*color);
                colors.push_back(*color);
            }
            for (int z = 0; z < numH + 1; z++ )
            {
                if(z == 0) color = &colorRed;
                else color = &colorGrey;
                
                // line X
                points.push_back(vec3(- w2 , 0.f, -h2 + z * stepZ));
                points.push_back(vec3( w2 , 0.f, -h2 + z * stepZ));
                colors.push_back(*color);
                colors.push_back(*color);
            }
            
            theMap.clear();
            
            geom->createGLBuffers();
            mesh->createVertexArray();
            
            theMap[conf] = mesh;
        }
        drawMesh(theMap[conf]);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawAxes(const MeshWeakPtr &weakMesh)
    {
        static map<MeshWeakPtr, MeshPtr, std::owner_less<MeshWeakPtr> > theMap;
        static vec4 colorRed(1.0, 0, 0 ,1.0), colorGreen(0, 1.0, 0 ,1.0), colorBlue(0, 0, 1.0, 1.0);
        
        if(theMap.find(weakMesh) == theMap.end())
        {
            Mesh::ConstPtr m = weakMesh.lock();
            if(!m) return;
            
            GeometryPtr geom = Geometry::create();
            geom->setPrimitiveType(GL_LINES);
            gl::MaterialPtr mat(new gl::Material);
            MeshPtr line_mesh (gl::Mesh::create(geom, mat));
            AABB bb = m->boundingBox();
            vector<vec3> &thePoints = geom->vertices();
            vector<vec4> &theColors = geom->colors();
            float axis_length = std::max(bb.width(), bb.height());
            axis_length = std::max(axis_length, bb.depth());
            
            thePoints.push_back(vec3(0));
            thePoints.push_back(vec3(axis_length, 0, 0));
            theColors.push_back(colorRed);
            theColors.push_back(colorRed);
            
            thePoints.push_back(vec3(0));
            thePoints.push_back(vec3(0, axis_length, 0));
            theColors.push_back(colorGreen);
            theColors.push_back(colorGreen);
            
            thePoints.push_back(vec3(0));
            thePoints.push_back(vec3(0, 0, axis_length));
            theColors.push_back(colorBlue);
            theColors.push_back(colorBlue);
            
            geom->createGLBuffers();
            line_mesh->createVertexArray();
            theMap[weakMesh] = line_mesh;
        }
        gl::drawMesh(theMap[weakMesh]);
        
        // cleanup
        map<MeshWeakPtr, MeshPtr >::iterator meshIt = theMap.begin();
        for (; meshIt != theMap.end(); ++meshIt)
        {
            if(! meshIt->first.lock() )
                theMap.erase(meshIt);
        }
    }

///////////////////////////////////////////////////////////////////////////////
    
void drawTransform(const glm::mat4& the_transform, float the_scale)
{
    static gl::MeshPtr transform_mesh;
    
    if(!transform_mesh)
    {
        transform_mesh = gl::Mesh::create(gl::Geometry::create(), gl::Material::create());
        auto &verts = transform_mesh->geometry()->vertices();
        auto &colors = transform_mesh->geometry()->colors();
        transform_mesh->geometry()->setPrimitiveType(GL_LINES);
        verts =
        {
            glm::vec3(0), gl::X_AXIS,
            glm::vec3(0), gl::Y_AXIS,
            glm::vec3(0), gl::Z_AXIS
        };
        colors =
        {
            gl::COLOR_RED, gl::COLOR_RED,
            gl::COLOR_GREEN, gl::COLOR_GREEN,
            gl::COLOR_BLUE, gl::COLOR_BLUE
        };
        transform_mesh->createVertexArray();
    }
    gl::ScopedMatrixPush sp(gl::MODEL_VIEW_MATRIX);
    gl::multMatrix(gl::MODEL_VIEW_MATRIX, glm::scale(the_transform, glm::vec3(the_scale)));
    gl::drawMesh(transform_mesh);
}
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawMesh(const MeshPtr &theMesh)
    {
        if(!theMesh || theMesh->geometry()->vertices().empty()) return;
        
        for(auto &mat : theMesh->materials())
        {
            mat->uniform("u_modelViewMatrix", g_modelViewMatrixStack.top());
            if(theMesh->geometry()->hasNormals())
            {
                mat->uniform("u_normalMatrix",
                             glm::inverseTranspose( glm::mat3(g_modelViewMatrixStack.top()) ));
            }
            mat->uniform("u_modelViewProjectionMatrix",
                         g_projectionMatrixStack.top() * g_modelViewMatrixStack.top());
            
            if(theMesh->geometry()->hasBones())
            {
                mat->uniform("u_bones", theMesh->boneMatrices());
            }
            
            GLuint block_index = mat->shader().getUniformBlockIndex("MaterialBlock");
            glUniformBlockBinding(mat->shader().getHandle(), block_index, 0);
        }
        gl::apply_material(theMesh->material());
        KINSKI_CHECK_GL_ERRORS();
        
#ifndef KINSKI_NO_VAO
        theMesh->bind_vertex_array();
#else
        theMesh->bindVertexPointers();
#endif
        
        if(theMesh->geometry()->hasIndices())
        {
#ifndef KINSKI_GLES
            if(!theMesh->entries().empty())
            {
                for (int i = 0; i < theMesh->entries().size(); i++)
                {
                    // skip disabled entries
                    if(!theMesh->entries()[i].enabled) continue;
                    
                    int mat_index = clamp<int>(theMesh->entries()[i].material_index,
                                               0,
                                               theMesh->materials().size() - 1);
                    theMesh->bind_vertex_array(mat_index);
                    apply_material(theMesh->materials()[mat_index]);
                    
                    glDrawElementsBaseVertex(theMesh->geometry()->primitiveType(),
                                             theMesh->entries()[i].num_indices,
                                             theMesh->geometry()->indexType(),
                                             BUFFER_OFFSET(theMesh->entries()[i].base_index *
                                                           sizeof(theMesh->geometry()->indexType())),
                                             theMesh->entries()[i].base_vertex);
                }
            }
            else
#endif
            {
                glDrawElements(theMesh->geometry()->primitiveType(),
                               theMesh->geometry()->indices().size(), theMesh->geometry()->indexType(),
                               BUFFER_OFFSET(0));
            }
        }
        else
        {
            glDrawArrays(theMesh->geometry()->primitiveType(), 0,
                         theMesh->geometry()->vertices().size());
        }
        KINSKI_CHECK_GL_ERRORS();
    
#ifndef KINSKI_NO_VAO
        GL_SUFFIX(glBindVertexArray)(0);
#endif
    
        KINSKI_CHECK_GL_ERRORS();
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawLight(const LightPtr &theLight)
    {
        static gl::MeshPtr directional_mesh, point_mesh, spot_mesh;
        

        gl::ScopedMatrixPush mat_push(gl::MODEL_VIEW_MATRIX);
        gl::multMatrix(gl::MODEL_VIEW_MATRIX, theLight->global_transform());
        
        if(!directional_mesh)
        {
            directional_mesh = gl::Mesh::create(gl::Geometry::create(), gl::Material::create());
            point_mesh = gl::Mesh::create(gl::Geometry::createSphere(5.f, 8),
                                          gl::Material::create());
            spot_mesh = gl::Mesh::create(gl::Geometry::createCone(5.f, 10.f, 8),
                                         gl::Material::create());
            
            glm::mat4 rot_spot_mat = glm::rotate(glm::mat4(), 90.f, gl::X_AXIS);
            
            for(auto &vert : spot_mesh->geometry()->vertices())
            {
                vert = (rot_spot_mat * glm::vec4(vert, 1.f)).xyz();
            }
            spot_mesh->geometry()->createGLBuffers();
            spot_mesh->createVertexArray();
            
            std::list<gl::MaterialPtr> mats =
            {
                directional_mesh->material(),
                point_mesh->material(),
                spot_mesh->material()
                
            };
            for (auto mat : mats)
            {
                mat->setWireframe();
            }
            
        }
        
        gl::MeshPtr light_mesh;
        
        switch (theLight->type())
        {
            case gl::Light::DIRECTIONAL:
                light_mesh = directional_mesh;
                break;
                
            case gl::Light::POINT:
                light_mesh = point_mesh;
                break;
                
            case gl::Light::SPOT:
                light_mesh = spot_mesh;
                break;
        }
        
        if(theLight->enabled())
        {
            light_mesh->material()->setDiffuse(theLight->diffuse());
        }
        else
        {
//            light_mesh->material()->setDiffuse(gl::COLOR_RED);
            return;
        }
        
        // draw the configured mesh
        gl::drawMesh(light_mesh);
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawBoundingBox(const Object3DPtr &the_obj)
    {
        static vec4 colorWhite(1.0), colorRed(1.0, 0, 0 ,1.0);
        static MeshPtr line_mesh;
        if(!the_obj) return;

        if(!line_mesh)
        {
            
            GeometryPtr geom = Geometry::create();
            geom->setPrimitiveType(GL_LINES);
            gl::MaterialPtr mat = gl::Material::create();
            line_mesh = gl::Mesh::create(geom, mat);

            auto bb = gl::AABB(vec3(-.5f), vec3(.5f));
            
            vector<vec3> &thePoints = geom->vertices();
            vector<vec4> &theColors = geom->colors();
            
            // bottom
            thePoints.push_back(bb.min);
            thePoints.push_back(vec3(bb.min.x, bb.min.y, bb.max.z));
            
            thePoints.push_back(vec3(bb.min.x, bb.min.y, bb.max.z));
            thePoints.push_back(vec3(bb.max.x, bb.min.y, bb.max.z));
            
            thePoints.push_back(vec3(bb.max.x, bb.min.y, bb.max.z));
            thePoints.push_back(vec3(bb.max.x, bb.min.y, bb.min.z));
            
            thePoints.push_back(vec3(bb.max.x, bb.min.y, bb.min.z));
            thePoints.push_back(bb.min);
            
            // top
            thePoints.push_back(vec3(bb.min.x, bb.max.y, bb.min.z));
            thePoints.push_back(vec3(bb.min.x, bb.max.y, bb.max.z));
            
            thePoints.push_back(vec3(bb.min.x, bb.max.y, bb.max.z));
            thePoints.push_back(vec3(bb.max.x, bb.max.y, bb.max.z));
            
            thePoints.push_back(vec3(bb.max.x, bb.max.y, bb.max.z));
            thePoints.push_back(vec3(bb.max.x, bb.max.y, bb.min.z));
            
            thePoints.push_back(vec3(bb.max.x, bb.max.y, bb.min.z));
            thePoints.push_back(vec3(bb.min.x, bb.max.y, bb.min.z));
            
            //sides
            thePoints.push_back(vec3(bb.min.x, bb.min.y, bb.min.z));
            thePoints.push_back(vec3(bb.min.x, bb.max.y, bb.min.z));
            
            thePoints.push_back(vec3(bb.min.x, bb.min.y, bb.max.z));
            thePoints.push_back(vec3(bb.min.x, bb.max.y, bb.max.z));
            
            thePoints.push_back(vec3(bb.max.x, bb.min.y, bb.max.z));
            thePoints.push_back(vec3(bb.max.x, bb.max.y, bb.max.z));
            
            thePoints.push_back(vec3(bb.max.x, bb.min.y, bb.min.z));
            thePoints.push_back(vec3(bb.max.x, bb.max.y, bb.min.z));
            
            for (int i = 0; i < 24; i++)
                theColors.push_back(colorWhite);
                
            geom->createGLBuffers();
            line_mesh->createVertexArray();
        }
        AABB mesh_bb = the_obj->boundingBox();
        glm::mat4 center_mat = glm::translate(glm::mat4(), mesh_bb.center());
        
        glm::mat4 scale_mat = glm::scale(glm::mat4(), vec3(mesh_bb.width(),
                                                           mesh_bb.height(),
                                                           mesh_bb.depth()));
        
    
        gl::ScopedMatrixPush sp(gl::MODEL_VIEW_MATRIX);
        gl::multMatrix(gl::MODEL_VIEW_MATRIX, center_mat * scale_mat);
        gl::drawMesh(line_mesh);
    }

///////////////////////////////////////////////////////////////////////////////
    
    void drawNormals(const MeshWeakPtr &theMesh)
    {
        static map<MeshWeakPtr, MeshPtr, std::owner_less<MeshWeakPtr> > theMap;
        static vec4 colorGrey(.7, .7, .7, 1.0), colorRed(1.0, 0, 0 ,1.0), colorBlue(0, 0, 1.0, 1.0);
        
        if(theMap.find(theMesh) == theMap.end())
        {
            Mesh::ConstPtr m = theMesh.lock();
            if(m->geometry()->normals().empty()) return;
            GeometryPtr geom = Geometry::create();
            geom->setPrimitiveType(GL_LINES);
            gl::MaterialPtr mat(new gl::Material);
            MeshPtr line_mesh = gl::Mesh::create(geom, mat);
            vector<vec3> &thePoints = geom->vertices();
            vector<vec4> &theColors = geom->colors();
            const vector<vec3> &vertices = m->geometry()->vertices();
            const vector<vec3> &normals = m->geometry()->normals();
            
            float length = (m->geometry()->boundingBox().max -
                            m->geometry()->boundingBox().min).length() * 5;
            
            for (uint32_t i = 0; i < vertices.size(); i++)
            {
                thePoints.push_back(vertices[i]);
                thePoints.push_back(vertices[i] + normals[i] * length);
                theColors.push_back(colorGrey);
                theColors.push_back(colorRed);
            }
            geom->createGLBuffers();
            line_mesh->createVertexArray();
            theMap[theMesh] = line_mesh;
        }
        gl::drawMesh(theMap[theMesh]);
        
        // cleanup
        map<MeshWeakPtr, MeshPtr >::iterator meshIt = theMap.begin();
        for (; meshIt != theMap.end(); ++meshIt)
        {
            if(! meshIt->first.lock() )
                theMap.erase(meshIt);
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void drawCircle(const glm::vec2 &center, float radius, bool solid,
                         const MaterialPtr &theMaterial, int numSegments)
    {
        static gl::MeshPtr solid_mesh, line_mesh;
        static gl::MaterialPtr default_mat;
        gl::MeshPtr our_mesh = solid ? solid_mesh : line_mesh;
        
        if(!our_mesh)
        {
            GeometryPtr geom = solid ? Geometry::createSolidUnitCircle(numSegments) :
                Geometry::createUnitCircle(numSegments);
            default_mat = gl::Material::create();
            default_mat->setDepthWrite(false);
            our_mesh = gl::Mesh::create(geom, default_mat);
            if(solid)
                solid_mesh = our_mesh;
            else
                line_mesh = our_mesh;
        }
        our_mesh->material() = theMaterial ? theMaterial : default_mat;
        mat4 projectionMatrix = ortho(0.0f, g_viewportDim[0], 0.0f, g_viewportDim[1], 0.0f, 1.0f);
        mat4 modelView = glm::scale(mat4(), vec3(radius));
        modelView[3].xyz() = vec3(center.x, g_viewportDim[1] - center.y, 0);
        
        ScopedMatrixPush m(MODEL_VIEW_MATRIX), p(PROJECTION_MATRIX);
        loadMatrix(PROJECTION_MATRIX, projectionMatrix);
        loadMatrix(MODEL_VIEW_MATRIX, modelView);
        drawMesh(our_mesh);
    }

///////////////////////////////////////////////////////////////////////////////
    
    KINSKI_API gl::Texture render_to_texture(const gl::Scene &theScene, gl::Fbo &theFbo,
                                             const gl::CameraPtr &theCam)
    {
        if(!theFbo)
        {
            LOG_WARNING << "trying to use an uninitialized FBO";
            return gl::Texture();
        }
        
        // push framebuffer and viewport states
        gl::SaveViewPort sv; gl::SaveFramebufferBinding sfb;
        gl::setWindowDimension(theFbo.getSize());
        theFbo.bindFramebuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        theScene.render(theCam);
        return theFbo.getTexture();
    }
    
#ifdef KINSKI_CPP11
    
    KINSKI_API gl::Texture render_to_texture(gl::Fbo &theFbo, std::function<void()> functor)
    {
        // push framebuffer and viewport states
        gl::SaveViewPort sv; gl::SaveFramebufferBinding sfb;
        gl::setWindowDimension(theFbo.getSize());
        theFbo.bindFramebuffer();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        functor();
        return theFbo.getTexture();
    }
    
#endif
    
///////////////////////////////////////////////////////////////////////////////
    
    void apply_material(const MaterialPtr &the_mat, bool force_apply)
    {
        static Material::WeakPtr weak_last;
        
        MaterialPtr last_mat = force_apply ? MaterialPtr() : weak_last.lock();

        if(!the_mat) return;
        
        // weak copy of current mat
        weak_last = the_mat;
        
        the_mat->shader().bind();
        
        char buf[512];
        
        // twoSided
        if(!last_mat || (last_mat->twoSided() != the_mat->twoSided() ||
                         last_mat->wireframe() != the_mat->wireframe()))
        {
            if(the_mat->twoSided() || the_mat->wireframe()) { glDisable(GL_CULL_FACE); }
            else
            {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }
        }
        KINSKI_CHECK_GL_ERRORS();
        
        // wireframe ?
#ifndef KINSKI_GLES
        if(!last_mat || last_mat->wireframe() != the_mat->wireframe())
            glPolygonMode(GL_FRONT_AND_BACK, the_mat->wireframe() ? GL_LINE : GL_FILL);
#endif
        
        KINSKI_CHECK_GL_ERRORS();
        
        // read write depth buffer ?
        if(!last_mat || last_mat->depthTest() != the_mat->depthTest())
        {
            if(the_mat->depthTest()) { glEnable(GL_DEPTH_TEST); }
            else { glDisable(GL_DEPTH_TEST); }
        }
        KINSKI_CHECK_GL_ERRORS();
        
        if(!last_mat || last_mat->depthWrite() != the_mat->depthWrite())
        {
            if(the_mat->depthWrite()) glDepthMask(GL_TRUE);
            else glDepthMask(GL_FALSE);
        }
        KINSKI_CHECK_GL_ERRORS();
        
        if(!last_mat || last_mat->blending() != the_mat->blending())
        {
            if(!the_mat->blending())
            {
                glDisable(GL_BLEND);
            }
            else
            {
                glEnable(GL_BLEND);
                glBlendFunc(the_mat->blend_src(), the_mat->blend_dst());
#ifndef KINSKI_GLES
                glBlendEquation(the_mat->blend_equation());
#endif
            }
        }
        KINSKI_CHECK_GL_ERRORS();
        
        if(!last_mat || last_mat->pointSize() != the_mat->pointSize())
        {
            if(the_mat->pointSize() > 0.f)
            {
#ifndef KINSKI_GLES
                glEnable(GL_PROGRAM_POINT_SIZE);
                glPointSize(the_mat->pointSize());
#endif
                KINSKI_CHECK_GL_ERRORS();
            }
        }
        // texture matrix from first texture, if any
        the_mat->shader().uniform("u_textureMatrix",
                         (the_mat->textures().empty() || !the_mat->textures().front()) ?
                                  glm::mat4() : the_mat->textures().front().getTextureMatrix());
        
        the_mat->shader().uniform("u_numTextures", (GLint) the_mat->textures().size());
        
        if(the_mat->textures().empty()) glBindTexture(GL_TEXTURE_2D, 0);
        
        // add texturemaps
        uint32_t tex_unit = 0, tex_2d = 0, tex_rect = 0, tex_3d = 0, tex_2d_array = 0;
        
        for(auto &t : the_mat->textures())
        {
            if(!t){ continue; }
            
            t.bind(tex_unit);
            
            switch (t.getTarget())
            {
                case GL_TEXTURE_2D:
                    sprintf(buf, "u_sampler_2D[%d]", tex_2d++);
                    break;
                    
#if !defined(KINSKI_GLES)

                case GL_TEXTURE_RECTANGLE:
                    sprintf(buf, "u_sampler_2Drect[%d]", tex_rect++);
                    break;
                    
                case GL_TEXTURE_3D:
                    sprintf(buf, "u_sampler_3D[%d]", tex_3d++);
                    break;
                    
                case GL_TEXTURE_2D_ARRAY:
                    sprintf(buf, "u_sampler_2D_array[%d]", tex_2d_array++);
                    break;
#endif
                default:
                    break;
            }
            the_mat->shader().uniform(buf, tex_unit);
            tex_unit++;
        }
        
        KINSKI_CHECK_GL_ERRORS();
        
        the_mat->update_uniform_buffer();
        
        // set all other uniform values
        Material::UniformMap::const_iterator it = the_mat->uniforms().begin();
        for (; it != the_mat->uniforms().end(); it++)
        {
            boost::apply_visitor(InsertUniformVisitor(the_mat->shader(), it->first), it->second);
            KINSKI_CHECK_GL_ERRORS();
        }
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    const std::set<std::string>& getExtensions()
    {
        static std::set<std::string> s_extensions;
        
        if(s_extensions.empty())
        {
#ifndef KINSKI_GLES
            GLint numExtensions = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions) ;
            for (int i = 0; i < numExtensions; ++i) {
                s_extensions.insert((char*)glGetStringi(GL_EXTENSIONS, i)) ;
            }
#endif
        }
        return s_extensions;
    }

///////////////////////////////////////////////////////////////////////////////
    
    bool isExtensionSupported(const std::string &theName)
    {
        return getExtensions().find(theName) != getExtensions().end();
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    // SaveFramebufferBinding
    SaveFramebufferBinding::SaveFramebufferBinding()
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_old_value );
    }
    
    SaveFramebufferBinding::~SaveFramebufferBinding()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_old_value );
    }

///////////////////////////////////////////////////////////////////////////////
    
    bool is_point_inside_mesh(const glm::vec3& p, gl::MeshPtr m)
    {
        // checks only make sense with triangle geometry
        if(!m ||
           m->geometry()->primitiveType() != GL_TRIANGLES ||
           m->geometry()->faces().empty())
        {
            return false;
        }
        
        auto aabb = m->boundingBox().transform(m->global_transform());
        
        // checks if p is inside the (transformed aabb of Mesh m)
        if(!aabb.contains(p)) return false;
        
        const auto &vertices = m->geometry()->vertices();
        
        // check the point's distance to all triangle planes
        for (const auto &face : m->geometry()->faces())
        {
            gl::Plane plane(vertices[face.a], vertices[face.b], vertices[face.c]);
            plane.transform(m->transform());
            if(plane.distance(p) < 0)
                return false;
        }
        return true;
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    void project_texcoords(gl::MeshPtr src, gl::MeshPtr dest)
    {
        const auto &src_verts = src->geometry()->vertices();
        const auto &src_texcoords = src->geometry()->texCoords();
        const auto &dest_verts = dest->geometry()->vertices();
        const auto &dest_normals = dest->geometry()->normals();
        auto &dest_texcoords = dest->geometry()->texCoords();
        
        // aquire enough space for texcoords
        dest_texcoords.resize(dest_verts.size());
        
        // helper structure and comparator for sorting
        struct hit_struct
        {
            gl::Face3 face;
            float u, v, distance;
        };
        auto hit_struct_comp = [](const hit_struct &h1, const hit_struct &h2) -> bool
        {
            return h1.distance < h2.distance;
        };
        
        float ray_offset = 2 * glm::length(src->boundingBox().transform(src->global_transform()).halfExtents());
        
        for(int i = 0; i < dest_verts.size(); i++)
        {
            gl::Ray ray(dest_verts[i] + dest_normals[i] * ray_offset, -dest_normals[i]);
            ray = ray.transform(dest->transform());
            
            float scale_val = 1.01f;
            gl::Ray ray_in_object_space = ray.transform(glm::inverse(glm::scale(src->global_transform(),
                                                                                vec3(scale_val))));
            
            std::vector<hit_struct> hit_structs;
            
            for (const auto &face : src->geometry()->faces())
            {
                gl::Triangle t(src_verts[face.a], src_verts[face.b], src_verts[face.c]);
                
                if(gl::ray_triangle_intersection ray_tri_hit = t.intersect(ray_in_object_space))
                {
                    hit_structs.push_back({face, ray_tri_hit.u, ray_tri_hit.v, ray_tri_hit.distance});
                }
            }
            if(!hit_structs.empty())
            {
                std::sort(hit_structs.begin(), hit_structs.end(), hit_struct_comp);
                const auto & hs = hit_structs.front();
                float u, v, w;
                u = hs.u, v = hs.v, w = 1 - u - v;
                
                dest_texcoords[i] = src_texcoords[hs.face.a] * v +
                src_texcoords[hs.face.b] * u +
                src_texcoords[hs.face.c] * w;
                
                dest_texcoords[i] = dest_texcoords[i].yx();
                dest_texcoords[i].x = 1 - dest_texcoords[i].x;
                dest_texcoords[i].y = 1 - dest_texcoords[i].y;
                
            }else{ LOG_ERROR << "no triangle hit"; }
        }
        dest->geometry()->createGLBuffers();
    }
    
///////////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////
    // global shader creation functions
    
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
        Shader ret;
        
        const char *vert_src = nullptr, *frag_src = nullptr, *geom_src = nullptr;
        
        switch (type)
        {
            case SHADER_UNLIT:
                vert_src = unlit_vert;
                frag_src = unlit_frag;
                break;
    
            case SHADER_GOURAUD:
                vert_src = gouraud_vert;
                frag_src = gouraud_frag;
                break;
    
            case SHADER_PHONG:
                vert_src = phong_vert;
                frag_src = phong_frag;
                break;
#if !defined(KINSKI_GLES)
            
            case SHADER_2D_RECT:
                vert_src = unlit_rect_vert;
                frag_src = unlit_rect_frag;
                break;
                
            case SHADER_PHONG_NORMALMAP:
                vert_src = phong_normalmap_vert;
                frag_src = phong_normalmap_frag;
                break;
    
            case SHADER_PHONG_SKIN:
                vert_src = phong_skin_vert;
                frag_src = phong_frag;
                break;
    
            case SHADER_LINES_2D:
                vert_src = unlit_vert;
                frag_src = unlit_frag;
                geom_src = lines_2D_geom;
                break;
            
            case SHADER_POINTS_COLOR:
            case SHADER_POINTS_TEXTURE:
                vert_src = points_vert;
                frag_src = points_frag;
                break;
                
            case SHADER_POINTS_SPHERE:
                vert_src = points_vert;
                frag_src = points_sphere_frag;
                break;
#endif
            default:
                break;
        }
        ret.loadFromData(vert_src, frag_src, geom_src);
        KINSKI_CHECK_GL_ERRORS();
        return ret;
    }
    
}}//namespace
