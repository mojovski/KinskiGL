#ifndef _KINSKIGL_H
#define _KINSKIGL_H

//#define KINSKI_GLES
#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU

#include <GL/glfw.h>
#include <AntTweakBar.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/random.hpp>

#include "kinskiCore/Definitions.h"
#include "kinskiCore/Exception.h"


namespace kinski { namespace gl {

    // forward declarations
    class Texture;
    class Material;
    class Geometry;
    class Mesh;
    
    enum Matrixtype { MODEL_VIEW_MATRIX = 1 << 0, PROJECTION_MATRIX = 1 << 1};
    void pushMatrix(const Matrixtype type);
    void popMatrix(const Matrixtype type);
    void multMatrix(const glm::mat4 &theMatrix);
    void loadMatrix(const Matrixtype type, const glm::mat4 &theMatrix);
    void getMatrix(const Matrixtype type, glm::mat4 &theMatrix);
    
    class ScopedMatrixPush
    {
    public:
        ScopedMatrixPush(const Matrixtype type):
        m_type(type)
        {pushMatrix(type);}
        
        ~ScopedMatrixPush()
        {popMatrix(m_type);}
    private:
        Matrixtype m_type;
    };
    
    void setWindowDimension(const glm::vec2 &theDim);
    
    template<class T> GLuint createVBO(const std::vector<T> &theVec, GLenum target, GLenum usage);
    
    GLuint createVBO(GLsizei numBytes, GLenum target, GLenum usage, bool initWithZeros = false);
    
    
    /********************************* Drawing Functions *****************************************/
    
    void drawLine(const glm::vec2 &a, const glm::vec2 &b, const glm::vec4 &theColor = glm::vec4(1));
    
    void drawLines(const std::vector<glm::vec3> &thePoints, const glm::vec4 &theColor);
    
    void drawPoints(GLuint thePointVBO, GLsizei theCount,
                    const std::shared_ptr<Material> &theMaterial = std::shared_ptr<Material>(),
                    GLsizei stride = 0,
                    GLsizei offset = 0);
    
    void drawPoints(const std::vector<glm::vec3> &thePoints,
                    const std::shared_ptr<Material> &theMaterial = std::shared_ptr<Material>());
    
    void drawTexture(gl::Texture &theTexture, const glm::vec2 &theSize,
                     const glm::vec2 &theTopLeft = glm::vec2(0));
    
    void drawQuad(gl::Material &theMaterial, const glm::vec2 &theSize,
                  const glm::vec2 &theTopLeft = glm::vec2(0));
    
    void drawQuad(gl::Material &theMaterial, float x0, float y0, float x1, float y1);
    
    void drawGrid(float width, float height, int numW = 20, int numH = 20);
    
    void drawAxes(const std::weak_ptr<Mesh> &theMesh);
    
    void drawMesh(const std::shared_ptr<Mesh> &theMesh);
    
    void drawBoundingBox(const std::weak_ptr<Mesh> &theMesh);
    
    void drawNormals(const std::weak_ptr<Mesh> &theMesh);
    
}}//namespace

#endif //_KINSKIGL_H