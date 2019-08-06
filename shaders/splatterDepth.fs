#version 430 core

in vec3 positions;
in vec3 normals;
in vec4 colTimDev;

layout(location = 0) out vec4 fragment_color;

void main() {

	fragment_color = vec4(normals.xyz, 1);

}