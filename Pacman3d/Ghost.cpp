#include "Ghost.h"
#include "Ghost.h"

#include "Map.h"
#include "Player.h"

#include <cmath>
#include <limits>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

Ghost::Ghost(GhostType type, const glm::ivec2& startGridPosition, const glm::ivec2& scatterCorner, const glm::vec3& color)
    : type_(type),
      state_(GhostState::SCATTER),
      currentGridPosition_(startGridPosition),
      targetGridPosition_(startGridPosition),
      previousGridPosition_(startGridPosition),
      scatterCorner_(scatterCorner),
      spawnGridPosition_(startGridPosition),
      position_(static_cast<float>(startGridPosition.x), 0.35f, static_cast<float>(startGridPosition.y)),
      halfExtents_(0.30f, 0.30f, 0.30f),
      color_(color),
      rotationYDegrees_(0.0f),
      moveSpeed_(2.4f),
      frightenedTimer_(0.0f),
      stateTimer_(0.0f)
{
    initCubeMesh();
}

Ghost::~Ghost()
{
    if (cubeVbo_ != 0)
    {
        glDeleteBuffers(1, &cubeVbo_);
    }
    if (cubeVao_ != 0)
    {
        glDeleteVertexArrays(1, &cubeVao_);
    }
}

Ghost::Ghost(Ghost&& other) noexcept
    : type_(other.type_),
      state_(other.state_),
      currentGridPosition_(other.currentGridPosition_),
      targetGridPosition_(other.targetGridPosition_),
      previousGridPosition_(other.previousGridPosition_),
      scatterCorner_(other.scatterCorner_),
      spawnGridPosition_(other.spawnGridPosition_),
      position_(other.position_),
      halfExtents_(other.halfExtents_),
      color_(other.color_),
      rotationYDegrees_(other.rotationYDegrees_),
      moveSpeed_(other.moveSpeed_),
      frightenedTimer_(other.frightenedTimer_),
      stateTimer_(other.stateTimer_),
      cubeVao_(other.cubeVao_),
      cubeVbo_(other.cubeVbo_)
{
    other.cubeVao_ = 0;
    other.cubeVbo_ = 0;
}

Ghost& Ghost::operator=(Ghost&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (cubeVbo_ != 0)
    {
        glDeleteBuffers(1, &cubeVbo_);
    }
    if (cubeVao_ != 0)
    {
        glDeleteVertexArrays(1, &cubeVao_);
    }

    type_ = other.type_;
    state_ = other.state_;
    currentGridPosition_ = other.currentGridPosition_;
    targetGridPosition_ = other.targetGridPosition_;
    previousGridPosition_ = other.previousGridPosition_;
    scatterCorner_ = other.scatterCorner_;
    spawnGridPosition_ = other.spawnGridPosition_;
    position_ = other.position_;
    halfExtents_ = other.halfExtents_;
    color_ = other.color_;
    rotationYDegrees_ = other.rotationYDegrees_;
    moveSpeed_ = other.moveSpeed_;
    frightenedTimer_ = other.frightenedTimer_;
    stateTimer_ = other.stateTimer_;
    cubeVao_ = other.cubeVao_;
    cubeVbo_ = other.cubeVbo_;

    other.cubeVao_ = 0;
    other.cubeVbo_ = 0;

    return *this;
}

