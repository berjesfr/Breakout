#pragma once

#include "GameObject.h"
#include "ResourceManager.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

class GameLevel
{
public:
	std::vector<GameObject> bricks;
	GameLevel() {}

	void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);
	void Draw(SpriteRenderer& renderer);
	bool IsCompleted();
private:
	void Init(std::vector<std::vector<unsigned int>>& tileData, unsigned int levelWidth, unsigned int levelHeight);
};
