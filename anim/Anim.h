#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
#include <vector>

#include "Shader.h"

class Anim
{
public:
    // Likely the inputs from a vertex shader
    glm::vec3 inPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 inNormal = glm::vec3(0.0f, 0.0f, 0.0f);

    // Parameters
private:
    const glm::vec4 pos = glm::vec4(inPosition, 1.0f);
    const glm::vec4 norm = glm::vec4(inNormal, 0.0f);
    GLuint positionLocation = 0;
    GLuint normalLocation = 0;

public:

    void DisableAttributeArrays();
    glm::vec4 posSkinned = { 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 normSkinned = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Shaders
    Shader shaderInstance;

    // Constructor
    Anim();

    // Destructor
    virtual ~Anim();
};