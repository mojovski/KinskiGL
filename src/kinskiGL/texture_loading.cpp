//
//  texture_loading.cpp
//  kinskiGL
//
//  Created by Fabian on 14.02.13.
//
//

#include "KinskiGL.h"
#include "Texture.h"
#include "stb_image.inl"

namespace kinski { namespace gl {
    
    bool isPowerOf2(int v)
    {
        int tmp = 1;
        while (tmp < v) {tmp <<= 1;}
        return tmp == v;
    }
    
    MiniMat loadImage(const std::string &theFileName, int num_channels)
    {
        std::vector<uint8_t> dataVec = kinski::readBinaryFile(theFileName);
        
        int width, height, num_components;
        unsigned char *data = stbi_load_from_memory(&dataVec[0], dataVec.size(),
                                                    &width, &height, &num_components, num_channels);
        
        if(!data) throw ImageLoadException(theFileName);
        
        LOG_TRACE<<"loaded image: "<<theFileName<<" -- "<<width
        <<" x "<<height<<" ("<<num_components<<" ch)";
        // ... process data if not NULL ...
        // ... x = width, y = height, n = # 8-bit components per pixel ...
        // ... replace '0' with '1'..'4' to force that many components per pixel
        // ... but 'n' will always be the number that it would have been if you said 0
        
        MiniMat ret(data, height, width, num_components);
        return ret;
    }
    
    Texture createTextureFromFile(const std::string &theFileName, bool mipmap, bool compress,
                                  GLfloat anisotropic_filter_lvl)
    {
        Texture ret;
        MiniMat img = loadImage(theFileName);
        
        GLenum format = 0, internal_format = 0;
        
        switch(img.bytes_per_pixel)
        {
#ifdef KINSKI_GLES
            case 1:
                internal_format = format = GL_LUMINANCE;
                break;
            case 2:
                internal_format = format = GL_LUMINANCE_ALPHA;
                break;
            case 3:
                internal_format = format = GL_RGB;
                break;
            case 4:
                internal_format =  format = GL_RGBA;
            default:
                break;
#else
            case 1:
                format = GL_RED;
                internal_format = compress? GL_COMPRESSED_RED_RGTC1 : GL_RGBA;
                break;
            case 2:
                format = GL_RG;
                internal_format = compress? GL_COMPRESSED_RG_RGTC2 : GL_RGBA;
                break;
            case 3:
                format = GL_RGB;
                internal_format = compress? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGBA;
                break;
            case 4:
                format = GL_RGBA;
                internal_format = compress? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA;
            default:
                break;
#endif
        }
        
        Texture::Format fmt;
        fmt.setInternalFormat(internal_format);
        
        if(mipmap)
        {
            fmt.set_mipmapping();
            fmt.setMinFilter(GL_LINEAR_MIPMAP_NEAREST);
        }
        
        ret = Texture (img.data, format, img.cols, img.rows, fmt);
        ret.setFlipped();
        KINSKI_CHECK_GL_ERRORS();
        
        ret.set_anisotropic_filter(anisotropic_filter_lvl);
        stbi_image_free(img.data);
        
        return ret;
    }
    
}}
