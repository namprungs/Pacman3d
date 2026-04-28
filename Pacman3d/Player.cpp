#include "Player.h"

#include "Animation.h"
#include "AnimationData.h"
#include "Animator.h"
#include "Map.h"
#include "Model.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    constexpr float kPlayerWallHalfExtent = 0.35f;
    constexpr float kWallCollisionHalfExtent = 0.44f;
}

Player::Player()
    : position_(1.0f, 0.35f, 1.0f),
      spawnPosition_(1.0f, 0.35f, 1.0f),
      halfExtents_(kPlayerWallHalfExtent, 0.35f, kPlayerWallHalfExtent),
      forwardDirection_(0.0f, 0.0f, 1.0f),
      rotationYDegrees_(0.0f),
      moveSpeed_(3.4f),
      hitCooldown_(0.0f),
      lives_(3)
{
    model_ = std::make_unique<Model>("resources/objects/player/Idle.dae");
    if (model_ && model_->IsLoaded())
    {
        std::cout << "Loaded model: resources/objects/player/Idle.dae (" << model_->GetMeshCount() << " meshes)\n";
    }
    else
    {
        std::cout << "Failed to load model: resources/objects/player/Idle.dae\n";
    }
    idleAnimation_ = std::make_unique<Animation>("resources/objects/player/Idle.dae", model_.get());
    walkAnimation_ = std::make_unique<Animation>("resources/objects/player/Running.dae", model_.get());
    animator_ = std::make_unique<Animator>(idleAnimation_.get());
}

Player::~Player() = default;

void Player::update(GLFWwindow* window, float deltaTime, const Map& map)
{
    if (hitCooldown_ > 0.0f)
    {
        hitCooldown_ -= deltaTime;
    }

    glm::vec3 inputDirection(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        inputDirection.z -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        inputDirection.z += 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        inputDirection.x -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        inputDirection.x += 1.0f;
    }

    bool moved = false;
    const bool hasMovementInput = glm::length(inputDirection) > 0.0f;
    if (hasMovementInput)
    {
        inputDirection = glm::normalize(inputDirection);

        const float movementStep = moveSpeed_ * std::min(deltaTime, 0.05f);
        const int subSteps = std::max(1, static_cast<int>(std::ceil(movementStep / 0.03f)));
        const float stepSize = movementStep / static_cast<float>(subSteps);

        for (int i = 0; i < subSteps; ++i)
        {
            const glm::vec3 moveX(inputDirection.x * stepSize, 0.0f, 0.0f);
            const glm::vec3 moveZ(0.0f, 0.0f, inputDirection.z * stepSize);

            if (moveX.x != 0.0f)
            {
                const glm::vec3 candidateX = position_ + moveX;
                if (!collidesWithWall(candidateX, map))
                {
                    position_ = candidateX;
                    moved = true;
                }
            }

            if (moveZ.z != 0.0f)
            {
                const glm::vec3 candidateZ = position_ + moveZ;
                if (!collidesWithWall(candidateZ, map))
                {
                    position_ = candidateZ;
                    moved = true;
                }
            }
        }

        if (moved)
        {
            const auto& grid = map.getGrid();
            if (!grid.empty())
            {
                const float rightEdge = static_cast<float>(grid[0].size() - 1);
                if (position_.x < -0.5f)
                {
                    position_.x = rightEdge;
                }
                else if (position_.x > rightEdge + 0.5f)
                {
                    position_.x = 0.0f;
                }
            }
        }

        forwardDirection_ = inputDirection;
        rotationYDegrees_ = glm::degrees(std::atan2(forwardDirection_.x, forwardDirection_.z));
    }

    setMovingState(hasMovementInput);
}

void Player::updateAnimation(float deltaTime)
{
    if (animator_)
    {
        animator_->UpdateAnimation(deltaTime * 0.75f);
    }
}

void Player::draw(GLuint shaderProgram) const
{
    const GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    const GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    glm::mat4 model(1.0f);
    model = glm::translate(model, position_);
    model = glm::rotate(model, glm::radians(rotationYDegrees_), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(modelScale_));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

    if (animator_)
    {
        const auto& transforms = animator_->GetFinalBoneMatrices();
        for (std::size_t i = 0; i < transforms.size(); ++i)
        {
            std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &transforms[i][0][0]);
        }
    }

    if (model_)
    {
        model_->Draw(shaderProgram);
    }
}

void Player::setMovingState(bool moving)
{
    if (moving == isMoving_)
    {
        return;
    }

    isMoving_ = moving;
    if (!animator_)
    {
        return;
    }

    if (isMoving_ && walkAnimation_)
    {
        animator_->PlayAnimation(walkAnimation_.get());
    }
    else
    {
        animator_->PlayAnimation(idleAnimation_.get());
    }
}

bool Player::collidesWithWall(const glm::vec3& candidatePosition, const Map& map) const
{
    const glm::vec3 playerMin = candidatePosition - halfExtents_;
    const glm::vec3 playerMax = candidatePosition + halfExtents_;

    const auto& grid = map.getGrid();
    for (std::size_t z = 0; z < grid.size(); ++z)
    {
        for (std::size_t x = 0; x < grid[z].size(); ++x)
        {
            if (grid[z][x] != 1)
            {
                continue;
            }

            const glm::vec3 wallMin(static_cast<float>(x) - kWallCollisionHalfExtent, 0.0f, static_cast<float>(z) - kWallCollisionHalfExtent);
            const glm::vec3 wallMax(static_cast<float>(x) + kWallCollisionHalfExtent, 1.0f, static_cast<float>(z) + kWallCollisionHalfExtent);

            if (intersectsAabb(playerMin, playerMax, wallMin, wallMax))
            {
                return true;
            }
        }
    }

    return false;
}

bool Player::intersectsAabb(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB) const
{
    return minA.x <= maxB.x &&
           maxA.x >= minB.x &&
           minA.y <= maxB.y &&
           maxA.y >= minB.y &&
           minA.z <= maxB.z &&
           maxA.z >= minB.z;
}

glm::vec3 Player::getPosition() const
{
    return position_;
}

glm::ivec2 Player::getGridPosition() const
{
    return glm::ivec2(static_cast<int>(std::lround(position_.x)), static_cast<int>(std::lround(position_.z)));
}

glm::vec3 Player::getForwardDirection() const
{
    return forwardDirection_;
}

int Player::getLives() const
{
    return lives_;
}

void Player::loseLife()
{
    if (lives_ <= 0 || hitCooldown_ > 0.0f)
    {
        return;
    }

    --lives_;
    hitCooldown_ = 1.0f;
}

void Player::reset()
{
    position_ = spawnPosition_;
    forwardDirection_ = glm::vec3(0.0f, 0.0f, 1.0f);
    rotationYDegrees_ = 0.0f;
    hitCooldown_ = 0.0f;
    lives_ = 3;
    setMovingState(false);
}
