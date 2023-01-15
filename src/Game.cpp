#include "Game.h"

SpriteRenderer* Renderer;
GameObject *Player;
BallObject* Ball;
ParticleGenerator *particleGenerator;
PostProcessor* Effects;
TextRenderer* Text;
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;
float ShakeTime = 0.0f;

enum class Direction {
	UP, RIGHT, DOWN, LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;


Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, -1.0f),
		glm::vec2(-1.0f, 0.0f)
	};

	float max = 0.0f;
	unsigned int bestMatch = -1;

	for (unsigned int i = 0; i < 4; i++) {
		float dotProduct = glm::dot(glm::normalize(target), compass[i]);
		if (dotProduct > max) {
			max = dotProduct;
			bestMatch = i;
		}
	}
	return (Direction)bestMatch;
}


float clamp(float value, float min, float max) {
	return std::max(min, std::min(max, value));
}

bool isColliding(GameObject& one, GameObject& two)
{
	bool collisionX = one.Position.x <= two.Position.x + two.Size.x && one.Position.x + one.Size.x >= two.Position.x;
	bool collisionY = one.Position.y <= two.Position.y + two.Size.y && one.Position.y + one.Size.y >= two.Position.y;

	return collisionX && collisionY;
}

Collision isColliding(BallObject& one, GameObject& two)
{
	glm::vec2 aabbHalfExtents = glm::vec2(two.Size.x / 2, two.Size.y / 2);
	glm::vec2 aabbCenter = glm::vec2(two.Position.x + aabbHalfExtents.x, two.Position.y + aabbHalfExtents.y);
	glm::vec2 circleCenter = one.Position + one.Radius;
	glm::vec2 D = circleCenter - aabbCenter;

	glm::vec2 P = aabbCenter + glm::vec2(clamp(D.x, -aabbHalfExtents.x, aabbHalfExtents.x), clamp(D.y, -aabbHalfExtents.y, aabbHalfExtents.y));
	
	D = P - circleCenter;
	if (one.Radius >= glm::length(D)) {
		return std::make_tuple(true, VectorDirection(D), D);
	}
	else {
		return std::make_tuple(false, Direction::UP, glm::vec2( 0.0f, 0.0f ));
	}
}

bool ShouldSpawn(unsigned int chance)
{
	unsigned int random = rand() % chance;
	return random == 0;
}

bool IsOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
	for (const PowerUp& powerUp : powerUps) {
		if (powerUp.Activated)
			if (powerUp.Type == type)
				return true;
	}
	return false;
}


