#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <Shader.h>

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Texture {
    GLuint id;
    string type;
    string path;
};

class Mesh {
public:
    GLuint VAO, VBO, EBO;
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;

    Mesh(const vector<Vertex>& vertices, const vector<GLuint>& indices, const vector<Texture>& textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        setupMesh();
    }

    void draw(const Shader& shader) const
    {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint normalNr = 1;
        GLuint heightNr = 1;
        for (GLuint i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
            {
                number = std::to_string(diffuseNr++);
            }
            else if (name == "texture_specular")
            {
                number = std::to_string(specularNr++);
            }
            else if (name == "texture_normal")
            {
                number = std::to_string(normalNr++);
            }
            else if (name == "texture_height")
            {
                number = std::to_string(heightNr++);
            }
            glUniform1i(glGetUniformLocation(shader.id, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        //draw mesh
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, static_cast<GLint>(indices.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        //vertices
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void*>(nullptr));

        //normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        //textures
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoords)));

        glBindVertexArray(0);
    }
};
