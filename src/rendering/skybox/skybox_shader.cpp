#include "skybox_shader.h"
#include "../environment.h"
#include "../../asset/asset_manager.h"
#include "../../asset/text_loader.h"
#include "../../util/shader_preprocessor.h"

namespace hyperion {
SkyboxShader::SkyboxShader(const ShaderProperties &properties)
    : Shader(properties)
{
    const std::string vs_path("shaders/skybox.vert");
    const std::string fs_path("shaders/skybox.frag");

    AddSubShader(
        Shader::SubShaderType::SUBSHADER_VERTEX,
        AssetManager::GetInstance()->LoadFromFile<TextLoader::LoadedText>(vs_path)->GetText(),
        properties,
        vs_path
    );

    AddSubShader(
        Shader::SubShaderType::SUBSHADER_FRAGMENT,
        AssetManager::GetInstance()->LoadFromFile<TextLoader::LoadedText>(fs_path)->GetText(),
        properties,
        fs_path
    );

    m_uniform_camera_position = m_uniforms.Acquire("u_camerapos").id;

    m_uniform_directional_light_direction = m_uniforms.Acquire("env_DirectionalLight.direction").id;
    m_uniform_directional_light_color = m_uniforms.Acquire("env_DirectionalLight.color").id;
    m_uniform_directional_light_intensity = m_uniforms.Acquire("env_DirectionalLight.intensity").id;
}

void SkyboxShader::ApplyMaterial(const Material &mat)
{
    Shader::ApplyMaterial(mat);

    auto env = Environment::GetInstance();

    SetUniform(m_uniform_directional_light_direction, env->GetSun().GetDirection());
    SetUniform(m_uniform_directional_light_color, env->GetSun().GetColor());
    SetUniform(m_uniform_directional_light_intensity, env->GetSun().GetIntensity());
}

void SkyboxShader::ApplyTransforms(const Transform &transform, Camera *camera)
{
    Transform updated_transform(transform);
    updated_transform.SetTranslation(camera->GetTranslation());

    Shader::ApplyTransforms(updated_transform, camera);

    SetUniform(m_uniform_camera_position, camera->GetTranslation());
}
} // namespace hyperion
