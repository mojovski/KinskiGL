#ifndef __gl__CameraController__
#define __gl__CameraController__

#include "gl/KinskiGL.h"

/*
* This class controls a camera capture
*/

namespace kinski
{
    class CameraController
    {
    private:
        
        struct Impl;
        std::shared_ptr<Impl> m_impl;
        
    public:
        CameraController(int device_id = 0);
        virtual ~CameraController();
        
        void start_capture();
        void stop_capture();
        bool is_capturing() const;
        
        bool copy_frame(std::vector<uint8_t>& data, int *width = nullptr, int *height = nullptr);
        
        /*!
         * upload the current frame to a gl::Texture object
         * return: true if a new frame could be successfully uploaded,
         * false otherwise
         */
        bool copy_frame_to_texture(gl::Texture &tex);
    };
    
    typedef std::shared_ptr<CameraController> CameraControllerPtr;
}

#endif
