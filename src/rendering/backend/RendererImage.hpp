#ifndef HYPERION_V2_BACKEND_RENDERER_IMAGE_H
#define HYPERION_V2_BACKEND_RENDERER_IMAGE_H

#include <util/Defines.hpp>
#include <math/MathUtil.hpp>
#include <Types.hpp>

namespace hyperion::renderer {

enum class ImageType : UInt32
{
    TEXTURE_TYPE_2D = 0,
    TEXTURE_TYPE_3D = 1,
    TEXTURE_TYPE_CUBEMAP = 2
};

enum class BaseFormat : UInt32
{
    TEXTURE_FORMAT_NONE,
    TEXTURE_FORMAT_R,
    TEXTURE_FORMAT_RG,
    TEXTURE_FORMAT_RGB,
    TEXTURE_FORMAT_RGBA,
    
    TEXTURE_FORMAT_BGR,
    TEXTURE_FORMAT_BGRA,

    TEXTURE_FORMAT_DEPTH
};

enum class InternalFormat : UInt32
{
    NONE,

    R8,
    RG8,
    RGB8,
    RGBA8,
    
    B8,
    BG8,
    BGR8,
    BGRA8,

    R16,
    RG16,
    RGB16,
    RGBA16,

    R32,
    RG32,
    RGB32,
    RGBA32,

    R32_,
    RG16_,
    R11G11B10F,
    R10G10B10A2,

    R16F,
    RG16F,
    RGB16F,
    RGBA16F,

    R32F,
    RG32F,
    RGB32F,
    RGBA32F,

    SRGB, /* begin srgb */

    R8_SRGB,
    RG8_SRGB,
    RGB8_SRGB,
    RGBA8_SRGB,
    
    B8_SRGB,
    BG8_SRGB,
    BGR8_SRGB,
    BGRA8_SRGB,
    
    DEPTH, /* begin depth */

    DEPTH_16 = DEPTH,
    DEPTH_24,
    DEPTH_32F
};

enum class FilterMode : UInt32
{
    TEXTURE_FILTER_NEAREST,
    TEXTURE_FILTER_LINEAR,
    TEXTURE_FILTER_NEAREST_MIPMAP,
    TEXTURE_FILTER_LINEAR_MIPMAP,
    TEXTURE_FILTER_MINMAX_MIPMAP
};

enum class WrapMode : UInt32
{
    TEXTURE_WRAP_CLAMP_TO_EDGE,
    TEXTURE_WRAP_CLAMP_TO_BORDER,
    TEXTURE_WRAP_REPEAT
};

enum class TextureMode : UInt32
{
    SAMPLED,
    STORAGE
};

static inline BaseFormat GetBaseFormat(InternalFormat fmt)
{
    switch (fmt) {
    case InternalFormat::R8:
    case InternalFormat::R8_SRGB:
    case InternalFormat::R32_:
    case InternalFormat::R16:
    case InternalFormat::R32:
    case InternalFormat::R16F:
    case InternalFormat::R32F:
        return BaseFormat::TEXTURE_FORMAT_R;
    case InternalFormat::RG8:
    case InternalFormat::RG8_SRGB:
    case InternalFormat::RG16_:
    case InternalFormat::RG16:
    case InternalFormat::RG32:
    case InternalFormat::RG16F:
    case InternalFormat::RG32F:
        return BaseFormat::TEXTURE_FORMAT_RG;
    case InternalFormat::RGB8:
    case InternalFormat::RGB8_SRGB:
    case InternalFormat::R11G11B10F:
    case InternalFormat::RGB16:
    case InternalFormat::RGB32:
    case InternalFormat::RGB16F:
    case InternalFormat::RGB32F:
        return BaseFormat::TEXTURE_FORMAT_RGB;
    case InternalFormat::RGBA8:
    case InternalFormat::RGBA8_SRGB:
    case InternalFormat::R10G10B10A2:
    case InternalFormat::RGBA16:
    case InternalFormat::RGBA32:
    case InternalFormat::RGBA16F:
    case InternalFormat::RGBA32F:
        return BaseFormat::TEXTURE_FORMAT_RGBA;
    case InternalFormat::BGR8_SRGB:
        return BaseFormat::TEXTURE_FORMAT_BGR;
    case InternalFormat::BGRA8:
    case InternalFormat::BGRA8_SRGB:
        return BaseFormat::TEXTURE_FORMAT_BGRA;
    case InternalFormat::DEPTH_16:
    case InternalFormat::DEPTH_24:
    case InternalFormat::DEPTH_32F:
        return BaseFormat::TEXTURE_FORMAT_DEPTH;
    default:
        // undefined result
        return BaseFormat::TEXTURE_FORMAT_NONE;
    }
}

static inline UInt NumComponents(BaseFormat format)
{
    switch (format) {
    case BaseFormat::TEXTURE_FORMAT_NONE: return 0;
    case BaseFormat::TEXTURE_FORMAT_R: return 1;
    case BaseFormat::TEXTURE_FORMAT_RG: return 2;
    case BaseFormat::TEXTURE_FORMAT_RGB: return 3;
    case BaseFormat::TEXTURE_FORMAT_BGR: return 3;
    case BaseFormat::TEXTURE_FORMAT_RGBA: return 4;
    case BaseFormat::TEXTURE_FORMAT_BGRA: return 4;
    case BaseFormat::TEXTURE_FORMAT_DEPTH: return 1;
    default: return 0; // undefined result
    }
}

static inline UInt NumComponents(InternalFormat format)
{
    return NumComponents(GetBaseFormat(format));
}

/*! \brief returns a texture format that has a shifted bytes-per-pixel count
 * e.g calling with RGB16 and num components = 4 --> RGBA16 */
static inline InternalFormat FormatChangeNumComponents(InternalFormat fmt, UInt8 new_num_components)
{
    if (new_num_components == 0) {
        return InternalFormat::NONE;
    }

    new_num_components = MathUtil::Clamp(new_num_components, static_cast<UInt8>(1), static_cast<UInt8>(4));

    int current_num_components = int(NumComponents(fmt));

    return InternalFormat(int(fmt) + int(new_num_components) - current_num_components);
}

static inline bool IsDepthFormat(BaseFormat fmt)
{
    return fmt == BaseFormat::TEXTURE_FORMAT_DEPTH;
}

static inline bool IsDepthFormat(InternalFormat fmt)
{
    return IsDepthFormat(GetBaseFormat(fmt));
}

static inline bool IsSRGBFormat(InternalFormat fmt)
{
    return fmt >= InternalFormat::SRGB && fmt < InternalFormat::DEPTH;
}

} // namespace hyperion::renderer

#if HYP_VULKAN
#include <rendering/backend/vulkan/RendererImage.hpp>
#else
#error Unsupported rendering backend
#endif

// to reduce noise, pull the above enum classes into hyperion namespace
namespace hyperion {

using renderer::ImageType;
using renderer::BaseFormat;
using renderer::InternalFormat;
using renderer::FilterMode;
using renderer::WrapMode;
using renderer::TextureMode;

} // namespace hyperion

#endif