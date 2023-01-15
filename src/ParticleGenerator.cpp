#include "ParticleGenerator.h"

// constructor
ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount)
	:m_Shader(shader), m_Texture(texture), m_Amount(amount)
{
	init();
}

void ParticleGenerator::init()
{
    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    glCreateVertexArrays(1, &m_VAO);
    glCreateBuffers(1, &VBO);

    glNamedBufferData(VBO, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    glEnableVertexArrayAttrib(m_VAO, 0);
    glVertexArrayAttribBinding(m_VAO, 0, 0);
    glVertexArrayAttribFormat(m_VAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayVertexBuffer(m_VAO, 0, VBO, 0, 4 * sizeof(float));

	for (unsigned int i = 0; i < m_Amount; ++i)
		m_Particles.push_back(Particle());
}
// update all particles
void ParticleGenerator::Update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset)
{
    for (unsigned int i = 0; i < newParticles; ++i) {
        int unusedParticle = firstUnusedParticle();
        respawnParticle(m_Particles[unusedParticle], object, offset);
    }
    // update all particles
    for (unsigned int i = 0; i < m_Amount; ++i) {
        Particle& p = m_Particles[i];
        p.Life -= dt; // reduce life
        if (p.Life > 0.0f) {	// particle is alive, thus update
            p.Position -= p.Velocity * dt;
            p.Color.a -= dt * 2.5f;
        }
    }
}
// render all particles
void ParticleGenerator::Draw()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    m_Shader.Use();
    glBindVertexArray(m_VAO);
    for (unsigned int i = 0; i < m_Particles.size(); i++) {
        if (m_Particles[i].Life > 0.0f) {
            m_Shader.SetVector2f("offset", m_Particles[i].Position);
            m_Shader.SetVector4f("color", m_Particles[i].Color);
            m_Texture.Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    // don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
unsigned int lastUsedParticle = 0;
unsigned int ParticleGenerator::firstUnusedParticle()
{
    for (unsigned int i = lastUsedParticle; i < m_Amount; ++i) {
        if (m_Particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (m_Particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // override first particle if all others are alive
    lastUsedParticle = 0;
    return 0;
}
// respawns particle
void ParticleGenerator::respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset)
{
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle.Position = object.Position + random + offset;
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = 1.0f;
    particle.Velocity = object.Velocity * 0.1f;
}