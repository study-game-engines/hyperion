#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_samplerless_texture_functions : enable

#define HYP_DO_NOT_DEFINE_DESCRIPTOR_SETS
#include "../include/defines.inc"
#include "../include/shared.inc"
#include "../include/material.inc"
#include "../include/packing.inc"
#undef HYP_DO_NOT_DEFINE_DESCRIPTOR_SETS

layout(location=0) in vec3 v_position;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec2 v_texcoord0;
layout(location=3) in vec4 v_color;

layout(location = 0) out vec4 gbuffer_albedo;
// layout(location=1) out vec4 gbuffer_normals;
// layout(location=2) out vec4 gbuffer_material;
// layout(location=3) out vec4 gbuffer_tangents;
// layout(location=4) out vec2 gbuffer_velocity;
// layout(location=5) out vec4 gbuffer_mask;

layout(set = 0, binding = 6) uniform texture2D albedo_texture;
layout(set = 0, binding = 7) uniform sampler texture_sampler;

void main()
{
    vec4 color = Texture2D(texture_sampler, albedo_texture, v_texcoord0);
    color *= v_color;

    gbuffer_albedo = color;
}
