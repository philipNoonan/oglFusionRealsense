#version 430 core

layout (binding=0) uniform sampler2D currentTextureDepth;
layout (binding=1) uniform sampler2D currentTextureNormal;

layout (binding=2) uniform isampler3D currentTextureVolume; 

in vec3 boxCenter;
in vec3 boxRadius;
in vec3 TexCoord3D;

uniform mat4 invView; 
uniform mat4 invProj;
uniform mat4 invModel;
