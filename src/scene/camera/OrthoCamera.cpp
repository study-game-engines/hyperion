#include "OrthoCamera.hpp"

namespace hyperion::v2 {
OrthoCameraController::OrthoCameraController()
    : OrthoCameraController(
          -100.0f, 100.0f,
          -100.0f, 100.0f,
          -100.0f, 100.0f
      )
{
}

OrthoCameraController::OrthoCameraController(Float left, Float right, Float bottom, Float top, Float _near, Float _far)
    : CameraController(CameraType::ORTHOGRAPHIC),
      m_left(left),
      m_right(right),
      m_bottom(bottom),
      m_top(top),
      m_near(_near),
      m_far(_far)
{
}

void OrthoCameraController::OnAdded(Camera *camera)
{
    CameraController::OnAdded(camera);

    camera->SetToOrthographicProjection(
        m_left, m_right,
        m_bottom, m_top,
        m_near, m_far
    );
}

void OrthoCameraController::UpdateLogic(double dt)
{
}

void OrthoCameraController::UpdateViewMatrix()
{
    m_camera->m_view_mat = Matrix4::LookAt(
        m_camera->m_translation,
        m_camera->GetTarget(),
        m_camera->m_up
    );
}

void OrthoCameraController::UpdateProjectionMatrix()
{
    m_camera->SetToOrthographicProjection(
        m_camera->m_left,   m_camera->m_right,
        m_camera->m_bottom, m_camera->m_top,
        m_camera->m_near,   m_camera->m_far
    );
}
} // namespace hyperion::v2
