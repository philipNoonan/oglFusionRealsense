#version 430
#extension GL_ARB_shader_viewport_layer_array : require

layout(location=0) in vec2 position;
out vec2 image_pos;
flat out vec2 flow_du;
flat out float mean_diff;
flat out int image0_layer, image1_layer;

uniform vec2 patch_size;  // In 0..1 coordinates.
layout(binding = 0) uniform sampler2D sparseFlowMap;

uniform int level;

void main()
{
	int num_patches = textureSize(sparseFlowMap, level).x * textureSize(sparseFlowMap, level).y;
	int patch_layer = gl_InstanceID / num_patches;
	int patch_x = gl_InstanceID % textureSize(sparseFlowMap, level).x;
	int patch_y = (gl_InstanceID % num_patches) / textureSize(sparseFlowMap, level).x;

	vec2 patch_center = ivec2(patch_x, patch_y) / (textureSize(sparseFlowMap, level).xy - 1.0);

	vec2 grown_pos = (position * 1.5) - 0.25;

	image_pos = patch_center + patch_size * (grown_pos - 0.5f);

	vec3 flow_du_and_mean_diff = texelFetch(sparseFlowMap, ivec2(patch_x, patch_y), level).xyz;
	flow_du = flow_du_and_mean_diff.xy;
	mean_diff = flow_du_and_mean_diff.z;

	gl_Position = vec4(2.0 * image_pos.x - 1.0, 2.0 * image_pos.y - 1.0, -1.0, 1.0);
	gl_Layer = patch_layer;
  
	// Forward flow (0) goes from 0 to 1. Backward flow (1) goes from 1 to 0.
	image0_layer = patch_layer;
	image1_layer = 1 - patch_layer;

}