Game::Game(unsigned int width, unsigned int height)
	: state(GAME_ACTIVE), width(width), height(height), lives(3)
{

}
Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete particleGenerator;
	delete Effects;
	delete SoundEngine;
}
void Game::Init()
{
	state = GAME_MENU;
	ResourceManager::LoadShader("src/shaders/sprite_vertex.glsl", "src/shaders/sprite_fragment.glsl", nullptr, "sprite");
	ResourceManager::LoadShader("src/shaders/particle_shader_vertex.glsl", "src/shaders/particle_shader_frag.glsl", nullptr, "particle");
	ResourceManager::LoadShader("src/shaders/postprocessing_vertex.glsl", "src/shaders/postprocessing_fragment.glsl", nullptr, "effects");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);
	Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

	ResourceManager::LoadTexture("assets/background.jpg", false, "background");
	ResourceManager::LoadTexture("assets/block.png", false, "brick");
	ResourceManager::LoadTexture("assets/block_solid.png", false, "brick_solid");
	ResourceManager::LoadTexture("assets/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("assets/awesomeface.png", true, "ball");
	ResourceManager::LoadTexture("assets/powerup_chaos.png", true, "tex_chaos");
	ResourceManager::LoadTexture("assets/powerup_confuse.png", true, "tex_confuse");
	ResourceManager::LoadTexture("assets/powerup_sticky.png", true, "tex_sticky");
	ResourceManager::LoadTexture("assets/powerup_increase.png", true, "tex_increase");
	ResourceManager::LoadTexture("assets/powerup_passthrough.png", true, "tex_passthrough");
	ResourceManager::LoadTexture("assets/powerup_speed.png", true, "tex_speed");

	GameLevel one; one.Load("src/levels/one.lvl", width, height/2);
	GameLevel two; two.Load("src/levels/two.lvl", width, height / 2);
	GameLevel three; three.Load("src/levels/three.lvl", width, height / 2);
	GameLevel four; four.Load("src/levels/four.lvl", width, height / 2);

	Levels.push_back(one);
	Levels.push_back(two);
	Levels.push_back(three);
	Levels.push_back(four);
	Level = 0;

	glm::vec2 playerPos = glm::vec2(
		width / 2.0f - PLAYER_SIZE.x / 2.0f,
		height - PLAYER_SIZE.y
	);
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
		-BALL_RADIUS * 2.0f);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("ball"));
	particleGenerator = new ParticleGenerator(
		ResourceManager::GetShader("particle"),
		ResourceManager::GetTexture("particle"),
		500
	);
	Effects = new PostProcessor(ResourceManager::GetShader("effects"), width, height);
	Text = new TextRenderer(this->width, this->height);
	Text->Load("fonts/ocraext.ttf", 24);
	SoundEngine->play2D("assets/breakout.mp3", true);
}
void Game::ProcessInput(float dt) 
{
	if (state == GAME_ACTIVE) {
		float velocity = PLAYER_VELOCITY * dt;
		if (keys[GLFW_KEY_A]) {
			if (Player->Position.x >= 0.0f) {
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}	
		}
		if (keys[GLFW_KEY_D]) {
			if (Player->Position.x <= width - Player->Size.x) {
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if (keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	} else if (this->state == GAME_MENU) {
		if (this->keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER]) {
			this->state = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
		if (this->keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W]) {
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_W] = true;
		}
		if (this->keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S]) {
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			this->KeysProcessed[GLFW_KEY_S] = true;
		}
	} else if (this->state == GAME_WIN) {
		if (this->keys[GLFW_KEY_ENTER]) {
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
			Effects->Chaos = false;
			this->state = GAME_MENU;
		}
	}
}
void Game::Update(float dt)
{
	Ball->Move(dt, width);
	this->DoCollisions();
	particleGenerator->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
	this->UpdatePowerUps(dt);
	if (ShakeTime > 0.0f) {
		ShakeTime -= dt;
		if (ShakeTime <= 0.0f) Effects->Shake = false;
	}
	if (Ball->Position.y >= this->height) // did ball reach bottom edge?
	{
		--this->lives;
		if (this->lives == 0) {
			this->ResetLevel();
			this->state = GAME_MENU;
		}
		this->ResetPlayer();
	}
	if (this->state == GAME_ACTIVE && this->Levels[this->Level].IsCompleted()) {
		this->ResetLevel();
		this->ResetPlayer();
		Effects->Chaos = true;
		this->state = GAME_WIN;
	}
}

void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : this->PowerUps) {
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated) {
			powerUp.Duration -= dt;

			if (powerUp.Duration <= 0.0f) {
				// remove powerup from list (will later be removed)
				powerUp.Activated = false;
				// deactivate effects
				if (powerUp.Type == "sticky") {
					if (!IsOtherPowerUpActive(this->PowerUps, "sticky")) {	// only reset if no other PowerUp of type sticky is active
						Ball->Sticky = false;
						Player->Color = glm::vec3(1.0f);
					}
				} else if (powerUp.Type == "pass-through") {
					if (!IsOtherPowerUpActive(this->PowerUps, "pass-through")) {	// only reset if no other PowerUp of type pass-through is active
						Ball->PassThrough = false;
						Ball->Color = glm::vec3(1.0f);
					}
				} else if (powerUp.Type == "confuse") {
					if (!IsOtherPowerUpActive(this->PowerUps, "confuse")) {	// only reset if no other PowerUp of type confuse is active
						Effects->Confuse = false;
					}
				} else if (powerUp.Type == "chaos") {
					if (!IsOtherPowerUpActive(this->PowerUps, "chaos")) {	// only reset if no other PowerUp of type chaos is active
						Effects->Chaos = false;
					}
				}
			}
		}
	}
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
		[](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
	), this->PowerUps.end());
}

