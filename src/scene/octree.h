#ifndef HYPERION_V2_OCTREE_H
#define HYPERION_V2_OCTREE_H

#include <core/containers.h>
#include "scene.h"
#include "spatial.h"
#include "visibility_state.h"

#include <math/vector3.h>
#include <math/bounding_box.h>

#include <array>

#define HYP_OCTREE_DEBUG 0

namespace hyperion {

class Camera;

} // namespace hyperion

namespace hyperion::v2 {

class Spatial;

class Octree {
    friend class Engine;
    friend class Spatial;

    enum {
        DEPTH_SEARCH_INF = -1,
        DEPTH_SEARCH_ONLY_THIS = 0
    };

    struct Callback {
        using CallbackFunction = std::function<void(Engine *, Octree *, Spatial *)>;
    };

    Octree(Octree *parent, const BoundingBox &aabb, uint32_t index);

public:
    struct Octant {
        std::unique_ptr<Octree> octree;
        BoundingBox             aabb;
    };

    struct Node {
        Ref<Spatial> spatial;
        BoundingBox  aabb;

        VisibilityState *visibility_state = nullptr;
    };

    struct Root {
        struct Events : ComponentEvents<Callback> {
            CallbackGroup on_insert_octant,
                          on_remove_octant,
                          on_insert_node,
                          on_remove_node;
        } events;

        std::unordered_map<Spatial *, Octree *> node_to_octree;
        std::atomic_uint32_t                    visibility_cursor{0};
    };

    static bool IsVisible(const Octree *parent, const Octree *child);

    Octree(const BoundingBox &aabb);
    ~Octree();

    inline Root *GetRoot() const { return m_root; }

    inline BoundingBox &GetAabb() { return m_aabb; }
    inline const BoundingBox &GetAabb() const { return m_aabb; }

    inline auto &GetCallbacks()
    {
        AssertThrow(m_root != nullptr);

        return m_root->events;
    }

    inline const auto &GetCallbacks() const
        { return const_cast<Octree *>(this)->GetCallbacks(); }

    inline VisibilityState &GetVisibilityState()             { return m_visibility_state; }
    inline const VisibilityState &GetVisibilityState() const { return m_visibility_state; }

    void Clear(Engine *engine);
    bool Insert(Engine *engine, Spatial *spatial);
    bool Remove(Engine *engine, Spatial *spatial);
    bool Update(Engine *engine, Spatial *spatial);
    void CalculateVisibility(Scene *scene);

private:
    bool Move(Engine *engine, Spatial *spatial, const std::vector<Node>::iterator *it = nullptr);

    inline auto FindNode(Spatial *spatial)
    {
        return std::find_if(m_nodes.begin(), m_nodes.end(), [spatial](const Node &node) {
            return node.spatial == spatial;
        });
    }

    inline bool IsRoot() const { return m_parent == nullptr; }
    inline bool Empty() const  { return m_nodes.empty(); }
    
    void SetParent(Octree *parent);
    bool EmptyDeep(int depth = DEPTH_SEARCH_INF, uint32_t mask = 0) const;

    void InitOctants();
    void Divide(Engine *engine);
    void Undivide(Engine *engine);

    /* Remove any potentially empty octants above the node */
    void CollapseParents(Engine *engine);
    bool InsertInternal(Engine *engine, Spatial *spatial);
    bool UpdateInternal(Engine *engine, Spatial *spatial);
    bool RemoveInternal(Engine *engine, Spatial *spatial);
    void UpdateVisibilityState(Scene *scene);

    /* Called from Spatial - remove the pointer */
    void OnSpatialRemoved(Engine *engine, Spatial *spatial);
    
    std::vector<Node> m_nodes;
    Octree *m_parent;
    BoundingBox m_aabb;
    std::array<Octant, 8> m_octants;
    bool m_is_divided;
    Root *m_root;
    VisibilityState m_visibility_state;
    uint32_t m_index;
};

} // namespace hyperion::v2

#endif