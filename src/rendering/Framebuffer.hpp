#ifndef HYPERION_V2_FRAMEBUFFER_H
#define HYPERION_V2_FRAMEBUFFER_H

#include <core/Base.hpp>

#include <rendering/backend/RenderObject.hpp>
#include <rendering/backend/RendererFramebuffer.hpp>
#include <rendering/backend/RendererRenderPass.hpp>
#include <rendering/backend/RendererCommandBuffer.hpp>

namespace hyperion::v2 {

using renderer::CommandBuffer;
;
using renderer::AttachmentUsage;
using renderer::Attachment;
using renderer::RenderPass;
using renderer::RenderPassStage;
using renderer::LoadOperation;
using renderer::StoreOperation;

struct AttachmentDef
{
    AttachmentRef attachment;
    AttachmentUsageRef attachment_usage;
    LoadOperation load_op;
    StoreOperation store_op;
};

struct AttachmentMap
{
    FlatMap<UInt, AttachmentDef> attachments;

    void AddAttachment(
        UInt binding,
        ImageRef &&image,
        RenderPassStage stage,
        LoadOperation load_op,
        StoreOperation store_op
    )
    {
        attachments.Set(
            binding,
            AttachmentDef {
                RenderObjects::Make<Attachment>(
                    std::move(image),
                    stage
                ),
                AttachmentUsageRef(),
                load_op,
                store_op
            }
        );
    }

    ~AttachmentMap();
};

class Framebuffer
    : public EngineComponentBase<STUB_CLASS(Framebuffer)>,
      public RenderResource
{
public:
    Framebuffer(
        Extent2D extent,
        RenderPassStage stage = RenderPassStage::SHADER,
        RenderPass::Mode render_pass_mode = RenderPass::Mode::RENDER_PASS_INLINE,
        UInt num_multiview_layers = 0
    );

    Framebuffer(
        Extent3D extent,
        RenderPassStage stage = RenderPassStage::SHADER,
        RenderPass::Mode render_pass_mode = RenderPass::Mode::RENDER_PASS_INLINE,
        UInt num_multiview_layers = 0
    );

    Framebuffer(const Framebuffer &other) = delete;
    Framebuffer &operator=(const Framebuffer &other) = delete;
    ~Framebuffer();

    void AddAttachment(
        UInt binding,
        ImageRef &&image,
        RenderPassStage stage,
        LoadOperation load_op,
        StoreOperation store_op
    )
    {
        m_attachment_map.AddAttachment(
            binding,
            std::move(image),
            stage,
            load_op,
            store_op
        );
    }

    void AddAttachmentUsage(AttachmentUsage *attachment);
    void RemoveAttachmentUsage(const Attachment *attachment);

    auto &GetAttachmentUsages() { return m_render_pass.GetAttachmentUsages(); }
    const auto &GetAttachmentUsages() const { return m_render_pass.GetAttachmentUsages(); }

    renderer::FramebufferObject &GetFramebuffer(UInt frame_index) { return m_framebuffers[frame_index]; }
    const renderer::FramebufferObject &GetFramebuffer(UInt frame_index) const { return m_framebuffers[frame_index]; }

    renderer::RenderPass &GetRenderPass() { return m_render_pass; }
    const renderer::RenderPass &GetRenderPass() const { return m_render_pass; }

    Extent2D GetExtent() const
        { return { m_framebuffers[0].GetWidth(), m_framebuffers[0].GetHeight() }; }

    void Init();

    void BeginCapture(UInt frame_index, CommandBuffer *command_buffer);
    void EndCapture(UInt frame_index, CommandBuffer *command_buffer);

private:
    AttachmentMap m_attachment_map;
    FixedArray<renderer::FramebufferObject, max_frames_in_flight> m_framebuffers;
    RenderPass m_render_pass;
};

} // namespace hyperion::v2

#endif // !HYPERION_V2_FRAMEBUFFER_H
