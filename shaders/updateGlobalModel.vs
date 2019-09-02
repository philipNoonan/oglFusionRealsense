#version 430 core

layout(location = 0) in vec4 vertexConfidence;
layout(location = 1) in vec4 normalRadius;
layout(location = 2) in vec4 colorTimeDevice;

layout(std430, binding = 0) buffer updateIndexMapBuffer
{
    vec4 updateIndexInterleaved [];
};


out vec4 geoVertPosConf;
out vec4 geoVertNormRadi;
out vec4 geoVertColTimDev;
flat out int toFuse;


uniform float texDim;
uniform int time;

float encodeColor(vec3 c)
{
    int rgb = int(round(c.x * 255.0f));
    rgb = (rgb << 8) + int(round(c.y * 255.0f));
    rgb = (rgb << 8) + int(round(c.z * 255.0f));
    return float(rgb);
}

vec3 decodeColor(float c)
{
    vec3 col;
    col.x = float(int(c) >> 16 & 0xFF) / 255.0f;
    col.y = float(int(c) >> 8 & 0xFF) / 255.0f;
    col.z = float(int(c) & 0xFF) / 255.0f;
    return col;
}

void main()
{
    // this gets run for every vertex in the global model



   toFuse = 0;


    vec4 newColor = updateIndexInterleaved[(gl_VertexID * 3) + 2];

    //Do averaging here
    if (newColor.w == -1)
    {
		toFuse = 1;
        vec4 newPos = updateIndexInterleaved[(gl_VertexID * 3)];
        vec4 newNorm = updateIndexInterleaved[(gl_VertexID * 3) + 1];

        float c_k = vertexConfidence.w;
        vec3 v_k = vertexConfidence.xyz;
        
        float a = newPos.w;
        vec3 v_g = newPos.xyz;
        
        if(newNorm.w < (1.0 + 0.5) * normalRadius.w)
        {
            // vert conf output
            geoVertPosConf = vec4(((c_k * v_k) + (a * v_g)) / (c_k + a), c_k + a);
	        
	        vec3 oldCol = decodeColor(colorTimeDevice.x);
	        vec3 newCol = decodeColor(newColor.x);
           
            vec3 avgColor = ((c_k * oldCol.xyz) + (a * newCol.xyz)) / (c_k + a);

            // norm radius output
            geoVertColTimDev = vec4(encodeColor(avgColor), colorTimeDevice.y, colorTimeDevice.z, time);
	        
	        geoVertNormRadi = ((c_k * normalRadius) + (a * newNorm)) / (c_k + a);

            // color time device output
            geoVertNormRadi = vec4(normalize(geoVertNormRadi.xyz), geoVertNormRadi.w);
        }
        else if(newColor.w == -2) // unstable vert
		{
			toFuse = 1;
            geoVertPosConf = vec4(vertexConfidence.xyz, c_k + a);
            geoVertNormRadi = normalRadius;
            geoVertColTimDev = vec4(colorTimeDevice.xyz, colorTimeDevice);
        }
    }
    else
    {
        //This point isn't being updated, so just transfer it
        geoVertPosConf = vertexConfidence;
        geoVertNormRadi = normalRadius;
        geoVertColTimDev = colorTimeDevice;
    }


    // need to wipe the update buffer after every update, this should always ensure that the active buffer values are set off after use
    updateIndexInterleaved[(gl_VertexID * 3)] = vec4(0.0f);
    updateIndexInterleaved[(gl_VertexID * 3) + 1] = vec4(0.0f);
    updateIndexInterleaved[(gl_VertexID * 3) + 2] = vec4(0.0f);

	

}