void Ghost::initCubeMesh()
{
    const float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
        0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

        -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

        0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
        0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
        0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};

    glGenVertexArrays(1, &cubeVao_);
    glGenBuffers(1, &cubeVbo_);

    glBindVertexArray(cubeVao_);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Ghost::update(float deltaTime, const Map& map, const Player& player)
{
    if (frightenedTimer_ > 0.0f)
    {
        frightenedTimer_ -= deltaTime;
        state_ = frightenedTimer_ > 0.0f ? GhostState::FRIGHTENED : GhostState::CHASE;
    }
    else
    {
        stateTimer_ += deltaTime;
        if (state_ == GhostState::CHASE && stateTimer_ >= 7.0f)
        {
            state_ = GhostState::SCATTER;
            stateTimer_ = 0.0f;
        }
        else if (state_ == GhostState::SCATTER && stateTimer_ >= 5.0f)
        {
            state_ = GhostState::CHASE;
            stateTimer_ = 0.0f;
        }
        //state_ = GhostState::CHASE;
    }

    const glm::vec3 targetWorldPosition(static_cast<float>(targetGridPosition_.x), 0.35f, static_cast<float>(targetGridPosition_.y));
    glm::vec3 toTarget = targetWorldPosition - position_;
    const float distanceToTarget = glm::length(toTarget);

    const float speedMultiplier = (state_ == GhostState::FRIGHTENED) ? 0.7f : 1.0f;
    const float step = moveSpeed_ * speedMultiplier * deltaTime;

    if (distanceToTarget <= step || distanceToTarget < 0.001f)
    {
        position_ = targetWorldPosition;
        previousGridPosition_ = currentGridPosition_;
        currentGridPosition_ = targetGridPosition_;
        targetGridPosition_ = chooseNextCell(map, player);
    }
    else
    {
        toTarget = glm::normalize(toTarget);
        position_ += toTarget * step;
    }

    const glm::vec3 movementDirection = glm::vec3(static_cast<float>(targetGridPosition_.x), 0.35f, static_cast<float>(targetGridPosition_.y)) - position_;
    if (glm::length(movementDirection) > 0.0001f)
    {
        const glm::vec3 dir = glm::normalize(movementDirection);
        rotationYDegrees_ = glm::degrees(std::atan2(dir.x, dir.z));
    }
}

