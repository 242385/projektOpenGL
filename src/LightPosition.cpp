#include "LightPosition.h"

#include "glm/glm.hpp"
#include <vector>
#include <glm/ext/scalar_constants.hpp>


LightPosition::LightPosition()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);

    shader = Shader("../../res/shaders/indicator.vert", "../../res/shaders/indicator.frag");
}

LightPosition::~LightPosition() = default;

// We do that to be able to make LightPosition a SceneObject so that it can be added as a Node to a scene
void LightPosition::draw(const Shader& shader) const {}

void LightPosition::drawSphere(glm::vec3 position, glm::vec4 color, glm::mat4 proview) const
{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    for (int i = 0; i <= 10; i++)
    {
        const float V = i / 10.0f;
        const float phi = V * glm::pi<float>();

        for (int k = 0; k <= 10; k++)
        {
            const float U = k / 10.0f;
            const float theta = U * (glm::pi<float>() * 2);

            // calculate vertex positions
            float x = cosf(theta) * sinf(phi) * 1.0f;
            float y = cosf(phi) * 1.0f;
            float z = sinf(theta) * sinf(phi) * 1.0f;
            vertices.push_back(x + position.x);
            vertices.push_back(y + position.y);
            vertices.push_back(z + position.z);
        }
    }

    // calculate index
    for (int i = 0; i < 110; i++)
    {
        indices.push_back(i);
        indices.push_back(i + 11);
        indices.push_back(i + 10);

        indices.push_back(i + 11);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(GLfloat)), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);

    shader.use();
    shader.setVec4("modulate", color);
    shader.setMat4("proview", proview);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

void LightPosition::drawArrow(glm::vec3 start, glm::vec3 end, glm::vec4 color, glm::mat4 proview) const
{
    glm::vec3 direction = glm::normalize(end - start);

    glm::vec3 tipOffset = -direction * 0.5f;
    glm::vec3 tipLeft = end - glm::cross(direction, glm::vec3(0.f, 0.5f, 0.f)) + tipOffset;
    glm::vec3 tipRight = end + glm::cross(direction, glm::vec3(0.f, 0.5f, 0.f)) + tipOffset;

    std::vector<glm::vec3> vertices = { start, end, tipLeft, tipRight };
    std::vector<GLuint> indices = { 0, 1, 2, 3, 1, 0 };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

    shader.use();
    shader.setVec4("modulate", color);
    shader.setMat4("proview", proview);

    glDrawElements(GL_LINE_STRIP, indices.size(), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}