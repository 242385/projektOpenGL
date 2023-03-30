#pragma once

#include "Shader.h"

class SceneObject
{
public:
	virtual ~SceneObject() = default;
	virtual void draw(const Shader& shader) const = 0;
};
