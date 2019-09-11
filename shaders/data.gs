#version 430 core

layout(points) in;
layout(points, max_vertices = 1) out;

flat in int vertID[];
flat in int newUnstable[];

flat out int outUpdateId;

void main() 
{
    //  only emit a vertex if it is a new unstable
	//  this will leave a buffer of only new unstable, and the IDs of which will be perfectly ordered to match and be appended to the global buffer

		if (newUnstable[0] == 1)
		{
			outUpdateId = vertID[0];
			EmitVertex();
			EndPrimitive(); 
		}
	   

    
}