void Game::Render()
{
	if (state == GAME_ACTIVE || state == GAME_MENU) {
		Effects->BeginRender();
		Renderer->DrawSprite(ResourceManager::GetTexture("background"),
			glm::vec2(0.0f, 0.0f), glm::vec2(width, height), 0.0f
		);
		Levels[Level].Draw(*Renderer);
		Player->Draw(*Renderer);
		particleGenerator->Draw();
		Ball->Draw(*Renderer);
		for (PowerUp& powerUp : this->PowerUps) {
			if (!powerUp.Destroyed) {
				powerUp.Draw(*Renderer);
			}
		}
		std::stringstream ss; ss << this->lives;
		Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
		Effects->EndRender();
		Effects->Render(glfwGetTime());
	}
	if (this->state == GAME_MENU) {
		Text->RenderText("Press ENTER to start", 250.0f, height / 2, 1.0f);
		Text->RenderText("Press W or S to select level", 245.0f, height / 2 + 20.0f, 0.75f);
	}
	if (this->state == GAME_WIN) {
		Text->RenderText(
			"You WON!!!", 320.0, height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0)
		);
		Text->RenderText(
			"Press ENTER to retry or ESC to quit", 130.0, height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0)
		);
	}
}

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].bricks) {
        if (!box.Destroyed) {

			Collision collision = isColliding(*Ball, box);
			if (std::get<0>(collision)) {
				if (!box.IsSolid) {
					SoundEngine->play2D("assets/bleep.mp3");
					box.Destroyed = true;
					SpawnPowerUps(box);
				} else {
					SoundEngine->play2D("assets/solid.wav");
					ShakeTime = 0.05f;
					Effects->Shake = true;
				}
				Direction dir = std::get<1>(collision);
				glm::vec2 diffVector = std::get<2>(collision);
				if (!(Ball->PassThrough && !box.IsSolid)) {
					if (dir == Direction::LEFT || dir == Direction::RIGHT) {
						Ball->Velocity.x = -Ball->Velocity.x;
						float R = Ball->Radius - std::abs(diffVector.x);
						if (dir == Direction::LEFT) Ball->Position.x += R;
						else Ball->Position.x -= R;
					} else {
						Ball->Velocity.y = -Ball->Velocity.y;
						float R = Ball->Radius - std::abs(diffVector.y);
						if (dir == Direction::DOWN) Ball->Position.y += R;
						else Ball->Position.y -= R;
					}
				}
            }
        }
    }
	Collision result = isColliding(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		SoundEngine->play2D("assets/bleep.wav");
		Ball->Stuck = Ball->Sticky;
		// check where it hit the board, and change velocity based on where it hit the board
		float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
		float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		float percentage = distance / (Player->Size.x / 2.0f);
		// then move accordingly
		float strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
	}
	for (PowerUp& powerUp : this->PowerUps) {
		if (!powerUp.Destroyed) {
			if (powerUp.Position.y >= this->height)
				powerUp.Destroyed = true;
			if (isColliding(*Player, powerUp)) {	// collided with player, now activate powerup
				SoundEngine->play2D("assets/powerup.wav");
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = true;
				powerUp.Activated = true;
			}
		}
	}
}  

void Game::ActivatePowerUp(PowerUp& powerUp)
{
	if (powerUp.Type == "speed") {
		Ball->Velocity *= 1.2;
	} else if (powerUp.Type == "sticky") {
		Ball->Sticky = true;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	} else if (powerUp.Type == "pass-through") {
		Ball->PassThrough = true;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	} else if (powerUp.Type == "pad-size-increase") {
		Player->Size.x += 50;
	} else if (powerUp.Type == "confuse") {
		if (!Effects->Chaos)
			Effects->Confuse = true; // only activate if chaos wasn't already active
	} else if (powerUp.Type == "chaos") {
		if (!Effects->Confuse)
			Effects->Chaos = true;
	}
}

void Game::SpawnPowerUps(GameObject& block)
{
	if (ShouldSpawn(75)) // 1 in 75 chance
		this->PowerUps.push_back(
			PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("tex_speed")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(
			PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("tex_sticky")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(
			PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("tex_passthrough")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(
			PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("tex_increase")));
	if (ShouldSpawn(15)) // negative powerups should spawn more often
		this->PowerUps.push_back(
			PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("tex_confuse")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(
			PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("tex_chaos")));
}

void Game::ResetPlayer()
{
	delete Player;
	delete Ball;

	glm::vec2 playerPos = glm::vec2(
		width / 2.0f - PLAYER_SIZE.x / 2.0f,
		height - PLAYER_SIZE.y
	);
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
		-BALL_RADIUS * 2.0f);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("ball"));
}

void Game::ResetLevel()
{
	this->lives = 3;
	for (GameObject& brick : this->Levels[Level].bricks)
	{
		brick.Destroyed = false;
	}
}