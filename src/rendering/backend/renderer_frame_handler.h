#ifndef HYPERION_V2_BACKEND_RENDERER_FRAME_HANDLER_H
#define HYPERION_V2_BACKEND_RENDERER_FRAME_HANDLER_H

#include <util/defines.h>

#if HYP_VULKAN
#include <rendering/backend/vulkan/renderer_frame_handler.h>
#else
#error Unsupported rendering backend
#endif

#endif