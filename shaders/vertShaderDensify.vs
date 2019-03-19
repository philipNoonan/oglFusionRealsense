#version 430 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 flowField;

out VS_OUT {
    vec2 flow;
} vs_out;



void main()
{
	vs_out.flow = flowField;
	gl_Position = vec4(position, 0, 1);
}
