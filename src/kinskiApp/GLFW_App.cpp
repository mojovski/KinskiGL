// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2013, Fabian Schmidt <crocdialer@googlemail.com>
//
// It is distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __

#include "kinskiGL/KinskiGL.h"
#include "GLFW_App.h"
#include "kinskiCore/file_functions.h"
#include "AntTweakBarConnector.h"

using namespace std;

namespace kinski
{
    GLFW_Window::GLFW_Window(int width, int height, const std::string &theName)
    {
        m_handle = glfwCreateWindow(width, height, theName.c_str(), NULL, NULL);
        if(!m_handle) throw CreateWindowException();
        glfwMakeContextCurrent(m_handle);
    }
    
    GLFW_Window::~GLFW_Window()
    {
        glfwDestroyWindow(m_handle);
    }
    
    GLFW_App::GLFW_App(const int width, const int height):
    App(width, height),
    m_lastWheelPos(0),
    m_displayTweakBar(true)
    {
        
    }
    
    GLFW_App::~GLFW_App()
    {
        TwTerminate();
        
        // Close window and terminate GLFW
        glfwTerminate();
    }
    
    void GLFW_App::init()
    {
        // Initialize GLFW
        if( !glfwInit() )
        {
            throw Exception("GLFW failed to initialize");
        }
        
        // request an OpenGl 3.2 Context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3 );
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2 );
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        
        // create the window
        m_windows.push_back(GLFW_Window::Ptr(new GLFW_Window(getWidth(), getHeight(), getName())));
        glfwSetWindowUserPointer(m_windows.back()->handle(), this);
        
        gl::setWindowDimension(windowSize());
        glfwSetInputMode(m_windows.back()->handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        
        // version
        LOG_INFO<<"OpenGL: " << glGetString(GL_VERSION);
        LOG_INFO<<"GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
        
        glfwSwapInterval(1);
        glClearColor(0, 0, 0, 1);
        
        // file search paths
        kinski::addSearchPath("./res", true);
        kinski::addSearchPath("../Resources", true);
        
        // AntTweakbar
        TwInit(TW_OPENGL_CORE, NULL);
        TwWindowSize(getWidth(), getHeight());
        
        glfwSetMouseButtonCallback(m_windows.back()->handle(), &GLFW_App::s_mouseButton);
        glfwSetCursorPosCallback(m_windows.back()->handle(), &GLFW_App::s_mouseMove);
        glfwSetScrollCallback(m_windows.back()->handle(), &GLFW_App::s_mouseWheel);
        glfwSetKeyCallback(m_windows.back()->handle(), &GLFW_App::s_keyFunc);
        glfwSetCharCallback(m_windows.back()->handle(), &GLFW_App::s_charFunc);
        glfwSetWindowSizeCallback(m_windows.back()->handle(), &GLFW_App::s_resize);
        
        // call user defined setup callback
        setup();
    }
    
    void GLFW_App::swapBuffers()
    {
        for(auto window : m_windows)
        {
            glfwSwapBuffers(window->handle());
        }
    }
    
    void GLFW_App::setWindowSize(const glm::vec2 size)
    {
        App::setWindowSize(size);
        if(!m_windows.empty())
            glfwSetWindowSize(m_windows.back()->handle(), (int)size[0], (int)size[1]);
    }
    
    void GLFW_App::draw_internal()
    {
        draw();
        
        // draw tweakbar
        if(m_displayTweakBar)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            TwDraw();
        }
        
        // trigger input callbacks
        glfwPollEvents();
    }
    
    bool GLFW_App::checkRunning()
    {
        return !glfwGetKey(m_windows.back()->handle(), GLFW_KEY_ESCAPE ) &&
            !glfwWindowShouldClose( m_windows.back()->handle() );
    }
    
    double GLFW_App::getApplicationTime()
    {
        return glfwGetTime();
    }
    
    void GLFW_App::setFullSceen(bool b)
    {
        App::setFullSceen(b);
        throw Exception("not yet supported");
    }
    
