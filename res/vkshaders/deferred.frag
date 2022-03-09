#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec3 v_position;
layout(location=1) in vec2 v_texcoord0;
layout(location=2) in vec3 v_light_direction;
layout(location=3) in vec3 v_camera_position;

layout(location=0) out vec4 out_color;

layout(set = 1, binding = 0) uniform sampler2D gbuffer_albedo;
layout(set = 1, binding = 1) uniform sampler2D gbuffer_normals;
layout(set = 1, binding = 2) uniform sampler2D gbuffer_positions;

void main() {
    vec2 texcoord = vec2(v_texcoord0.x, 1.0 - v_texcoord0.y);

    vec4 albedo = texture(gbuffer_albedo, texcoord);
    vec4 normal = texture(gbuffer_normals, texcoord);
    vec4 position = texture(gbuffer_positions, texcoord);
    
    float NdotL = dot(normal.xyz, v_light_direction);

    out_color = vec4(vec3(max(NdotL, 0.025)) * albedo.rgb, 1.0);
}