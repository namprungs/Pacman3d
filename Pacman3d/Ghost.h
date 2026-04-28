#pragma once
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <memory>

class Map;
class Player;
class Animation;
class Animator;
class Model;

enum class GhostState
{
    CHASE,
    SCATTER,
    FRIGHTENED
};

enum class GhostType
{
    BLINKY,
    PINKY,
    INKY,
    CLYDE
};

class Ghost
{
public:
    Ghost(GhostType type, const glm::ivec2& startGridPosition, const glm::ivec2& scatterCorner, const glm::vec3& color);
    ~Ghost();

    Ghost(const Ghost&) = delete;
    Ghost& operator=(const Ghost&) = delete;
    Ghost(Ghost&& other) noexcept;
    Ghost& operator=(Ghost&& other) noexcept;

    void update(float deltaTime, const Map& map, const Player& player);
    void updateAnimation(float deltaTime);
    void draw(GLuint shaderProgram) const;

    void triggerFrightened(float durationSeconds);
    bool checkCollisionWithPlayer(Player& player, const Map& map);
    void reset();

    glm::vec3 getPosition() const;
    GhostState getState() const;

private:
    void setFrightened(bool frightened);
    void triggerAttack();
    void updateAnimationState();
    void respawnAwayFromPlayer(const Map& map, const Player& player);

    glm::ivec2 chooseNextCell(const Map& map, const Player& player) const;
    glm::ivec2 getChaseTarget(const Player& player) const;
    bool isWalkableCell(const glm::ivec2& gridCell, const Map& map) const;
    bool intersectsAabb(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB) const;

    GhostType type_;
    GhostState state_;

    glm::ivec2 currentGridPosition_;
    glm::ivec2 targetGridPosition_;
    glm::ivec2 previousGridPosition_;
    glm::ivec2 scatterCorner_;
    glm::ivec2 spawnGridPosition_;

    glm::vec3 position_;
    glm::vec3 halfExtents_;
    glm::vec3 color_;
    float rotationYDegrees_;

    float moveSpeed_;
    float frightenedTimer_;
    float stateTimer_;

    std::shared_ptr<Model> model_;
    std::shared_ptr<Animation> runAnimation_;
    std::shared_ptr<Animation> crawlAnimation_;
    std::shared_ptr<Animation> attackAnimation_;
    std::shared_ptr<Animation> neckBiteAnimation_;
    std::unique_ptr<Animator> animator_;
    float attackTimer_ = 0.0f;
    float attackDuration_ = 0.0f;
    bool useNeckBite_ = false;
    bool isFrightened_ = false;
    float modelScale_ = 0.95f;
};
