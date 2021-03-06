#include "frustum_tester.hpp"

using namespace std;
using namespace glm;

namespace kinski
{
    void Frustum_Tester::setup()
    {
        ViewerApp::setup();
        
        /******************** add search paths ************************/
        kinski::addSearchPath("/Library/Fonts");
        m_font.load("Courier New Bold.ttf", 24);
        outstream_gl().set_font(m_font);
        
        /*********** init our application properties ******************/
        m_frustum_rotation = Property_<glm::mat3>::create("Frustum rotation", mat3());
        registerProperty(m_frustum_rotation);
        m_perspective = Property_<bool>::create("Perspective / Ortho", true);
        registerProperty(m_perspective);
        m_near = RangedProperty<float>::create("Near", 10.f, 0, 100);
        registerProperty(m_near);
        m_far = RangedProperty<float>::create("Far", 60.f, 0, 500);
        registerProperty(m_far);
        m_fov = RangedProperty<float>::create("Fov", 45.f, 0, 120);
        registerProperty(m_fov);
        
        create_tweakbar_from_component(shared_from_this());
        
        // enable observer mechanism
        observeProperties();
    
        // load state from config file
        try
        {
            Serializer::loadComponentState(shared_from_this(), "config.json", PropertyIO_GL());
        }catch(Exception &e)
        {
            LOG_WARNING << e.what();
        }
        
        /********************** construct a simple scene ***********************/
        int num_points = 40000;
        gl::GeometryPtr points = gl::Geometry::create();
        points->setPrimitiveType(GL_POINTS);
        points->vertices().reserve(num_points);
        points->colors().reserve(num_points);
        for (int i = 0; i < num_points; i++)
        {
            points->vertices().push_back(glm::linearRand(glm::vec3(-100), glm::vec3(100)));
            points->colors().push_back(glm::vec4(1.f));
            points->point_sizes().push_back(kinski::random(2.f, 3.f));
        }
        
        gl::MaterialPtr mat = gl::Material::create(gl::createShader(gl::SHADER_POINTS_COLOR));
        mat->setPointSize(30.f);
        mat->setPointAttenuation(0.f, 0.01f, 0.f);
        mat->setBlending();
        mat->setDepthWrite(false);
        //mat->uniform("u_pointRadius", 50.f);
        m_point_mesh = gl::Mesh::create(points, mat);
        
        if(*m_perspective)
            m_test_cam = gl::CameraPtr(new gl::PerspectiveCamera(16.f/9.f, *m_fov, *m_near, *m_far));
        else
            m_test_cam = gl::CameraPtr(new gl::OrthographicCamera(-25, 25, -20, 20, *m_near, *m_far));
        
        m_test_cam->setPosition(vec3(0, 0, 50));
        m_frustum_mesh = gl::createFrustumMesh(m_test_cam);
        
        auto poop = [&](float p){LOG_INFO<<"onken";};
        poop(3);
    }
    
    void Frustum_Tester::update(float timeDelta)
    {
        ViewerApp::update(timeDelta);
        
        m_test_cam->setRotation(glm::quat_cast(m_frustum_rotation->value()));
        gl::Frustum f = m_test_cam->frustum();
        for(int i = 0; i < m_point_mesh->geometry()->vertices().size(); i++)
        {
            if(f.intersect(m_point_mesh->geometry()->vertices()[i]))
            {
                m_point_mesh->geometry()->colors()[i] = vec4(1, .7, 0 ,.7);
            }
            else
            {
                m_point_mesh->geometry()->colors()[i] = vec4(.3);
            }
        }
        
        m_point_mesh->geometry()->createGLBuffers();
        m_point_mesh->material()->uniform("u_lightDir", light_direction());
    }
    
    void Frustum_Tester::draw()
    {
        gl::setMatrices(camera());
        if(draw_grid()) gl::drawGrid(50, 50);
        
        gl::drawMesh(m_point_mesh);
        
        gl::loadMatrix(gl::MODEL_VIEW_MATRIX, camera()->getViewMatrix() * m_test_cam->transform());
        gl::drawMesh(m_frustum_mesh);
        
        // draw fps string
        gl::drawText2D(kinski::as_string(framesPerSec()), m_font,
                       vec4(vec3(1) - clear_color().xyz(), 1.f),
                       glm::vec2(windowSize().x - 110, 10));
    }
    
    // Property observer callback
    void Frustum_Tester::updateProperty(const Property::ConstPtr &theProperty)
    {
        ViewerApp::updateProperty(theProperty);
        
        if(theProperty == m_near || theProperty == m_far || theProperty == m_fov ||
           theProperty == m_perspective)
        {
            if(*m_perspective)
                m_test_cam = gl::CameraPtr(new gl::PerspectiveCamera(16.f/9.f, *m_fov, *m_near, *m_far));
            else
                m_test_cam = gl::CameraPtr(new gl::OrthographicCamera(-25, 25, -20, 20, *m_near, *m_far));
            
            m_frustum_mesh = gl::createFrustumMesh(m_test_cam);
            m_test_cam->setPosition(vec3(0, 0, 50));
        }
    }
    
    void Frustum_Tester::tearDown()
    {
        LOG_PRINT<<"ciao frustum tester";
    }
}

int main(int argc, char *argv[])
{
    kinski::App::Ptr theApp(new kinski::Frustum_Tester());
    return theApp->run();
}