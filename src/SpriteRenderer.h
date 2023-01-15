#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Texture.h"

class SpriteRenderer
{
public:
	
public:
	SpriteRenderer(const Shader& shader);
	~SpriteRenderer()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}
	void DrawSprite(const Texture2D& texture, glm::vec2 position,
		glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f,
		glm::vec3 color = glm::vec3(1.0f));
private:
	void initSpriteData();
private:
	unsigned int m_VAO;
	Shader m_Shader;
};