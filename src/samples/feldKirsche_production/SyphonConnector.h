//
//  SyphonConnector.h
//  kinskiGL
//
//  Created by Fabian on 5/3/13.
//
//

#ifndef __kinskiGL__SyphonConnector__
#define __kinskiGL__SyphonConnector__

#include "kinskiGL/KinskiGL.h"
#include "kinskiCore/Component.h"

namespace kinski{ namespace gl{
    
    class SyphonConnector : public kinski::Component
    {
     public:
        SyphonConnector(){};
        SyphonConnector(const std::string &theName);
        
        void publish_framebuffer(const Fbo &theFbo);
        void publish_texture(const Texture &theTexture);
        void setName(const std::string &theName);
        std::string getName();
        
     private:
        
        struct Obj;
        typedef std::shared_ptr<Obj> ObjPtr;
        ObjPtr m_obj;
        
        //! Emulates shared_ptr-like behavior
        typedef ObjPtr SyphonConnector::*unspecified_bool_type;
        operator unspecified_bool_type() const { return ( m_obj.get() == 0 ) ? 0 : &SyphonConnector::m_obj; }
        void reset() { m_obj.reset(); }
    };
    
    class SyphonNotRunningException: public Exception
    {
    public:
        SyphonNotRunningException() :
        Exception("No Syphon server running"){}
    };
}}//namespace

#endif /* defined(__kinskiGL__SyphonConnector__) */