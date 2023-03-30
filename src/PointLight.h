#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

class PointLight
{
public:
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) :
		position(position), ambient(ambient), diffuse(diffuse), specular(specular) {};

private:
	GLuint VBO, VAO, EBO;
};