void Ghost::draw(GLuint shaderProgram) const
{
    const GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    const GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    glm::mat4 model(1.0f);
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(rotationYDegrees_), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(halfExtents_ / 0.5f));

    glm::vec3 drawColor = color_;
    if (state_ == GhostState::FRIGHTENED)
    {
        drawColor = glm::vec3(0.2f, 0.2f, 1.0f);
    }

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniform3f(colorLoc, drawColor.r, drawColor.g, drawColor.b);

    glBindVertexArray(cubeVao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Ghost::triggerFrightened(float durationSeconds)
{
    frightenedTimer_ = durationSeconds;
    state_ = GhostState::FRIGHTENED;
}

bool Ghost::checkCollisionWithPlayer(Player& player)
{
    const glm::vec3 playerHalfExtents(0.35f, 0.35f, 0.35f);
    const glm::vec3 playerMin = player.getPosition() - playerHalfExtents;
    const glm::vec3 playerMax = player.getPosition() + playerHalfExtents;

    const glm::vec3 ghostMin = position_ - halfExtents_;
    const glm::vec3 ghostMax = position_ + halfExtents_;

    if (!intersectsAabb(playerMin, playerMax, ghostMin, ghostMax))
    {
        return false;
    }

    if (state_ == GhostState::FRIGHTENED)
    {
        currentGridPosition_ = spawnGridPosition_;
        targetGridPosition_ = spawnGridPosition_;
        previousGridPosition_ = spawnGridPosition_;
        position_ = glm::vec3(static_cast<float>(spawnGridPosition_.x), 0.35f, static_cast<float>(spawnGridPosition_.y));
        frightenedTimer_ = 0.0f;
        state_ = GhostState::SCATTER;
        return true;
    }

    player.loseLife();
    return true;
}

void Ghost::reset()
{
    state_ = GhostState::SCATTER;
    currentGridPosition_ = spawnGridPosition_;
    targetGridPosition_ = spawnGridPosition_;
    previousGridPosition_ = spawnGridPosition_;
    position_ = glm::vec3(static_cast<float>(spawnGridPosition_.x), 0.35f, static_cast<float>(spawnGridPosition_.y));
    rotationYDegrees_ = 0.0f;
    frightenedTimer_ = 0.0f;
    stateTimer_ = 0.0f;
}

glm::vec3 Ghost::getPosition() const
{
    return position_;
}

GhostState Ghost::getState() const
{
    return state_;
}

glm::ivec2 Ghost::chooseNextCell(const Map& map, const Player& player) const
{
    static const glm::ivec2 directions[] = {
        glm::ivec2(0, -1),
        glm::ivec2(1, 0),
        glm::ivec2(0, 1),
        glm::ivec2(-1, 0)};

    std::vector<glm::ivec2> candidates;
    for (const glm::ivec2& dir : directions)
    {
        const glm::ivec2 next = currentGridPosition_ + dir;
        if (!isWalkableCell(next, map))
        {
            continue;
        }
        if (next == previousGridPosition_)
        {
            continue;
        }
        candidates.push_back(next);
    }

    if (candidates.empty())
    {
        for (const glm::ivec2& dir : directions)
        {
            const glm::ivec2 next = currentGridPosition_ + dir;
            if (isWalkableCell(next, map))
            {
                candidates.push_back(next);
            }
        }
    }

    if (candidates.empty())
    {
        return currentGridPosition_;
    }

    const glm::ivec2 playerCell = player.getGridPosition();
    glm::ivec2 targetCell = scatterCorner_;

    if (state_ == GhostState::CHASE)
    {
        targetCell = getChaseTarget(player);
    }
    else if (state_ == GhostState::FRIGHTENED)
    {
        float farthestDistance = -1.0f;
        glm::ivec2 best = candidates.front();
        for (const glm::ivec2& candidate : candidates)
        {
            const glm::vec2 delta = glm::vec2(candidate - playerCell);
            const float dist = glm::length(delta);
            if (dist > farthestDistance)
            {
                farthestDistance = dist;
                best = candidate;
            }
        }
        return best;
    }

    float bestDistance = std::numeric_limits<float>::max();
    glm::ivec2 best = candidates.front();
    for (const glm::ivec2& candidate : candidates)
    {
        const glm::vec2 delta = glm::vec2(candidate - targetCell);
        const float dist = glm::length(delta);
        if (dist < bestDistance)
        {
            bestDistance = dist;
            best = candidate;
        }
    }

    return best;
}

glm::ivec2 Ghost::getChaseTarget(const Player& player) const
{
    const glm::ivec2 playerCell = player.getGridPosition();
    const glm::vec3 forward = player.getForwardDirection();
    const glm::ivec2 lookAhead(static_cast<int>(std::lround(forward.x * 2.0f)), static_cast<int>(std::lround(forward.z * 2.0f)));

    switch (type_)
    {
    case GhostType::BLINKY:
        return playerCell;
    case GhostType::PINKY:
        return playerCell + lookAhead;
    case GhostType::INKY:
        return playerCell + lookAhead + glm::ivec2(2, -2);
    case GhostType::CLYDE:
    {
        const glm::vec2 delta = glm::vec2(currentGridPosition_ - playerCell);
        if (glm::length(delta) < 3.0f)
        {
            return scatterCorner_;
        }
        return playerCell;
    }
    }

    return playerCell;
}

bool Ghost::isWalkableCell(const glm::ivec2& gridCell, const Map& map) const
{
    const auto& grid = map.getGrid();
    if (gridCell.y < 0 || gridCell.y >= static_cast<int>(grid.size()))
    {
        return false;
    }
    if (gridCell.x < 0 || gridCell.x >= static_cast<int>(grid[gridCell.y].size()))
    {
        return false;
    }
    return grid[gridCell.y][gridCell.x] != 1;
}

bool Ghost::intersectsAabb(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB) const
{
    return minA.x <= maxB.x &&
           maxA.x >= minB.x &&
           minA.y <= maxB.y &&
           maxA.y >= minB.y &&
           minA.z <= maxB.z &&
           maxA.z >= minB.z;
}
