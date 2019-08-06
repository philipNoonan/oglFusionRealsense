#version 430 core

in vec2 texCoord;
in float radi;
flat in int unstablePoint;

layout(location = 0) out vec4 fragmentColor;

void main() {

	if (dot(texCoord, texCoord) > 1.0f)
	{
		discard;
	}

	fragmentColor = vec4(texCoord.xy, 0.0f, 1.0f);

	if (unstablePoint == 1)
	{
		gl_FragDepth = gl_FragCoord.z + radi;
	}
	else
	{
		gl_FragDepth = gl_FragCoord.z;
	}

}