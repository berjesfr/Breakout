#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <irrklang/irrKlang.h>
#include <vector>
#include <tuple>
#include "SpriteRenderer.h"
#include "ResourceManager.h"
#include "GameLevel.h"
#include "BallObject.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "PowerUp.h"
#include "TextRenderer.h"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN,
};

class Game {
public:
	GameState state;
	std::vector<GameLevel> Levels;
	std::vector<PowerUp> PowerUps;
	int Level;
	bool keys[1024];
	bool KeysProcessed[1024];
	unsigned int width, height;
	unsigned int lives;
	Game(unsigned int width, unsigned int height);
	~Game();
	void Init();
	void ProcessInput(float dt);
	void SpawnPowerUps(GameObject& block);
	void UpdatePowerUps(float dt);
	void ActivatePowerUp(PowerUp& powerUp);
	void Update(float dt);
	void Render();
	void DoCollisions();
	void ResetPlayer();
	void ResetLevel();
};