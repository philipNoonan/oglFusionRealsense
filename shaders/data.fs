#version 430 core

in vec4 outVertPosConf;
in vec4 outVertNormRadi;
in vec4 outVertColTimDev;
flat in int outUpdateId;

layout(location = 0) out vec4 fragVertPosConf;
layout(location = 1) out vec4 fragVertNormRadi;
layout(location = 2) out vec4 fragVertColTimDev;

void main() 
{
    //If we have a point to update in the existing model, store that
    if(outUpdateId == 1)
    {
        fragVertPosConf = outVertPosConf;
        fragVertNormRadi = outVertNormRadi;
        fragVertColTimDev = outVertColTimDev;
    }
}