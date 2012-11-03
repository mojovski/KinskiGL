#include "kinskiGL/App.h"

#include "kinskiGL/Scene.h"
#include "kinskiGL/Mesh.h"

#include "TextureIO.h"

#include "kinskiCV/CVThread.h"

using namespace std;
using namespace kinski;
using namespace glm;

class SimpleGeometryApp : public App
{
private:
    
    gl::Material::Ptr m_material;
    gl::Geometry::Ptr m_geometry;
    
    gl::Mesh::Ptr m_mesh;
    
    gl::PerspectiveCamera::Ptr m_Camera;
    
    gl::Scene m_scene;
    
    RangedProperty<float>::Ptr m_distance;
    
    RangedProperty<float>::Ptr m_textureMix;
    Property_<bool>::Ptr m_wireFrame;
    
    Property_<glm::vec4>::Ptr m_color;
    
    Property_<glm::mat3>::Ptr m_rotation;
    RangedProperty<float>::Ptr m_rotationSpeed;
    
    CVThread::Ptr m_cvThread;

    void drawLine(const vec2 &a, const vec2 &b)
    {
        
    }
    
    void drawQuad(gl::Material &theMaterial,
                  const vec2 &theSize,
                  const vec2 &theTl = vec2(0))
    {
        vec2 sz = theSize;
        vec2 tl = theTl == vec2(0) ? vec2(0, getHeight()) : theTl;
        drawQuad(theMaterial, tl[0], tl[1], (tl+sz)[0], tl[1]-sz[1]);
    }
    
    
    void drawQuad(gl::Material &theMaterial,
                  float x0, float y0, float x1, float y1)
    {
        // orthographic projection with a [0,1] coordinate space
        static mat4 projectionMatrix = ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        
        float scaleX = (x1 - x0) / getWidth();
        float scaleY = (y0 - y1) / getHeight();
        
        mat4 modelViewMatrix = glm::scale(mat4(), vec3(scaleX, scaleY, 1));
        modelViewMatrix[3] = vec4(x0 / getWidth(), y1 / getHeight() , 0, 1);
        
        theMaterial.uniform("u_modelViewProjectionMatrix", projectionMatrix * modelViewMatrix);
        
        theMaterial.apply();
        
        static GLuint canvasVAO = 0;
        
        if(!canvasVAO)
        {
            //GL_T2F_V3F
            const GLfloat array[] ={0.0,0.0,0.0,0.0,0.0,
                                    1.0,0.0,1.0,0.0,0.0,
                                    1.0,1.0,1.0,1.0,0.0,
                                    0.0,1.0,0.0,1.0,0.0};
            
            // create VAO to record all VBO calls
            glGenVertexArrays(1, &canvasVAO);
            glBindVertexArray(canvasVAO);
            
            GLuint canvasBuffer;
            glGenBuffers(1, &canvasBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, canvasBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(array), array, GL_STATIC_DRAW);
            
            GLsizei stride = 5 * sizeof(GLfloat);
            
            GLuint vertexAttribLocation = theMaterial.getShader().getAttribLocation("a_vertex");
            glEnableVertexAttribArray(vertexAttribLocation);
            glVertexAttribPointer(vertexAttribLocation, 3, GL_FLOAT, GL_FALSE,
                                  stride, BUFFER_OFFSET(2 * sizeof(GLfloat)));
            
            GLuint texCoordAttribLocation = theMaterial.getShader().getAttribLocation("a_texCoord");
            glEnableVertexAttribArray(texCoordAttribLocation);
            glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE,
                                  stride, BUFFER_OFFSET(0));
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glBindVertexArray(0);
        }
        
