#ifndef __gl__MovieController__
#define __gl__MovieController__

#include "gl/KinskiGL.h"

/*
* This class controls playback of Movies and manages their assets.
*/

namespace kinski
{
    class MovieController;
    typedef std::shared_ptr<MovieController> MovieControllerPtr;
    
    class MovieController : public std::enable_shared_from_this<MovieController>
    {
    public:
        
        typedef std::function<void(MovieControllerPtr the_movie)> MovieCallback;
        
        static MovieControllerPtr create();
        static MovieControllerPtr create(const std::string &filePath, bool autoplay = false,
                                         bool loop = false);
        
        
        virtual ~MovieController();
        
        void load(const std::string &filePath, bool autoplay = false, bool loop = false);
        void play();
        void pause();
        bool isPlaying() const;
        void restart();
        void unload();
        void seek_to_time(float value);
        double duration() const;
        double current_time() const;
        float volume() const;
        void set_volume(float volume);
        void set_loop(bool b);
        void set_rate(float r);
        bool loop() const;
        const std::string& get_path() const;
        
        void set_on_load_callback(MovieCallback c);
        void set_movie_ended_callback(MovieCallback c);
        
        /*!
         * upload the current frame to a gl::Texture object
         * return: true if a new frame has been uploaded successfully,
         * false otherwise
         */
        bool copy_frame_to_texture(gl::Texture &tex, bool as_texture2D = false);
        
        /*!
         * copy the current frame to a std::vector<uint8_t>
         * return: true if a new frame has been copied successfully,
         * false otherwise
         */
        bool copy_frame(std::vector<uint8_t>& data, int *width = nullptr, int *height = nullptr);
        
        /*!
         * upload all frames to a gl::Texture object with target GL_TEXTURE_2D_ARRAY
         * return: true if all frames have been uploaded successfully,
         * false otherwise
         */
        bool copy_frames_offline(gl::Texture &tex, bool compress = false);
        
        inline bool operator==(const MovieController& other){ return m_impl == other.m_impl; }
        inline bool operator!=(const MovieController& other){ return !(*this == other); }
        
    private:
        
        MovieController();
        MovieController(const std::string &filePath, bool autoplay = false, bool loop = false);
        std::unique_ptr<struct MovieControllerImpl> m_impl;
    };
}

#endif
