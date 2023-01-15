#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

#include "Texture.h"
#include "SpriteRenderer.h"
#include "Shader.h"

class PostProcessor
{
public:
	Shader PostProcessingShader;
	Texture2D Texture;
	unsigned int Width, Height;
	bool Confuse, Chaos, Shake;
public:
	PostProcessor(Shader shader, unsigned int width, unsigned int height);
	void BeginRender();
	void EndRender();
	void Render(float time);

private:
	unsigned int m_MSFBO = 0, m_FBO = 0;
	unsigned int m_RBO = 0;
	unsigned int m_VAO = 0;
private:
	void initRenderData();
};