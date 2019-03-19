#version 430 core
out vec4 FragColor;

in VS_OUT {
    vec2 flow;
} vs_in;



void main()
{
    FragColor = vec4(vs_in.flow, 0.0, 1.0);
}