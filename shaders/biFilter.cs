// Ref: https://www.shadertoy.com/view/4dfGDH
#version 430

layout (binding = 0, r16ui) readonly uniform uimage2DArray dataIn;		// Input depth map
layout (binding = 1, r16ui) writeonly uniform uimage2DArray dataOut;	// Output depth map

uniform float depthScale;
uniform float sigma;
uniform float bSigma;
const int mSize = 15;

layout (local_size_x = 32, local_size_y = 32) in;

float normPdf(float x, float sigma)
{
	return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

void main(void)
{
	ivec2 u = ivec2(gl_GlobalInvocationID.xy);
	float c = float(imageLoad(dataIn, ivec3(u, 0)).r) * depthScale;

	const int kSize = (mSize - 1) / 2;
	float kernel[mSize];
	float finalColor = 0.0;

	// Create the 1-D kernel
	float Z = 0.0;
	for (int i = 0; i <= kSize; ++i)
	{
		kernel[kSize + i] = kernel[kSize - i] = normPdf(float(i), sigma);
	}

	float cc;
	float factor;
	float bZ = 1.0 / normPdf(0.0, bSigma);
	// Read out the texels
	for (int x = -kSize; x <= kSize; ++x)
	{
		for (int y = -kSize; y <= kSize; ++y)
		{
			cc = float(imageLoad(dataIn, ivec3(u + ivec2(x, y), 0)).r) * depthScale;
			factor = normPdf(cc - c, bSigma) * bZ * kernel[kSize + y] * kernel[kSize + x];
			Z += factor;
			finalColor += factor * cc;
		}
	}

	imageStore(dataOut, ivec3(u, 0), uvec4((finalColor / Z) / depthScale, 0.0, 0.0, 1.0));
}