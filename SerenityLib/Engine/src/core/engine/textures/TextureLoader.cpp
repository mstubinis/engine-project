#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/textures/DDS.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/Engine.h>
#include <core/engine/renderer/FramebufferObject.h>

#include <boost/filesystem.hpp>

#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>

using namespace Engine;
using namespace Engine::epriv;
using namespace Engine::epriv::textures;
using namespace std;

void TextureLoader::InitCommon(Texture& texture, const GLuint& openglTextureType, const bool& toBeMipmapped) {
    texture.m_Mipmapped       = false;
    texture.m_IsToBeMipmapped = toBeMipmapped;
    texture.m_MinFilter       = GL_LINEAR;
    texture.m_Type            = openglTextureType;
}

void TextureLoader::InitFramebuffer(Texture& texture, const uint& w, const uint& h, const ImagePixelType::Type& pxlType, const ImagePixelFormat::Format& pxlFormat, const ImageInternalFormat::Format& _internal, const float& divisor) {
    texture.m_TextureType = TextureType::RenderTarget;
    const uint _width(static_cast<uint>(static_cast<float>(w) * divisor));
    const uint _height(static_cast<uint>(static_cast<float>(h) * divisor));
    auto image = std::make_unique<ImageLoadedStructure>(_width, _height, pxlType, pxlFormat, _internal);

    TextureLoader::InitCommon(texture, GL_TEXTURE_2D, false);

    texture.m_ImagesDatas.push_back(std::move(image));
    texture.setName("RenderTarget");
}
void TextureLoader::InitFromMemory(Texture& texture, const sf::Image& sfImage, const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) {
    texture.m_TextureType = TextureType::Texture2D;
    auto image = std::make_unique<ImageLoadedStructure>(sfImage, name);
    image->pixelType = ImagePixelType::UNSIGNED_BYTE;
    image->internalFormat = _internal;

    TextureLoader::InitCommon(texture, openglTextureType, genMipMaps);

    TextureLoader::ChoosePixelFormat(image->pixelFormat, image->internalFormat);
    texture.m_ImagesDatas.push_back(std::move(image));
    texture.setName(name);
}
void TextureLoader::InitFromFile(Texture& texture, const string& filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) {
    texture.m_TextureType = TextureType::Texture2D;
    auto image = std::make_unique<ImageLoadedStructure>();
    image->filename = filename;
    const string& extension = boost::filesystem::extension(filename);
    TextureLoader::InitCommon(texture, openglTextureType, genMipMaps);
    if (extension == ".dds") {
        TextureLoader::LoadDDSFile(texture, filename, *image);
    }else{
        image->pixelType = ImagePixelType::UNSIGNED_BYTE;
        image->internalFormat = _internal;
    }
    TextureLoader::ChoosePixelFormat(image->pixelFormat, image->internalFormat);
    texture.m_ImagesDatas.insert(texture.m_ImagesDatas.begin(), std::move(image)); //yes, this NEEDS to be pushed into the front, not the back

    texture.setName(filename);
}
void TextureLoader::InitFromFilesCubemap(Texture& texture, const string files[], const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal) {
    texture.m_TextureType = TextureType::CubeMap;
    for (uint j = 0; j < 6; ++j) {
        auto image = std::make_unique<ImageLoadedStructure>();
        image->filename = files[j];
        texture.m_ImagesDatas.push_back(std::move(image));
    }
    TextureLoader::InitCommon(texture, GL_TEXTURE_CUBE_MAP, genMipMaps);
    for (auto& sideImage : texture.m_ImagesDatas) {
        texture.m_Type = GL_TEXTURE_CUBE_MAP;
        sideImage->pixelType = ImagePixelType::UNSIGNED_BYTE;
        sideImage->internalFormat = _internal;
        TextureLoader::ChoosePixelFormat(sideImage->pixelFormat, sideImage->internalFormat);
    }
    texture.setName(name);
}

