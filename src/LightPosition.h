#pragma once
#include "Model.h"

class LightPosition : public SceneObject
{
protected:
    GLuint VAO{}, VBO{}, EBO{};
    Shader shader;

public:
    glm::mat4 world{};

    LightPosition();
    ~LightPosition() override;
    void draw(const Shader& shader) const override;

    void drawSphere(glm::vec3 position, glm::vec4 color, glm::mat4 proview) const;
    void drawArrow(glm::vec3 start, glm::vec3 end, glm::vec4 color, glm::mat4 proview) const;
};
