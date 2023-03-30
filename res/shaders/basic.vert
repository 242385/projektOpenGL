#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;

uniform mat4 transform;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = transform * aInstanceMatrix * vec4(aPos, 1.0);
}
