#include "kinskiApp/App.h"
#include "kinskiApp/TextureIO.h"

#include "kinskiCV/CVThread.h"

#include "kinskiGL/SerializerGL.h"
#include "kinskiGL/Scene.h"
#include "kinskiGL/Mesh.h"
#include "kinskiGL/Fbo.h"

#include "AssimpConnector.h"

using namespace std;
using namespace kinski;
using namespace glm;

class SimpleGeometryApp : public App
{
private:
    
    gl::Texture m_noiseTexture;
    
    gl::Material::Ptr m_material, m_pointMaterial;
    gl::Geometry::Ptr m_geometry;
    gl::Geometry::Ptr m_straightPlane;
    
    gl::Mesh::Ptr m_mesh;
    gl::PerspectiveCamera::Ptr m_Camera;
    gl::Scene m_scene;
    
    RangedProperty<float>::Ptr m_distance;
    RangedProperty<float>::Ptr m_textureMix;
    Property_<string>::Ptr m_modelPath;
    RangedProperty<float>::Ptr m_animationTime;
    
    Property_<bool>::Ptr m_wireFrame;
    Property_<bool>::Ptr m_drawNormals;
    Property_<glm::vec3>::Ptr m_lightDir;
    
    Property_<glm::vec4>::Ptr m_color;
    Property_<glm::mat3>::Ptr m_rotation;
    RangedProperty<float>::Ptr m_rotationSpeed;
    RangedProperty<float>::Ptr m_simplexDim;
    RangedProperty<float>::Ptr m_simplexSpeed;
    
    // opencv interface
    CVThread::Ptr m_cvThread;
    
    // mouse rotation control
    vec2 m_clickPos;
    mat4 m_lastTransform;
    float m_lastDistance;

public:
    
    void setup()
    {
        /*********** init our application properties ******************/
        
        m_distance = RangedProperty<float>::create("view distance", 25, 0, 5000);
        registerProperty(m_distance);
        
        m_textureMix = RangedProperty<float>::create("texture mix ratio", 0.2, 0, 1);
        registerProperty(m_textureMix);
        
        m_modelPath = Property_<string>::create("Model path", "duck.dae");
        registerProperty(m_modelPath);
        
        m_animationTime = RangedProperty<float>::create("Animation time", 0, 0, 1);
        registerProperty(m_animationTime);
        
        m_wireFrame = Property_<bool>::create("Wireframe", false);
        registerProperty(m_wireFrame);
        
        m_drawNormals = Property_<bool>::create("Normals", false);
        registerProperty(m_drawNormals);
        
        m_lightDir = Property_<vec3>::create("Light dir", vec3(1));
        registerProperty(m_lightDir);
        
        m_color = Property_<glm::vec4>::create("Material color", glm::vec4(1 ,1 ,0, 0.6));
        registerProperty(m_color);
        
        m_rotation = Property_<glm::mat3>::create("Geometry Rotation", glm::mat3());
        registerProperty(m_rotation);
        
        m_rotationSpeed = RangedProperty<float>::create("Rotation Speed", 0, -100, 100);
        registerProperty(m_rotationSpeed);
        
        m_simplexDim = RangedProperty<float>::create("Simplex Resolution", 1/8.f, 0, 2);
        registerProperty(m_simplexDim);
        
        m_simplexSpeed = RangedProperty<float>::create("Simplex Speed", .5, 0, 5);
        registerProperty(m_simplexSpeed);
        
        // add properties
        addPropertyListToTweakBar(getPropertyList());
        
        setBarColor(vec4(0, 0 ,0 , .5));
        setBarSize(ivec2(250, 500));

        // enable observer mechanism
        observeProperties();
        
        /********************** construct a simple scene ***********************/
        
        m_material = gl::Material::Ptr(new gl::Material);
        m_material->uniform("u_textureMix", m_textureMix->val());
        m_material->setDiffuse(m_color->val());
        m_material->addTexture(gl::TextureIO::loadTexture("/Users/Fabian/Pictures/artOfNoise.png"));
        m_material->addTexture(gl::TextureIO::loadTexture("/Users/Fabian/Pictures/David_Jien_02.png"));
        //m_material->setBlending();
        //m_material->setTwoSided();
        
        m_pointMaterial = gl::Material::Ptr(new gl::Material);
        m_pointMaterial->addTexture(gl::TextureIO::loadTexture("smoketex.png"));
        m_pointMaterial->setPointSize(30.f);
        m_pointMaterial->setBlending();
        
        try
        {
            m_material->getShader().loadFromFile("shader_phong_skin.vert", "shader_phong.frag");
        }catch (std::exception &e)
        {
            fprintf(stderr, "%s\n",e.what());
        }
        
        m_Camera = gl::PerspectiveCamera::Ptr(new gl::PerspectiveCamera);
        m_Camera->setClippingPlanes(.1, 5000);
        
//        {
//            int w = 1024, h = 1024;
//            float data[w * h];
//            
//            for (int i = 0; i < h; i++)
//                for (int j = 0; j < w; j++)
//                {
//                    data[i * h + j] = (glm::simplex( vec3( m_simplexDim->val() * vec2(i ,j),
//                                                           m_simplexSpeed->val() * 0.5)) + 1) / 2.f;
//                }
//            
//            m_noiseTexture.update(data, GL_RED, w, h, true);
//        }
        
//        m_cvThread = CVThread::Ptr(new CVThread());
//        m_cvThread->streamUSBCamera();
        
        // load state from config file
        try
        {
            Serializer::loadComponentState(shared_from_this(), "config.json", PropertyIO_GL());
        }catch(FileNotFoundException &e)
        {
            printf("%s\n", e.what());
        }
    }
    