        glBindVertexArray(canvasVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

public:
    
    void setup()
    {
        glClearColor(0, 0, 0, 1);
        
        /*********** init our application properties ******************/
        
        m_distance = RangedProperty<float>::create("view distance", 25, -50, 50);
        registerProperty(m_distance);
        
        m_textureMix = RangedProperty<float>::create("texture mix ratio", 0.2, 0, 1);
        registerProperty(m_textureMix);
        
        m_wireFrame = Property_<bool>::create("Wireframe", false);
        registerProperty(m_wireFrame);
        
        m_color = Property_<glm::vec4>::create("Material color", glm::vec4(1 ,1 ,0, 0.6));
        registerProperty(m_color);
        
        m_rotation = Property_<glm::mat3>::create("Geometry Rotation", glm::mat3());
        registerProperty(m_rotation);
        
        m_rotationSpeed = RangedProperty<float>::create("Rotation Speed", 40, -100, 100);
        registerProperty(m_rotationSpeed);
        
        // add properties
        addPropertyListToTweakBar(getPropertyList());

        // enable observer mechanism
        observeProperties();
        
        /********************** construct a simple scene ***********************/
        
        m_material = gl::Material::Ptr(new gl::Material);
        m_material->uniform("u_textureMix", m_textureMix->val());
        m_material->setDiffuse(m_color->val());
        
        try
        {
            m_material->getShader().loadFromFile("shader_vert.glsl", "shader_frag.glsl");
        }catch (std::exception &e)
        {
            fprintf(stderr, "%s\n",e.what());
        }
        
        m_geometry =  gl::Geometry::Ptr( new gl::Plane(20, 20, 100, 100) );
        m_geometry->createGLBuffers();
        
        m_mesh = gl::Mesh::Ptr(new gl::Mesh(m_geometry, m_material));
        
        m_scene.addObject(m_mesh);
        
        m_Camera = gl::PerspectiveCamera::Ptr(new gl::PerspectiveCamera);
        m_Camera->setPosition( m_rotation->val() * glm::vec3(0, 0, m_distance->val()) );
        m_Camera->setLookAt(glm::vec3(0));
        
        m_material->addTexture(TextureIO::loadTexture("/Users/Fabian/Pictures/artOfNoise.png"));
        m_material->addTexture(TextureIO::loadTexture("/Users/Fabian/Pictures/David_Jien_02.png"));
        m_material->setTwoSided();
        
//        m_cvThread = CVThread::Ptr(new CVThread());
//        m_cvThread->streamUSBCamera();
    }
    
    void update(const float timeDelta)
    {

//        if(m_cvThread->hasImage())
//        {
//            vector<cv::Mat> images = m_cvThread->getImages();
//            TextureIO::updateTexture(m_material->getTextures()[0], images[0]);
//        }
        
        *m_rotation = mat3( glm::rotate(mat4(m_rotation->val()),
                                        m_rotationSpeed->val() * timeDelta,
                                        vec3(0, 1, .5)));
    }
    
    void draw()
    {
        gl::Material cloneMat1 = *m_material;
        
        cloneMat1.setDepthWrite(false);
        
        drawQuad(cloneMat1, getWindowSize());

        m_scene.render(m_Camera);
    }
    
    void resize(int w, int h)
    {
        m_Camera->setAspectRatio(getAspectRatio());
    }
    
    void tearDown()
    {
        printf("ciao simple geometry\n");
    }
    
    // Property observer callback
    void updateProperty(const Property::Ptr &theProperty)
    {
        // one of our porperties was changed
        if(theProperty == m_wireFrame)
            m_material->setWireframe(m_wireFrame->val());
        
        else if(theProperty == m_color)
            m_material->setDiffuse(m_color->val());
        
        else if(theProperty == m_textureMix)
            m_material->uniform("u_textureMix", m_textureMix->val());
        
        else if(theProperty == m_distance ||
                theProperty == m_rotation)
        {
            m_Camera->setPosition( m_rotation->val() * glm::vec3(0, 0, m_distance->val()) );
            m_Camera->setLookAt(glm::vec3(0));
        }
    }
};

int main(int argc, char *argv[])
{
    App::Ptr theApp(new SimpleGeometryApp);
    
    return theApp->run();
}
