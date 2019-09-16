#version 430 core

layout(points) in;
layout(points, max_vertices = 1) out;

flat in int vertID[];
flat in int newUnstable[];

flat out int outUpdateId;

void main() 
{
   
		if (newUnstable[0] == 1)
		{
			outUpdateId = vertID[0];
			EmitVertex();
			EndPrimitive(); 
		}
	   

    
}