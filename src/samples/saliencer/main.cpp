#include "kinskiApp/GLFW_App.h"
#include "kinskiApp/TextureIO.h"

#include "kinskiGL/Material.h"

#include "kinskiCV/CVThread.h"
#include "SalienceNode.h"

#include "kinskiGL/SerializerGL.h"

using namespace std;
using namespace kinski;
using namespace glm;

class Saliencer : public GLFW_App
{
private:
    
    gl::Texture m_textures[4];
    
    gl::Material::Ptr m_material;
    
    Property_<bool>::Ptr m_activator;
    
    RangedProperty<uint32_t>::Ptr m_imageIndex;
    
    CVThread::Ptr m_cvThread;
    
    CVProcessNode::Ptr m_processNode;
    
public:
    
    void setup()
    {
        kinski::addSearchPath("/Users/Fabian/Desktop/");
        setBarColor(vec4(0, 0 ,0 , .5));
        setBarSize(ivec2(250, 500));

        // add 2 empty textures
        m_material = gl::Material::Ptr(new gl::Material);
        m_material->addTexture(m_textures[0]);
        m_material->addTexture(m_textures[1]);
        m_material->setDepthTest(false);
        m_material->setDepthWrite(false);
        
        m_activator = Property_<bool>::create("processing", true);
        m_imageIndex = RangedProperty<uint32_t>::create("Image Index", 0, 0, 1);
        
        registerProperty(m_activator);
        registerProperty(m_imageIndex);
        
        // add component-props to tweakbar
        addPropertyListToTweakBar(getPropertyList());
        
        // CV stuff
        m_cvThread = CVThread::Ptr(new CVThread());
        m_processNode = CVProcessNode::Ptr(new SalienceNode);
        
        m_cvThread->setProcessingNode(m_processNode);
        
        //m_cvThread->streamVideo("/Users/Fabian/dev/testGround/python/cvScope/scopeFootage/testMovie_00.MOV", true);
        m_cvThread->streamUSBCamera();
        
        if(m_processNode)
        {
            addPropertyListToTweakBar(m_processNode->getPropertyList());
            m_processNode->observeProperties();
            LOG_INFO<<"CVProcessNode: \n"<<m_processNode->getDescription();
        }
        
        LOG_INFO<<"CVThread source: \n"<<m_cvThread->getSourceInfo();
        
        try
        {
            m_material->setShader(gl::createShaderFromFile("applyMap.vert", "applyMap.frag"));
            Serializer::loadComponentState(shared_from_this(), "config.json", PropertyIO_GL());
            
        }catch(Exception &e)
        {
            LOG_WARNING<<e.what();
        }
    }
    
    void tearDown()
    {
        m_cvThread->stop();
        Serializer::saveComponentState(shared_from_this(), "config.json", PropertyIO_GL());
        
        LOG_PRINT<<"ciao saliencer";
    }
    
    void update(float timeDelta)
    {
        if(m_cvThread->hasImage())
        {
            vector<cv::Mat> images = m_cvThread->getImages();
            
            for(int i=0;i<images.size();i++)
                gl::TextureIO::updateTexture(m_textures[i], images[i]);
            
            m_imageIndex->setRange(0, images.size() - 1);
        }
        
        // trigger processing
        m_cvThread->setProcessing(*m_activator);
    }
    
    void draw()
    {
        // draw fullscreen image
        if(*m_activator)
            gl::drawQuad(m_material, windowSize());
        else
            gl::drawTexture(m_material->textures()[*m_imageIndex], windowSize());
        
        // draw process-results map(s)
        glm::vec2 offset(getWidth() - getWidth()/5.f - 10, getHeight() - 10);
        glm::vec2 step(0, - getHeight()/5.f - 10);
        
        for(int i=0;i<m_cvThread->getImages().size();i++)
        {
            gl::drawTexture(m_textures[i],
                            windowSize()/5.f,
                            offset);
            
            offset += step;
        }
    }
    
    void keyPress(const KeyEvent &e)
    {
        if(e.getChar() == KeyEvent::KEY_SPACE)
        {
            set_displayTweakBar(!displayTweakBar());
        }
    }
};

int main(int argc, char *argv[])
{
    App::Ptr theApp(new Saliencer);
    
    return theApp->run();
}
