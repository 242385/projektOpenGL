#pragma once

#include "Model.h"

class Node
{
public:
	Node() : local(1.0f), dirty(true), sceneObject(nullptr) {}
	Node(SceneObject* sceneObject) : local(1.0f), dirty(true), sceneObject(sceneObject) {}

	std::vector<Node*> children; // Scene graph
	glm::vec4 modulate = glm::vec4(1.0f);


	void setTransform(glm::mat4 newLocal)
	{
		local = newLocal;
		dirty = true;
	}

	void setWorld(glm::mat4 newWorld)
	{
		world = newWorld * local;
		for (int i = 0; i < children.size(); i++)
		{
			children[i]->setWorld(world);
		}
	}

	glm::mat4 getWorld() const
	{
		return world;
	}

	void getNewWorld(glm::mat4 parentWorld, bool isDirty) {
		isDirty |= dirty;
		if (isDirty)
		{
			world = parentWorld * local;
			dirty = false;
		}
		for (Node* child : children)
		{
			child->getNewWorld(world, dirty);
		}
	}

	void draw(glm::mat4 parentWorld, Shader shader) const
	{
		shader.setMat4("model", world);
		if (sceneObject != nullptr)
		{
			sceneObject->draw(shader);
		}
		for (Node* child : children)
		{
			child->draw(parentWorld, shader);
		}
	}

	void drawThis(glm::mat4 parentWorld, Shader shader) const
	{
		shader.setMat4("model", world);
		if (sceneObject != nullptr) 
		{
			sceneObject->draw(shader);
		}
	}

	void addChild(Node* child)
	{
		children.push_back(child);
	}

	glm::mat4 getLocal() const { return local; }

private:
	//Shader shader;

	glm::vec3 scale = glm::vec3(1.0f);
	glm::mat4 world;
	glm::mat4 local;

	bool dirty;

	// Scene graph variables
	SceneObject* sceneObject;
	int childrenNum = 0;
};