void TextureLoader::ImportIntoOpengl(Texture& texture, const Engine::epriv::ImageMipmap& mipmap, const GLuint& openGLType) {
    auto& imageData = *texture.m_ImagesDatas[0];
    if (TextureLoader::IsCompressedType(imageData.internalFormat) && mipmap.compressedSize != 0)
        glCompressedTexImage2D(openGLType, mipmap.level, imageData.internalFormat, mipmap.width, mipmap.height, 0, mipmap.compressedSize, &mipmap.pixels[0]);
    else
        glTexImage2D(openGLType, mipmap.level, imageData.internalFormat, mipmap.width, mipmap.height, 0, imageData.pixelFormat, imageData.pixelType, &mipmap.pixels[0]);
}

void TextureLoader::LoadDDSFile(Texture& texture, const string& filename, ImageLoadedStructure& image) {
    ifstream stream(filename.c_str(), ios::binary);
    if (!stream) 
        return;
    uchar header_buffer[128];
    stream.read((char*)header_buffer, sizeof(header_buffer));

    DDS::DDS_Header head(header_buffer);
    if (head.magic != 0x20534444) { stream.close(); return; } //check if this is "DDS "
    //DX10 header here
    DDS::DDS_Header_DX10 headDX10;
    if ((head.header_flags & DDS::DDPF_FOURCC) && head.format.fourCC == FourCC_DX10) {
        uchar header_buffer_DX10[20];
        stream.read((char*)header_buffer_DX10, sizeof(header_buffer_DX10));
        headDX10.fill(header_buffer_DX10);
    }
    uint32_t factor, blockSize, offset = 0;
    switch (head.format.fourCC) {
        case FourCC_DXT1: {
            factor = 2;
            blockSize = 8;
            image.internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
            break;
        }case FourCC_DXT2: {
            factor = 4;
            blockSize = 16;
            break;
        }case FourCC_DXT3: {
            factor = 4;
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
            break;
        }case FourCC_DXT4: {
            factor = 4;
            blockSize = 16;
            break;
        }case FourCC_DXT5: {
            factor = 4;
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
            break;
        }case FourCC_DX10: {
            break;
        }case FourCC_ATI1: { //useful for 1 channel textures (greyscales, glow / specular / ao / smoothness / metalness etc)
            factor = 2;
            blockSize = 8;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RED_RGTC1;
            break;
        }case FourCC_ATI2: {//aka ATI2n aka 3Dc aka LATC2 aka BC5 - used for normal maps (store x,y recalc z) z = sqrt( 1-x*x-y*y )
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RG_RGTC2;
            break;
        }case FourCC_RXGB: { //By its design, it is just a DXT5 image with reading it in the shader differently
            //As I recall, the most you would have to do in the shader is something like:
            //vec3 normal;
            //normal.xy = texture2D(RXGBnormalmap, texcoord).ag;
            //normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);
            factor = 4;
            blockSize = 16;
            image.internalFormat = ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        }case FourCC_$: {
            break;
        }case FourCC_o: {
            break;
        }case FourCC_p: {
            break;
        }case FourCC_q: {
            break;
        }case FourCC_r: {
            break;
        }case FourCC_s: {
            break;
        }case FourCC_t: {
            break;
        }case FourCC_BC4U: {
            break;
        }case FourCC_BC4S: {
            break;
        }case FourCC_BC5U: {
            break;
        }case FourCC_BC5S: {
            break;
        }case FourCC_RGBG: {
            break;
        }case FourCC_GRGB: {
            break;
        }case FourCC_YUY2: {
            break;
        }default: {
            return;
        }
    }

    uint numberOfMainImages = 1;
    if (head.caps & DDS::DDS_CAPS_COMPLEX) {
        if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP) {//cubemap
            //note: in skybox dds files, especially in gimp, layer order is as follows:
            //right,left,top,bottom,front,back
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEX) { ++numberOfMainImages; }//right
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEX) { ++numberOfMainImages; }//left
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEY) { ++numberOfMainImages; }//top
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEY) { ++numberOfMainImages; }//bottom
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_POSITIVEZ) { ++numberOfMainImages; }//front
            if (head.caps2 & DDS::DDS_CAPS2_CUBEMAP_NEGATIVEZ) { ++numberOfMainImages; }//back
            texture.m_Type = GL_TEXTURE_CUBE_MAP;
            texture.m_TextureType = TextureType::CubeMap;
            texture.m_IsToBeMipmapped = false;
            --numberOfMainImages;
        }
    }

    const uint& bufferSize = (head.mipMapCount >= 2 ? head.pitchOrlinearSize * factor : head.pitchOrlinearSize);
    uchar* pxls = (uchar*)malloc(bufferSize * numberOfMainImages);
    stream.read((char*)pxls, bufferSize * numberOfMainImages);
    stream.close();

    image.pixelFormat = ImagePixelFormat::RGBA;
    image.pixelType = ImagePixelType::UNSIGNED_BYTE;

    uint _width = head.w;
    uint _height = head.h;
    for (uint mainImageLevel = 0; mainImageLevel < numberOfMainImages; ++mainImageLevel) {

        ImageLoadedStructure* imgPtr = nullptr;
        if (mainImageLevel == 0) {
            imgPtr = &image;
        }else if (texture.m_ImagesDatas.size() < mainImageLevel) {
            imgPtr = new ImageLoadedStructure();
            imgPtr->pixelFormat = image.pixelFormat;
            imgPtr->pixelType = image.pixelType;
            imgPtr->internalFormat = image.internalFormat;
            texture.m_ImagesDatas.push_back(std::unique_ptr<ImageLoadedStructure>(imgPtr));
        }else{
            imgPtr = texture.m_ImagesDatas[mainImageLevel].get();
        }

        _width = head.w;
        _height = head.h;
        for (uint level = 0; level < head.mipMapCount && (_width || _height); ++level) {
            if (level > 0 && (_width < 64 || _height < 64)) break;
            ImageMipmap* mipmap = nullptr;
            ImageMipmap  mipMapCon = ImageMipmap();
            if (level >= 1) { mipmap = &mipMapCon; }
            else { mipmap = &imgPtr->mipmaps[0]; }
            mipmap->level = level;
            mipmap->width = _width;
            mipmap->height = _height;
            mipmap->compressedSize = ((_width + 3) / 4) * ((_height + 3) / 4) * blockSize;

            mipmap->pixels.resize(mipmap->compressedSize);
            for (uint t = 0; t < mipmap->pixels.size(); ++t) {
                const uint& _index = offset + t;
                mipmap->pixels[t] = pxls[_index];
            }
            _width = Math::Max(uint(_width / 2), 1);
            _height = Math::Max(uint(_height / 2), 1);
            offset += mipmap->compressedSize;
            if (level >= 1) { imgPtr->mipmaps.push_back(*mipmap); }
        }
    }
    free(pxls);
}
void TextureLoader::LoadTexture2DIntoOpenGL(Texture& texture) {
    Renderer::bindTextureForModification(texture.m_Type, texture.m_TextureAddress[0]);
    for (auto& mipmap : texture.m_ImagesDatas[0]->mipmaps) {
        TextureLoader::ImportIntoOpengl(texture, mipmap, texture.m_Type);
        //TextureLoader::WithdrawPixelsFromOpenGLMemory(texture,0,mipmap.level);
    }
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::Repeat);
}
void TextureLoader::LoadTextureFramebufferIntoOpenGL(Texture& texture) {
    Renderer::bindTextureForModification(texture.m_Type, texture.m_TextureAddress[0]);
    const auto& image = *texture.m_ImagesDatas[0];
    const uint& _w = image.mipmaps[0].width;
    const uint& _h = image.mipmaps[0].height;
    glTexImage2D(texture.m_Type, 0, image.internalFormat, _w, _h, 0, image.pixelFormat, image.pixelType, NULL);
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::ClampToEdge);
}
void TextureLoader::LoadTextureCubemapIntoOpenGL(Texture& texture) {
    Renderer::bindTextureForModification(texture.m_Type, texture.m_TextureAddress[0]);
    uint imageIndex = 0;
    for (auto& image : texture.m_ImagesDatas) {
        for (auto& mipmap : image->mipmaps) {
            TextureLoader::ImportIntoOpengl(texture, mipmap, GL_TEXTURE_CUBE_MAP_POSITIVE_X + imageIndex);
            //TextureLoader::WithdrawPixelsFromOpenGLMemory(texture,imageIndex,mipmap.level);
        }
        ++imageIndex;
    }
    texture.setFilter(TextureFilter::Linear);
    texture.setWrapping(TextureWrap::ClampToEdge);
}
void TextureLoader::WithdrawPixelsFromOpenGLMemory(Texture& texture, const uint& imageIndex, const uint& mipmapLevel) {
    auto& image = *texture.m_ImagesDatas[imageIndex];
    auto& pxls = image.mipmaps[mipmapLevel].pixels;
    if (pxls.size() != 0)
        return;
    const uint& _w = image.mipmaps[mipmapLevel].width;
    const uint& _h = image.mipmaps[mipmapLevel].height;
    pxls.resize(_w * _h * 4);
    Renderer::bindTextureForModification(texture.m_Type, texture.m_TextureAddress[0]);
    glGetTexImage(texture.m_Type, 0, image.pixelFormat, image.pixelType, &pxls[0]);
}
void TextureLoader::ChoosePixelFormat(ImagePixelFormat::Format& out, const ImageInternalFormat::Format& in) {
    switch (in) {
        case ImageInternalFormat::COMPRESSED_RED: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::COMPRESSED_RED_RGTC1: { out = ImagePixelFormat::RED; break; }//recheck this
        case ImageInternalFormat::COMPRESSED_RG: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::COMPRESSED_RGB: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::COMPRESSED_RGBA: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::COMPRESSED_RG_RGTC2: { out = ImagePixelFormat::RG; break; }//recheck this
        case ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1: { out = ImagePixelFormat::RED; break; }//recheck this
        case ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2: { out = ImagePixelFormat::RG; break; }//recheck this
        case ImageInternalFormat::COMPRESSED_SRGB: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::Depth16: { out = ImagePixelFormat::DEPTH_COMPONENT; break; }
        case ImageInternalFormat::Depth24: { out = ImagePixelFormat::DEPTH_COMPONENT; break; }
        case ImageInternalFormat::Depth32: { out = ImagePixelFormat::DEPTH_COMPONENT; break; }
        case ImageInternalFormat::Depth32F: { out = ImagePixelFormat::DEPTH_COMPONENT; break; }
        case ImageInternalFormat::Depth24Stencil8: { out = ImagePixelFormat::DEPTH_STENCIL; break; }
        case ImageInternalFormat::Depth32FStencil8: { out = ImagePixelFormat::DEPTH_STENCIL; break; }
        case ImageInternalFormat::DEPTH_COMPONENT: { out = ImagePixelFormat::DEPTH_COMPONENT; break; }
        case ImageInternalFormat::DEPTH_STENCIL: { out = ImagePixelFormat::DEPTH_STENCIL; break; }
        case ImageInternalFormat::R11F_G11F_B10F: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::R16: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::R16F: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::R16I: { out = ImagePixelFormat::RED_INTEGER; break; }
        case ImageInternalFormat::R16UI: { out = ImagePixelFormat::RED_INTEGER; break; }
        case ImageInternalFormat::R16_SNORM: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::R32F: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::R32I: { out = ImagePixelFormat::RED_INTEGER; break; }
        case ImageInternalFormat::R32UI: { out = ImagePixelFormat::RED_INTEGER; break; }
        case ImageInternalFormat::R3_G3_B2: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::R8: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::R8I: { out = ImagePixelFormat::RED_INTEGER; break; }
        case ImageInternalFormat::R8UI: { out = ImagePixelFormat::RED_INTEGER; break; }
        case ImageInternalFormat::R8_SNORM: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::RED: { out = ImagePixelFormat::RED; break; }
        case ImageInternalFormat::RG: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RG16: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RG16F: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RG16I: { out = ImagePixelFormat::RG_INTEGER; break; }
        case ImageInternalFormat::RG16UI: { out = ImagePixelFormat::RG_INTEGER; break; }
        case ImageInternalFormat::RG16_SNORM: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RG32F: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RG32I: { out = ImagePixelFormat::RG_INTEGER; break; }
        case ImageInternalFormat::RG32UI: { out = ImagePixelFormat::RG_INTEGER; break; }
        case ImageInternalFormat::RG8: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RG8I: { out = ImagePixelFormat::RG_INTEGER; break; }
        case ImageInternalFormat::RG8UI: { out = ImagePixelFormat::RG_INTEGER; break; }
        case ImageInternalFormat::RG8_SNORM: { out = ImagePixelFormat::RG; break; }
        case ImageInternalFormat::RGB: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB10: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB10_A2: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGB12: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB16F: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB16I: { out = ImagePixelFormat::RGB_INTEGER; break; }
        case ImageInternalFormat::RGB16UI: { out = ImagePixelFormat::RGB_INTEGER; break; }
        case ImageInternalFormat::RGB16_SNORM: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB32F: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB32I: { out = ImagePixelFormat::RGB_INTEGER; break; }
        case ImageInternalFormat::RGB32UI: { out = ImagePixelFormat::RGB_INTEGER; break; }
        case ImageInternalFormat::RGB4: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB5: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB5_A1: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGB8: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB8I: { out = ImagePixelFormat::RGB_INTEGER; break; }
        case ImageInternalFormat::RGB8UI: { out = ImagePixelFormat::RGB_INTEGER; break; }
        case ImageInternalFormat::RGB8_SNORM: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::RGB9_E5: { out = ImagePixelFormat::RGB; break; }//recheck this
        case ImageInternalFormat::RGBA: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA12: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA16: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA16F: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA16I: { out = ImagePixelFormat::RGBA_INTEGER; break; }
        case ImageInternalFormat::RGBA16UI: { out = ImagePixelFormat::RGBA_INTEGER; break; }
        case ImageInternalFormat::RGBA2: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA32F: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA32I: { out = ImagePixelFormat::RGBA_INTEGER; break; }
        case ImageInternalFormat::RGBA32UI: { out = ImagePixelFormat::RGBA_INTEGER; break; }
        case ImageInternalFormat::RGBA4: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA8: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::RGBA8I: { out = ImagePixelFormat::RGBA_INTEGER; break; }
        case ImageInternalFormat::RGBA8UI: { out = ImagePixelFormat::RGBA_INTEGER; break; }
        case ImageInternalFormat::RGBA8_SNORM: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::SRGB8: { out = ImagePixelFormat::RGB; break; }
        case ImageInternalFormat::SRGB8_ALPHA8: { out = ImagePixelFormat::RGBA; break; }
        case ImageInternalFormat::StencilIndex8: { out = ImagePixelFormat::STENCIL_INDEX; break; }
        case ImageInternalFormat::STENCIL_INDEX: { out = ImagePixelFormat::STENCIL_INDEX; break; }
        default: { out = ImagePixelFormat::RGBA; break; }
    }
}
const bool TextureLoader::IsCompressedType(const ImageInternalFormat::Format& format) {
    switch (format) {
        case ImageInternalFormat::COMPRESSED_RED: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RED_RGTC1: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RG: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RGB: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_RG_RGTC2: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: { return true; break; }
        case ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT: { return true; break; }
        default: { return false; break; }
    }
    return false;
}
void TextureLoader::GenerateMipmapsOpenGL(Texture& texture) {
    if (texture.m_Mipmapped)
        return;
    //const auto& image = *texture.m_ImagesDatas[0];
    //const uint& _w = image.mipmaps[0].width;
    //const uint& _h = image.mipmaps[0].height;
    Renderer::bindTextureForModification(texture.m_Type, texture.m_TextureAddress[0]);
    glTexParameteri(texture.m_Type, GL_TEXTURE_BASE_LEVEL, 0);
    if (texture.m_MinFilter == GL_LINEAR) {
        texture.m_MinFilter = GL_LINEAR_MIPMAP_LINEAR;
    }else if (texture.m_MinFilter == GL_NEAREST) {
        texture.m_MinFilter = GL_NEAREST_MIPMAP_NEAREST;
    }
    glTexParameteri(texture.m_Type, GL_TEXTURE_MIN_FILTER, texture.m_MinFilter);
    glGenerateMipmap(texture.m_Type);
    texture.m_Mipmapped = true;
    //uint mipmaplevels = uint(glm::log2(glm::max(_w,_h)) + 1.0f);
}
void TextureLoader::EnumWrapToGL(unsigned int& gl, const TextureWrap::Wrap& wrap) {
    if      (wrap == TextureWrap::Repeat)         gl = GL_REPEAT;
    else if (wrap == TextureWrap::RepeatMirrored) gl = GL_MIRRORED_REPEAT;
    else if (wrap == TextureWrap::ClampToBorder)  gl = GL_CLAMP_TO_BORDER;
    else if (wrap == TextureWrap::ClampToEdge)    gl = GL_CLAMP_TO_EDGE;
}
void TextureLoader::EnumFilterToGL(unsigned int& gl, const TextureFilter::Filter& filter, const bool& min) {
    if (min) {
        if      (filter == TextureFilter::Linear)                  gl = GL_LINEAR;
        else if (filter == TextureFilter::Nearest)                 gl = GL_NEAREST;
        else if (filter == TextureFilter::Linear_Mipmap_Linear)    gl = GL_LINEAR_MIPMAP_LINEAR;
        else if (filter == TextureFilter::Linear_Mipmap_Nearest)   gl = GL_LINEAR_MIPMAP_NEAREST;
        else if (filter == TextureFilter::Nearest_Mipmap_Linear)   gl = GL_NEAREST_MIPMAP_LINEAR;
        else if (filter == TextureFilter::Nearest_Mipmap_Nearest)  gl = GL_NEAREST_MIPMAP_NEAREST;
    }else{
        if      (filter == TextureFilter::Linear)                  gl = GL_LINEAR;
        else if (filter == TextureFilter::Nearest)                 gl = GL_NEAREST;
        else if (filter == TextureFilter::Linear_Mipmap_Linear)    gl = GL_LINEAR;
        else if (filter == TextureFilter::Linear_Mipmap_Nearest)   gl = GL_LINEAR;
        else if (filter == TextureFilter::Nearest_Mipmap_Linear)   gl = GL_NEAREST;
        else if (filter == TextureFilter::Nearest_Mipmap_Nearest)  gl = GL_NEAREST;
    }
}

