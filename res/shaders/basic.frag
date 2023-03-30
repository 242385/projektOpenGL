#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform vec4 modulate;
uniform sampler2D texture_diffuse1;

void main()
{
   FragColor = texture(texture_diffuse1, TexCoords) * modulate;
}
