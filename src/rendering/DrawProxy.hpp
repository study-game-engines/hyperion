#ifndef HYPERION_V2_DRAW_PROXY_H
#define HYPERION_V2_DRAW_PROXY_H

#include <core/Base.hpp>
#include <rendering/RenderBucket.hpp>

#include <math/BoundingBox.hpp>
#include <math/Vector4.hpp>
#include <math/Color.hpp>
#include <math/Frustum.hpp>

#include <core/ID.hpp>

#include <rendering/backend/RendererBuffer.hpp>
#include <rendering/backend/RendererStructs.hpp>
#include <rendering/backend/RendererFrame.hpp>

#include <Constants.hpp>
#include <Types.hpp>

#include <memory>
#include <atomic>

namespace hyperion::v2 {

using renderer::IndirectDrawCommand;
using renderer::IndirectBuffer;
using renderer::StorageBuffer;
using renderer::StagingBuffer;
using renderer::Frame;
using renderer::Result;
;
using renderer::ShaderVec2;
using renderer::ShaderVec4;
using renderer::ShaderValue;

class Mesh;
class Material;
class Engine;
class Entity;
class Camera;
class Skeleton;
class Scene;
class EnvProbe;
class Light;

void WaitForRenderUpdatesToComplete();

enum class LightType : UInt32
{
    DIRECTIONAL,
    POINT,
    SPOT
};

template <class T>
struct DrawProxy
{
private:
    DrawProxy(); // break intentionally; needs specialization
};

template <>
struct DrawProxy<STUB_CLASS(Material)>
{
    // TODO
};

using MaterialDrawProxy = DrawProxy<STUB_CLASS(Material)>;

template <>
struct DrawProxy<STUB_CLASS(Entity)>
{
    ID<Entity> entity_id;
    ID<Mesh> mesh_id;
    ID<Material> material_id;
    ID<Skeleton> skeleton_id;

    BoundingBox bounding_box;

    Transform transform;

    Mesh *mesh = nullptr;

    Bucket bucket = Bucket::BUCKET_OPAQUE;
};

using EntityDrawProxy = DrawProxy<STUB_CLASS(Entity)>;

template <>
struct DrawProxy<STUB_CLASS(Camera)>
{
    Matrix4 view;
    Matrix4 projection;
    Matrix4 previous_view;
    Vector3 position;
    Vector3 direction;
    Vector3 up;
    Extent2D dimensions;
    Float clip_near;
    Float clip_far;
    Float fov;
    Frustum frustum;

    UInt64 visibility_bitmask;
    UInt16 visibility_nonce;
};

using CameraDrawProxy = DrawProxy<STUB_CLASS(Camera)>;

template <>
struct DrawProxy<STUB_CLASS(Scene)>
{
    UInt32 frame_counter;
};

using SceneDrawProxy = DrawProxy<STUB_CLASS(Scene)>;

using EnvProbeFlags = UInt32;

enum EnvProbeFlagBits : EnvProbeFlags
{
    ENV_PROBE_FLAGS_NONE = 0x0,
    ENV_PROBE_FLAGS_PARALLAX_CORRECTED = 0x1,
    ENV_PROBE_FLAGS_SHADOW = 0x2,
    ENV_PROBE_FLAGS_DIRTY = 0x4,
    ENV_PROBE_FLAGS_MAX = 0x7 // 3 bits after are used for shadow
};

using ShadowFlags = UInt32;

enum ShadowFlagBits : ShadowFlags
{
    SHADOW_FLAGS_NONE = 0x0,
    SHADOW_FLAGS_PCF = 0x1,
    SHADOW_FLAGS_VSM = 0x2,
    SHADOW_FLAGS_CONTACT_HARDENED = 0x4
};

template <>
struct DrawProxy<STUB_CLASS(EnvProbe)>
{
    ID<EnvProbe> id;
    BoundingBox aabb;
    Vector3 world_position;
    UInt32 texture_index;
    Float camera_near;
    Float camera_far;
    EnvProbeFlags flags;
    UInt32 grid_slot;
};

using EnvProbeDrawProxy = DrawProxy<STUB_CLASS(EnvProbe)>;

template <>
struct DrawProxy<STUB_CLASS(Light)>
{
    ID<Light> id;
    LightType type;
    Color color;
    Float radius;
    Float falloff;
    UInt32 shadow_map_index;
    Vector4 position_intensity;
    UInt64 visibility_bits; // bitmask indicating if light is visible to cameras by camera ID
};

using LightDrawProxy = DrawProxy<STUB_CLASS(Light)>;



template <class T>
class HasDrawProxy
{
public:
    HasDrawProxy()
        : m_draw_proxy{}
    {
    }

    HasDrawProxy(const DrawProxy<T> &draw_proxy)
        : m_draw_proxy(draw_proxy)
    {
    }

    HasDrawProxy(DrawProxy<T> &&draw_proxy)
        : m_draw_proxy(std::move(draw_proxy))
    {
    }

    template <class ...Args>
    HasDrawProxy(Args &&... args)
        : m_draw_proxy(std::forward<Args>(args)...)
    {
    }

    HasDrawProxy(const HasDrawProxy &other)
        : m_draw_proxy(other.m_draw_proxy)
    {
    }

    HasDrawProxy &operator=(const HasDrawProxy &other)
    {
        m_draw_proxy = other.m_draw_proxy;
        return *this;
    }

    HasDrawProxy(HasDrawProxy &&other) noexcept
        : m_draw_proxy(std::move(other.m_draw_proxy))
    {
    }

    HasDrawProxy &operator=(HasDrawProxy &&other) noexcept
    {
        m_draw_proxy = std::move(other.m_draw_proxy);
        return *this;
    }

    ~HasDrawProxy() = default;

    /*! \brief Get the DrawProxy for this object. Only call from render thread. */
    const DrawProxy<T> &GetDrawProxy() const { return m_draw_proxy; }

protected:
    bool HasPendingRenderUpdates() const
        { return m_has_render_updates.load(std::memory_order_acquire) != 0; }

    // Only touch from render thread.
    // Update this when updates are enqueued, so just update
    // the shader data state to dirty to refresh this.
    DrawProxy<T> m_draw_proxy;

private:
    std::atomic_uint m_has_render_updates { 0u };
};

} // namespace hyperion::v2

#endif