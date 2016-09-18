#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#define VELOCITY_EPSILON 0.001
#define POSITION_EPSILON 0.003

#include "physics2/rigid_body.h"
#include "physics2/collision.h"
#include "physics2/box_physics_shape.h"
#include "physics2/sphere_physics_shape.h"
#include "physics2/plane_physics_shape.h"

#include <vector>
#include <memory>

namespace apex {
class PhysicsManager {
public:
    static PhysicsManager *GetInstance();

    PhysicsManager();
    ~PhysicsManager();

    void RegisterBody(std::shared_ptr<physics::Rigidbody> body);
    void ResetCollisions();
    void DetectCollisions();
    void RunPhysics(double dt);
    void UpdateInternals(std::vector<physics::CollisionInfo> &collisions, double dt);
    void UpdateVelocities(std::vector<physics::CollisionInfo> &collisions, double dt);
    void UpdatePositions(std::vector<physics::CollisionInfo> &collisions, double dt);

private:
    static PhysicsManager *instance;
    std::vector<std::shared_ptr<physics::Rigidbody>> m_bodies;
};
}

#endif