    void update(const float timeDelta)
    {
        *m_rotation = mat3( glm::rotate(mat4(m_rotation->val()),
                                        m_rotationSpeed->val() * timeDelta,
                                        vec3(0, 1, .5)));
        
        if(m_mesh && m_mesh->geometry()->hasBones())
        {
            m_mesh->geometry()->updateAnimation(getApplicationTime() / 5.0f);
//            m_mesh->getGeometry()->updateAnimation(m_animationTime->val() *
//                                                   m_mesh->getGeometry()->animation()->duration);
        }
    }
    
    void draw()
    {
        gl::Material cloneMat1 = *m_material;
        cloneMat1.setDepthWrite(false);
        cloneMat1.setBlending(false);
        cloneMat1.setWireframe(false);
        
        //gl::drawQuad(cloneMat1, getWindowSize() / 1.2f);
        gl::drawTexture(cloneMat1.getTextures()[0], getWindowSize());

        gl::loadMatrix(gl::PROJECTION_MATRIX, m_Camera->getProjectionMatrix());
        gl::loadMatrix(gl::MODEL_VIEW_MATRIX, m_Camera->getViewMatrix());
        gl::drawGrid(500, 500);
        
        m_scene.render(m_Camera);
        
        if(m_mesh)
        {
            gl::loadMatrix(gl::MODEL_VIEW_MATRIX, m_Camera->getViewMatrix() * m_mesh->getTransform());
            gl::drawAxes(m_mesh);
            gl::drawBoundingBox(m_mesh);
            if(m_drawNormals->val()) gl::drawNormals(m_mesh);
            
//            gl::drawPoints(m_mesh->getGeometry()->getInterleavedBuffer(),
//                           m_mesh->getGeometry()->getVertices().size(),
//                           m_pointMaterial,
//                           m_mesh->getGeometry()->getNumComponents() * sizeof(GLfloat),
//                           5 * sizeof(GLfloat));
            
            if(m_mesh->geometry()->hasBones())
            {
                vector<vec3> points;
                buildSkeleton(m_mesh->geometry()->rootBone(), points);
                gl::drawPoints(points);
                gl::drawLines(points, vec4(1, 0, 0, 1));
            }
        }
    }
    
