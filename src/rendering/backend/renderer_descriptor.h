#ifndef HYPERION_RENDERER_DESCRIPTOR_H
#define HYPERION_RENDERER_DESCRIPTOR_H

#include "renderer_buffer.h"
#include "renderer_image_view.h"
#include "renderer_sampler.h"
#include "renderer_image.h"
#include "../uniform_buffer.h"

#include "../../util/non_owning_ptr.h"

#include <vector>

namespace hyperion {
namespace renderer {
class Device;
class DescriptorSet;

class Descriptor {
    friend class DescriptorSet;
public:
    enum class Mode {
        UNSET,
        UNIFORM_BUFFER,
        UNIFORM_BUFFER_DYNAMIC,
        STORAGE_BUFFER,
        STORAGE_BUFFER_DYNAMIC,
        IMAGE_SAMPLER,
        IMAGE_STORAGE
    };

    struct SubDescriptor {
        GPUBuffer *gpu_buffer = nullptr;
        uint32_t range = 0; /* 0 --> gpu_buffer->size*/

        ImageView *image_view = nullptr;
        Sampler *sampler = nullptr;
    };

    struct Info {
        VkDescriptorSetLayoutBinding binding;
        VkWriteDescriptorSet write;
    };

    enum State {
        DESCRIPTOR_CLEAN = 0,
        DESCRIPTOR_DIRTY = 1
    };

    Descriptor(uint32_t binding, Mode mode, VkShaderStageFlags stage_flags);
    Descriptor(const Descriptor &other) = delete;
    Descriptor &operator=(const Descriptor &other) = delete;
    ~Descriptor();

    inline uint32_t GetBinding() const { return m_binding; }
    inline void SetBinding(uint32_t binding) { m_binding = binding; }

    inline State GetState() const { return m_state; }
    inline void SetState(State state) { m_state = state; }

    inline const std::vector<SubDescriptor> &GetSubDescriptors() const
        { return m_sub_descriptors; }

    inline Descriptor *AddSubDescriptor(SubDescriptor &&sub_descriptor)
        { m_sub_descriptors.push_back(sub_descriptor); return this; }


    void Create(Device *device, Info *out_info);
    void Destroy(Device *device);

protected:
    struct BufferInfo {
        std::vector<VkDescriptorBufferInfo> buffers;
        std::vector<VkDescriptorImageInfo>  images;
    };

    static VkDescriptorType GetDescriptorType(Mode mode);

    std::vector<SubDescriptor> m_sub_descriptors;
    BufferInfo m_sub_descriptor_buffer;
    State m_state;
    
    uint32_t m_binding;
    Mode m_mode;
    VkShaderStageFlags m_stage_flags;

private:
    non_owning_ptr<DescriptorSet> m_descriptor_set;
};

class UniformBufferDescriptor : public Descriptor {
public:
    UniformBufferDescriptor(
        uint32_t binding,
        VkShaderStageFlags stage_flags
    ) : Descriptor(
        binding,
        Mode::UNIFORM_BUFFER,
        stage_flags)
    {}
};

class DynamicUniformBufferDescriptor : public Descriptor {
public:
    DynamicUniformBufferDescriptor(
        uint32_t binding,
        VkShaderStageFlags stage_flags
    ) : Descriptor(
        binding,
        Mode::UNIFORM_BUFFER_DYNAMIC,
        stage_flags)
    {}
};

class StorageBufferDescriptor : public Descriptor {
public:
    StorageBufferDescriptor(
        uint32_t binding,
        VkShaderStageFlags stage_flags
    ) : Descriptor(
        binding,
        Mode::STORAGE_BUFFER,
        stage_flags)
    {}
};

class DynamicStorageBufferDescriptor : public Descriptor {
public:
    DynamicStorageBufferDescriptor(
        uint32_t binding,
        VkShaderStageFlags stage_flags
    ) : Descriptor(
        binding,
        Mode::STORAGE_BUFFER_DYNAMIC,
        stage_flags)
    {}
};

class ImageSamplerDescriptor : public Descriptor {
public:
    ImageSamplerDescriptor(
        uint32_t binding,
        VkShaderStageFlags stage_flags
    ) : Descriptor(
        binding,
        Mode::IMAGE_SAMPLER,
        stage_flags)
    {}
};

class ImageStorageDescriptor : public Descriptor {
public:
    ImageStorageDescriptor(
        uint32_t binding,
        VkShaderStageFlags stage_flags
    ) : Descriptor(
        binding,
        Mode::IMAGE_STORAGE,
        stage_flags)
    {}
};

} // namespace renderer
} // namespace hyperion

#endif