void TextureLoader::GeneratePBRData(Texture& texture, const unsigned int& convoludeTextureSize, const unsigned int& preEnvFilterSize) {
    if (texture.m_TextureAddress.size() == 1) {
        texture.m_TextureAddress.push_back(0);
        Renderer::genAndBindTexture(texture.m_Type, texture.m_TextureAddress[1]);
        for (uint i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, convoludeTextureSize, convoludeTextureSize, 0, GL_RGB, GL_FLOAT, NULL);
        }
        texture.setWrapping(TextureWrap::ClampToEdge);
        texture.setFilter(TextureFilter::Linear);
    }
    if (texture.m_TextureAddress.size() == 2) {
        texture.m_TextureAddress.push_back(0);
        Renderer::genAndBindTexture(texture.m_Type, texture.m_TextureAddress[2]);
        for (uint i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, preEnvFilterSize, preEnvFilterSize, 0, GL_RGB, GL_FLOAT, NULL);
        }
        texture.setWrapping(TextureWrap::ClampToEdge);
        texture.setMinFilter(TextureFilter::Linear_Mipmap_Linear);
        texture.setMaxFilter(TextureFilter::Linear);
        glGenerateMipmap(texture.m_Type);
    }
    Core::m_Engine->m_RenderManager._genPBREnvMapData(texture, convoludeTextureSize, preEnvFilterSize);
}

