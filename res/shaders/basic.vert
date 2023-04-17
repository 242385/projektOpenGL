#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 4) in ivec4 skinIndices;
layout (location = 5) in vec4 skinWeights;

uniform mat4 transform;

out vec2 TexCoords;

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(std140, binding = 1) uniform SkinningBuffer {
    mat4 bones[512];
} skin;

void main()
{
	const vec4 pos = vec4(aPos, 1.0f);
    const vec4 norm = vec4(aNormal, 0.0f);
    vec4 posSkinned = {0.0f, 0.0f, 0.0f, 0.0f};
    vec4 normSkinned = {0.0f, 0.0f, 0.0f, 0.0f};

    for(int i=0; i<4; ++i)
    {
        if(skinIndices[i] >= 0)
        {
            const mat4 bone = skin.bones[skinIndices[i]];
            const float weight = skinWeights[i];
			
            posSkinned += (bone * pos) * weight;
            normSkinned += (bone * norm) * weight;
        }
    }

    posSkinned.w = 1.0f;

    gl_Position = ubo.proj * ubo.view * ubo.model * posSkinned;
}