#pragma once
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glad/glad.h>

class SpotLight
{
public:
	glm::vec3 position = glm::vec3(0.7f, 0.2f, 2.0f);
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
	float cutOff = glm::cos(glm::radians(12.5f));
	float outerCutOff = glm::cos(glm::radians(15.0f));

	SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) :
		position(position), direction(direction), ambient(ambient), diffuse(diffuse), specular(specular) {};

};