    void buildSkeleton(std::shared_ptr<gl::Bone> currentBone, vector<vec3> &points)
    {
        list<shared_ptr<gl::Bone> >::iterator it = currentBone->children.begin();
        for (; it != currentBone->children.end(); ++it)
        {
            mat4 globalTransform = currentBone->worldtransform;
            mat4 childGlobalTransform = (*it)->worldtransform;
            points.push_back(globalTransform[3].xyz());
            points.push_back(childGlobalTransform[3].xyz());
            
            buildSkeleton(*it, points);
        }
    }
    
    void mousePress(const MouseEvent &e)
    {
        m_clickPos = vec2(e.getX(), e.getY());
        m_lastTransform = mat4(m_rotation->val());
        m_lastDistance = m_distance->val();
    }
    
    void mouseDrag(const MouseEvent &e)
    {
        vec2 mouseDiff = vec2(e.getX(), e.getY()) - m_clickPos;
        if(e.isLeft() && e.isAltDown())
        {
            mat4 mouseRotate = glm::rotate(m_lastTransform, mouseDiff.x, vec3(m_lastTransform[1]) );
            mouseRotate = glm::rotate(mouseRotate, mouseDiff.y, vec3(m_lastTransform[0]) );
            *m_rotation = mat3(mouseRotate);
        }
        else if(e.isRight())
        {
            *m_distance = m_lastDistance + 0.3f * mouseDiff.y;
        }
    }
    
    void keyPress(const KeyEvent &e)
    {
        App::keyPress(e);
        
        switch (e.getChar())
        {
        case KeyEvent::KEY_s:
            Serializer::saveComponentState(shared_from_this(), "config.json", PropertyIO_GL());
            break;
            
        case KeyEvent::KEY_r:
            try
            {
                Serializer::loadComponentState(shared_from_this(), "config.json", PropertyIO_GL());
            }catch(FileNotFoundException &e)
            {
                printf("%s\n", e.what());
            }
            break;
                
        default:
            break;
        }
    }

    void resize(int w, int h)
    {
        m_Camera->setAspectRatio(getAspectRatio());
    }
    
    // Property observer callback
    void updateProperty(const Property::Ptr &theProperty)
    {
        // one of our porperties was changed
        if(theProperty == m_wireFrame)
        {
            if(m_mesh) m_mesh->material()->setWireframe(m_wireFrame->val());
        }
        else if(theProperty == m_lightDir)
        {
            if(m_mesh) m_mesh->material()->uniform("u_lightDir", m_lightDir->val());
        }
        
        else if(theProperty == m_color)
        {
            if(m_mesh)
            {
                m_mesh->material()->setDiffuse(m_color->val());
                m_mesh->material()->setBlending(m_color->val().a < 1.0f);
            }
            m_material->setDiffuse(m_color->val());
            m_pointMaterial->setDiffuse(m_color->val());
            
//            m_pointMaterial->setBlending();
//            m_pointMaterial->setDepthWrite(false);
        }
        else if(theProperty == m_textureMix)
        {
            if(m_mesh) m_mesh->material()->uniform("u_textureMix", m_textureMix->val());
        }
        else if(theProperty == m_distance ||
                theProperty == m_rotation)
        {
            m_Camera->setPosition( m_rotation->val() * glm::vec3(0, 0, m_distance->val()) );
            m_Camera->setLookAt(glm::vec3(0, 100, 0));
        }
        else if(theProperty == m_modelPath)
        {
            try
            {
                m_modelPath->val();
                gl::Mesh::Ptr m = gl::AssimpConnector::loadModel(m_modelPath->val());
                
                m_scene.removeObject(m_mesh);
                m_mesh = m;
                
                m_scene.addObject(m);
                
            } catch (Exception &e)
            {
                cout<<"WARNING: "<< e.what() << endl;
                
                m_modelPath->removeObserver(shared_from_this());
                m_modelPath->val("- not found -");
                m_modelPath->addObserver(shared_from_this());
            }
        }
    }
    
    void tearDown()
    {
        printf("ciao simple geometry\n");
    }
};

int main(int argc, char *argv[])
{
    App::Ptr theApp(new SimpleGeometryApp);
    
    return theApp->run();
}
