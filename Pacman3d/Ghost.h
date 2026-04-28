#pragma once
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class Map;
class Player;

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
    void draw(GLuint shaderProgram) const;

    void triggerFrightened(float durationSeconds);
    bool checkCollisionWithPlayer(Player& player);
    void reset();

    glm::vec3 getPosition() const;
    GhostState getState() const;

private:
    void initCubeMesh();

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

    GLuint cubeVao_ = 0;
    GLuint cubeVbo_ = 0;
};