void InternalTexturePublicInterface::LoadCPU(Texture& texture) {
    if (texture.m_TextureAddress.size() == 0)
        texture.m_TextureAddress.push_back(0);
    for (auto& image : texture.m_ImagesDatas) {
        if (!image->filename.empty()) {
            bool _do = false;
            if (image->mipmaps.size() == 0) { 
                _do = true; 
            }
            if (!_do) {
                for (auto& mip : image->mipmaps) {
                    if (mip.pixels.size() == 0) {
                        _do = true;
                        break;
                    }
                }
            }
            if (_do) {
                const string& extension = boost::filesystem::extension(image->filename);
                if (extension == ".dds") {
                    TextureLoader::LoadDDSFile(texture, image->filename, *image);
                }else{
                    sf::Image _sfImage;
                    _sfImage.loadFromFile(image->filename);
                    image->load(_sfImage, image->filename);
                }
            }
        }
    }
}
void InternalTexturePublicInterface::LoadGPU(Texture& texture) {
    Renderer::genAndBindTexture(texture.m_Type, texture.m_TextureAddress[0]);
    switch (texture.m_TextureType) {
        case TextureType::RenderTarget: {
            TextureLoader::LoadTextureFramebufferIntoOpenGL(texture);
            break;
        }case TextureType::Texture1D: {
            break;
        }case TextureType::Texture2D: {
            TextureLoader::LoadTexture2DIntoOpenGL(texture);
            break;
        }case TextureType::Texture3D: {
            break;
        }case TextureType::CubeMap: {
            TextureLoader::LoadTextureCubemapIntoOpenGL(texture);
            break;
        }
        default: { break; }
    }
    if (texture.m_IsToBeMipmapped) {
        TextureLoader::GenerateMipmapsOpenGL(texture);
    }

    cout << "(Texture) ";
    texture.EngineResource::load();
}
void InternalTexturePublicInterface::UnloadCPU(Texture& texture) {
    for (auto& image : texture.m_ImagesDatas) {
        if (image->filename != "") {
            for (auto& mipmap : image->mipmaps) {
                if (mipmap.pixels.size() == 0) {
                    vector_clear(mipmap.pixels);
                }
            }
        }
    }
    texture.m_Mipmapped = false;

    cout << "(Texture) ";
    texture.EngineResource::unload();
}
void InternalTexturePublicInterface::UnloadGPU(Texture& texture) {
    for (uint i = 0; i < texture.m_TextureAddress.size(); ++i) {
        glDeleteTextures(1, &texture.m_TextureAddress[i]);
    }
    vector_clear(texture.m_TextureAddress);
}
void InternalTexturePublicInterface::Load(Texture& texture) {
    if (!texture.isLoaded()) {
        InternalTexturePublicInterface::LoadCPU(texture);
        InternalTexturePublicInterface::LoadGPU(texture);
    }
}
void InternalTexturePublicInterface::Unload(Texture& texture) {
    if (texture.isLoaded()) {
        InternalTexturePublicInterface::UnloadGPU(texture);
        InternalTexturePublicInterface::UnloadCPU(texture);
    }
}
void InternalTexturePublicInterface::Resize(Texture& texture, Engine::epriv::FramebufferObject& fbo, const uint& width, const uint& height) {
    if (texture.m_TextureType != TextureType::RenderTarget) {
        cout << "Error: Non-framebuffer texture cannot be resized. Returning..." << endl;
        return;
    }
    const float _divisor = fbo.divisor();
    Renderer::bindTextureForModification(texture.m_Type, texture.m_TextureAddress[0]);
    const uint _w = static_cast<uint>(static_cast<float>(width) * _divisor);
    const uint _h = static_cast<uint>(static_cast<float>(height) * _divisor);
    auto& imageData = *texture.m_ImagesDatas[0];
    imageData.mipmaps[0].width = _w;
    imageData.mipmaps[0].height = _h;
    glTexImage2D(texture.m_Type, 0, imageData.internalFormat, _w, _h, 0, imageData.pixelFormat, imageData.pixelType, NULL);
}