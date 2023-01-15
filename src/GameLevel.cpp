#include "GameLevel.h"


void GameLevel::Load(const char* file, unsigned int levelWidth, unsigned int levelHeight)
{
	bricks.clear();
	unsigned int tileCode;
	std::string line;
	std::ifstream fstream(file);
	std::vector<std::vector<unsigned int>> tileData;
	if (fstream)
	{
		while (std::getline(fstream, line))
		{
			std::istringstream sstream(line);
			std::vector<unsigned int> row;
			while (sstream >> tileCode)
				row.push_back(tileCode);
			tileData.push_back(row);
		}
		if (tileData.size() > 0)
			Init(tileData, levelWidth, levelHeight);
	}
}

void GameLevel::Init(std::vector<std::vector<unsigned int>>& tileData, unsigned int levelWidth, unsigned int levelHeight)
{
	unsigned int height = tileData.size();
	unsigned int width = tileData[0].size();
	float unitWidth = levelWidth / static_cast<float>(width);
	float unitHeight = levelHeight / (height);
	
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (tileData[y][x] == 1) {
				glm::vec2 pos(unitWidth * x, unitHeight * y);
				glm::vec2 size(unitWidth, unitHeight);

				GameObject obj = GameObject(pos, size, ResourceManager::GetTexture("brick_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
				obj.IsSolid = true;
				bricks.push_back(obj);
			}
			else if (tileData[y][x] > 1) {
				glm::vec3 color = glm::vec3(1.0f);
				if (tileData[y][x] == 2)
					color = glm::vec3(0.2f, 0.6f, 1.0f);
				else if (tileData[y][x] == 3)
					color = glm::vec3(0.0f, 0.7f, 0.0f);
				else if (tileData[y][x] == 4)
					color = glm::vec3(0.8f, 0.8f, 0.4f);
				else if (tileData[y][x] == 5)
					color = glm::vec3(1.0f, 0.5f, 0.0f);


				glm::vec2 pos(unitWidth * x, unitHeight * y);
				glm::vec2 size(unitWidth, unitHeight);

				GameObject obj(pos, size, ResourceManager::GetTexture("brick"), color);
				bricks.push_back(obj);
			}
		}
	}
}

void GameLevel::Draw(SpriteRenderer& renderer)
{
	for (GameObject& tile : bricks) {
		if (!tile.Destroyed)
			tile.Draw(renderer);
	}
}

bool GameLevel::IsCompleted()
{
	for (GameObject& tile : bricks)
		if (!tile.IsSolid && !tile.Destroyed)
			return false;
	return true;
}
