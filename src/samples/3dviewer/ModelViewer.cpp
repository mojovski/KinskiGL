//
//  ModelViewer.cpp
//  gl
//
//  Created by Fabian on 29/01/14.
//
//

#include "ModelViewer.h"
#include "AssimpConnector.h"

using namespace std;
using namespace kinski;
using namespace glm;

/////////////////////////////////////////////////////////////////

void ModelViewer::setup()
{
    ViewerApp::setup();
    
    registerProperty(m_model_path);
    observeProperties();
    create_tweakbar_from_component(shared_from_this());
    
    m_light_component = std::make_shared<LightComponent>();
    m_light_component->set_lights(lights());
    create_tweakbar_from_component(m_light_component);
    
    // add lights to scene
    for (auto l : lights()){ scene().addObject(l ); }
    
    load_settings();
    m_light_component->refresh();
}

/////////////////////////////////////////////////////////////////

void ModelViewer::update(float timeDelta)
{
    ViewerApp::update(timeDelta);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::draw()
{
    gl::setMatrices(camera());
    if(draw_grid()){ gl::drawGrid(50, 50); }
    
    if(m_light_component->draw_light_dummies())
    {
        for (auto l : lights()){ gl::drawLight(l); }
    }
    
    scene().render(camera());
    
    // draw texture map(s)
    if(displayTweakBar()){ draw_textures(); }
}

/////////////////////////////////////////////////////////////////

void ModelViewer::resize(int w ,int h)
{
    ViewerApp::resize(w, h);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::keyPress(const KeyEvent &e)
{
    ViewerApp::keyPress(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::keyRelease(const KeyEvent &e)
{
    ViewerApp::keyRelease(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::mousePress(const MouseEvent &e)
{
    ViewerApp::mousePress(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::mouseRelease(const MouseEvent &e)
{
    ViewerApp::mouseRelease(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::mouseMove(const MouseEvent &e)
{
    ViewerApp::mouseMove(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::mouseDrag(const MouseEvent &e)
{
    ViewerApp::mouseDrag(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::mouseWheel(const MouseEvent &e)
{
    ViewerApp::mouseWheel(e);
}

/////////////////////////////////////////////////////////////////

void ModelViewer::got_message(const std::vector<uint8_t> &the_message)
{
    LOG_INFO<<string(the_message.begin(), the_message.end());
}

/////////////////////////////////////////////////////////////////

void ModelViewer::fileDrop(const MouseEvent &e, const std::vector<std::string> &files)
{
    for(const string &f : files)
    {
        LOG_INFO << f;

        // add path to searchpaths
        kinski::addSearchPath(kinski::getDirectoryPart(f));
        
        switch (get_filetype(f))
        {
            case FileType::FILE_MODEL:
                *m_model_path = f;
                break;
            
            case FileType::FILE_IMAGE:
                try
                {
                    textures().push_back(gl::createTextureFromFile(f, true, false));
                    
                    if(m_mesh)
                    {
                        m_mesh->material()->textures().clear();
                        m_mesh->material()->textures().push_back(textures().back());
                    }
                }
                catch (Exception &e) { LOG_WARNING << e.what();}
                if(scene().pick(gl::calculateRay(camera(), e.getX(), e.getY())))
                {
                    LOG_INFO << "texture drop on model";
                }
                break;
            default:
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////

void ModelViewer::tearDown()
{
    LOG_PRINT<<"ciao model viewer";
}

/////////////////////////////////////////////////////////////////

void ModelViewer::updateProperty(const Property::ConstPtr &theProperty)
{
    ViewerApp::updateProperty(theProperty);
    
    if(theProperty == m_model_path)
    {
        addSearchPath(getDirectoryPart(*m_model_path));
        gl::MeshPtr m = gl::AssimpConnector::loadModel(*m_model_path);
        
        if(m)
        {
            for(auto &t : m->material()->textures()){ textures().push_back(t); }
            
            scene().removeObject(m_mesh);
            m_mesh = m;
            scene().addObject(m_mesh);
            
            auto aabb = m->boundingBox();
            
            float scale_factor = 50.f / aabb.width();
            m->setScale(scale_factor);
        }
    }
}
