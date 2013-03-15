//
//  Font.h
//  kinskiGL
//
//  Created by Fabian on 3/9/13.
//
//

#ifndef __kinskiGL__Font__
#define __kinskiGL__Font__

#include "KinskiGL.h"

namespace kinski { namespace gl {
    
    class KINSKI_API Font
    {
    public:
        Font();
        
        void load(const std::string &thePath, size_t theSize);
        Texture glyph_texture() const;
        Texture create_texture(const std::string &theText, const glm::vec4 &theColor = glm::vec4(1)) const;
        gl::MeshPtr create_mesh(const std::string &theText, const glm::vec4 &theColor = glm::vec4(1)) const;
        
    private:
        // forward declared Implementation object
        struct Obj;
        typedef std::shared_ptr<Obj> ObjPtr;
        ObjPtr m_obj;
        
    public:
        //! Emulates shared_ptr-like behavior
        typedef ObjPtr Font::*unspecified_bool_type;
        operator unspecified_bool_type() const { return ( m_obj.get() == 0 ) ? 0 : &Font::m_obj; }
        void reset() { m_obj.reset(); }
        //@}
    };
    
}}// namespace

#endif /* defined(__kinskiGL__Font__) */
