#version 330 core

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

out vec4 FragColor;

void main()
{
    vec3 I = normalize(Position - cameraPos);
    vec3 n = normalize(Normal);
    vec3 ratio = vec3(1.00 / 1.52, 1.00 / 1.55, 1.00 / 1.58);

    vec3 refractR = refract(I, n, ratio.r);
    vec3 refractG = refract(I, n, ratio.g);
    vec3 refractB = refract(I, n, ratio.b);

    FragColor.r = texture(skybox, refractR).r;
    FragColor.g = texture(skybox, refractG).g;
    FragColor.b = texture(skybox, refractB).b;
    FragColor.a = 1.0;
}  