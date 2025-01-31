#include "Triangle.hpp"
#include "MathUtil.hpp"

namespace hyperion {
Triangle::Triangle()
    : m_points{}
{
}

Triangle::Triangle(const Vertex &v0, const Vertex &v1, const Vertex &v2)
    : m_points({ v0, v1, v2 })
{
}

Triangle::Triangle(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2)
    : m_points({ Vertex(v0), Vertex(v1), Vertex(v2) })
{
}

Triangle::Triangle(const Triangle &other)
    : m_points(other.m_points)
{
}

Vertex &Triangle::Closest(const Vector3 &vec)
{
    Float distances[3];
    UInt shortest_index = 0;

    for (UInt i = 0; i < 3; i++) {
        distances[i] = m_points[i].GetPosition().DistanceSquared(vec);

        if (i != 0) {
            if (distances[i] < distances[shortest_index]) {
                shortest_index = i;
            }
        }
    }

    return m_points[shortest_index];
}

const Vertex &Triangle::Closest(const Vector3 &vec) const
{
    return const_cast<Triangle *>(this)->Closest(vec);
}
} // namespace
