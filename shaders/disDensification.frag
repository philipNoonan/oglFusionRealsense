#version 430 core

in vec2 image_pos;
flat in int image0_layer, image1_layer;
flat in vec2 flow_du;
flat in float mean_diff;
out vec3 flow_contribution;

uniform sampler2DArray image_tex;

void main()
{         
	float diff = texture(image_tex, vec3(image_pos, image0_layer)).x - texture(image_tex, vec3(image_pos + flow_du, image1_layer)).x;
	diff -= mean_diff;
	float weight = 1.0 / max(abs(diff), 2.0 / 255.0);
	flow_contribution = vec3(flow_du.x * weight, flow_du.y * weight, weight);
}