/****************************  Application Events (internal) **************************/
    
    void GLFW_App::s_resize(GLFWwindow* window, int w, int h)
    {
        GLFW_App* app = static_cast<GLFW_App*>(glfwGetWindowUserPointer(window));
        app->setWindowSize(glm::vec2(w, h));
        glViewport(0, 0, w, h);
        gl::setWindowDimension(app->windowSize());
        TwWindowSize(w, h);
        
        // user hook
        if(app->running()) app->resize(w, h);
    }
    
    void GLFW_App::s_mouseMove(GLFWwindow* window, double x, double y)
    {
        GLFW_App* app = static_cast<GLFW_App*>(glfwGetWindowUserPointer(window));
        if(app->displayTweakBar())
            TwEventMousePosGLFW((int)x, (int)y);
        uint32_t buttonModifiers, keyModifiers, bothMods;
        s_getModifiers(window, 0, buttonModifiers, keyModifiers);
        bothMods = buttonModifiers | keyModifiers;
        MouseEvent e(buttonModifiers, (int)x, (int)y, bothMods, glm::ivec2(0));
        
        if(buttonModifiers)
            app->mouseDrag(e);
        else
            app->mouseMove(e);
    }
    
    void GLFW_App::s_mouseButton(GLFWwindow* window,int button, int action, int modifier_mask)
    {
        GLFW_App* app = static_cast<GLFW_App*>(glfwGetWindowUserPointer(window));
        if(app->displayTweakBar())
            TwEventMouseButtonGLFW(button, action);
        
        uint32_t initiator, keyModifiers, bothMods;
        s_getModifiers(window, modifier_mask, initiator, keyModifiers);
        bothMods = initiator | keyModifiers;
        
        double posX, posY;
        glfwGetCursorPos(window, &posX, &posY);
        
        MouseEvent e(initiator, (int)posX, (int)posY, bothMods, glm::ivec2(0));
        
        switch(action)
        {
            case GLFW_PRESS:
                app->mousePress(e);
                
            case GLFW_RELEASE:
                app->mouseRelease(e);
        }
    }
    
    void GLFW_App::s_mouseWheel(GLFWwindow* window,double offset_x, double offset_y)
    {
        GLFW_App* app = static_cast<GLFW_App*>(glfwGetWindowUserPointer(window));
        glm::ivec2 offset = glm::ivec2(offset_x, offset_y);
        app->m_lastWheelPos -= offset;
        if(app->displayTweakBar())
            TwEventMouseWheelGLFW(app->m_lastWheelPos.y);
        
        double posX, posY;
        glfwGetCursorPos(window, &posX, &posY);
        uint32_t buttonMod, keyModifiers = 0;
        s_getModifiers(window, 0, buttonMod, keyModifiers);
        MouseEvent e(0, (int)posX, (int)posY, keyModifiers, offset);
        if(app->running()) app->mouseWheel(e);
    }
    
    void GLFW_App::s_keyFunc(GLFWwindow* window, int key, int scancode, int action, int modifier_mask)
    {
        GLFW_App* app = static_cast<GLFW_App*>(glfwGetWindowUserPointer(window));
        if(app->displayTweakBar())
            TwEventKeyGLFW(key, action);
        
        uint32_t buttonMod, keyMod;
        s_getModifiers(window, modifier_mask, buttonMod, keyMod);
        
        KeyEvent e(key, key, keyMod);
        
        switch(action)
        {
            case GLFW_PRESS:
            case GLFW_REPEAT:
                app->keyPress(e);
                
            case GLFW_RELEASE:
                app->keyRelease(e);
        }
    }
    
    void GLFW_App::s_charFunc(GLFWwindow* window, unsigned int key)
    {
        GLFW_App* app = static_cast<GLFW_App*>(glfwGetWindowUserPointer(window));
        if(app->displayTweakBar())
            TwEventCharGLFW(key, GLFW_PRESS);
        
        if(key == GLFW_KEY_SPACE){return;}
        
        uint32_t buttonMod, keyMod;
        s_getModifiers(window, 0, buttonMod, keyMod);
        
        KeyEvent e(0, key, keyMod);
        app->keyPress(e);
    }
    
    void GLFW_App::s_getModifiers(GLFWwindow* window, int modifier_mask,
                                  uint32_t &buttonModifiers, uint32_t &keyModifiers)
    {
        buttonModifiers = 0;
        if( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) )
            buttonModifiers |= MouseEvent::LEFT_DOWN;
        if( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) )
            buttonModifiers |= MouseEvent::MIDDLE_DOWN;
        if( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) )
            buttonModifiers |= MouseEvent::RIGHT_DOWN;
        
        keyModifiers = 0;
        if( glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
            keyModifiers |= KeyEvent::CTRL_DOWN;
        if( glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
            keyModifiers |= KeyEvent::SHIFT_DOWN;
        if( glfwGetKey(window, GLFW_KEY_LEFT_ALT) || glfwGetKey(window, GLFW_KEY_RIGHT_ALT))
            keyModifiers |= KeyEvent::ALT_DOWN;
        if( glfwGetKey(window, GLFW_KEY_LEFT_SUPER) || glfwGetKey(window, GLFW_KEY_RIGHT_SUPER))
            keyModifiers |= KeyEvent::META_DOWN;
    }
    
/****************************  TweakBar + Properties **************************/
    
    void GLFW_App::create_tweakbar_from_component(const Component::Ptr &the_component)
    {
        if(!the_component) return;
        m_tweakBars.push_back(TwNewBar(the_component->getName().c_str()));
        setBarColor(glm::vec4(0, 0, 0, .5), m_tweakBars.back());
        setBarSize(glm::ivec2(250, 500));
        glm::ivec2 offset(10);
        setBarPosition(glm::ivec2(offset.x + 260 * (m_tweakBars.size() - 1), offset.y), m_tweakBars.back());
        addPropertyListToTweakBar(the_component->getPropertyList(), "", m_tweakBars.back());
    }
    
    void GLFW_App::addPropertyToTweakBar(const Property::Ptr propPtr,
                                         const string &group,
                                         TwBar *theBar)
    {
        if(!theBar)
        {   if(m_tweakBars.empty()) return;
            theBar = m_tweakBars.front();
        }
        m_tweakProperties[theBar].push_back(propPtr);
        
        try {
            AntTweakBarConnector::connect(theBar, propPtr, group);
        } catch (AntTweakBarConnector::PropertyUnsupportedException &e) {
            LOG_ERROR<<e.what();
        }
    }
    
    void GLFW_App::addPropertyListToTweakBar(const list<Property::Ptr> &theProps,
                                             const string &group,
                                             TwBar *theBar)
    {
        if(!theBar)
        {   if(m_tweakBars.empty()) return;
            theBar = m_tweakBars.front();
        }
        for (const auto property : theProps)
        {   
            addPropertyToTweakBar(property, group, theBar);
        }
        TwAddSeparator(theBar, "sep1", NULL);
    }
    
    void GLFW_App::setBarPosition(const glm::ivec2 &thePos, TwBar *theBar)
    {
        if(!theBar)
        {   if(m_tweakBars.empty()) return;
            theBar = m_tweakBars.front();
        }
        std::stringstream ss;
        ss << TwGetBarName(theBar) << " position='" <<thePos.x
        <<" " << thePos.y <<"'";
        TwDefine(ss.str().c_str());
    }
    
    void GLFW_App::setBarSize(const glm::ivec2 &theSize, TwBar *theBar)
    {
        if(!theBar)
        {   if(m_tweakBars.empty()) return;
            theBar = m_tweakBars.front();
        }
        std::stringstream ss;
        ss << TwGetBarName(theBar) << " size='" <<theSize.x
        <<" " << theSize.y <<"'";
        TwDefine(ss.str().c_str());
    }
    
    void GLFW_App::setBarColor(const glm::vec4 &theColor, TwBar *theBar)
    {
        if(!theBar)
        {   if(m_tweakBars.empty()) return;
            theBar = m_tweakBars.front();
        }
        std::stringstream ss;
        glm::ivec4 color(theColor * 255);
        ss << TwGetBarName(theBar) << " color='" <<color.r
        <<" " << color.g << " " << color.b <<"' alpha="<<color.a;
        TwDefine(ss.str().c_str());
    }
    
    void GLFW_App::setBarTitle(const std::string &theTitle, TwBar *theBar)
    {
        if(!theBar)
        {   if(m_tweakBars.empty()) return;
            theBar = m_tweakBars.front();
        }
        std::stringstream ss;
        ss << TwGetBarName(theBar) << " label='" << theTitle <<"'";
        TwDefine(ss.str().c_str());
    }
}