#version 430 core

uniform int level;
uniform vec2 invDenseTexSize;

flat in vec4 flow; // flow.xy, meanDiff.z

out vec4 flow_contribution;

layout(binding = 1, rgba32f) uniform image2D testMap;

layout(binding = 0) uniform sampler2D lastImage;
layout(binding = 1) uniform sampler2D nextImage;

float luminance(vec3 rgb)
{
    return (0.299f * float(rgb.x) + 0.587f * float(rgb.y) + 0.114f * float(rgb.z));
}

void main()
{         
	float diff = luminance(textureLod(lastImage, vec2(gl_FragCoord.xy) * invDenseTexSize, level).xyz) - luminance(textureLod(nextImage, vec2(gl_FragCoord.xy * invDenseTexSize) + flow.xy, level).xyz);
	

	//imageStore(testMap, ivec2(gl_FragCoord.xy), vec4(flow.xy,0, 1));
	

	diff -= flow.z;
	float weight = 1.0 / max(abs(diff), 2.0f);
	flow_contribution = vec4(flow.x * weight, flow.y * weight, weight, diff);
	//flow_contribution = vec4(flow.xyz, 1);
}