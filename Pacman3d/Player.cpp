#include "Player.h"
#include "Player.h"

#include "Map.h"

#include <cmath>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;
    constexpr int kSphereStacks = 12;
    constexpr int kSphereSectors = 18;
}

Player::Player()
    : position_(1.0f, 0.35f, 1.0f),
      spawnPosition_(1.0f, 0.35f, 1.0f),
      halfExtents_(0.35f, 0.35f, 0.35f),
      forwardDirection_(0.0f, 0.0f, 1.0f),
      rotationYDegrees_(0.0f),
      moveSpeed_(3.5f),
      hitCooldown_(0.0f),
      lives_(3)
{
    initSphereMesh();
    initCubeMesh();
}

Player::~Player()
{
    if (cubeVbo_ != 0)
    {
        glDeleteBuffers(1, &cubeVbo_);
    }
    if (cubeVao_ != 0)
    {
        glDeleteVertexArrays(1, &cubeVao_);
    }

    if (sphereEbo_ != 0)
    {
        glDeleteBuffers(1, &sphereEbo_);
    }
    if (sphereVbo_ != 0)
    {
        glDeleteBuffers(1, &sphereVbo_);
    }
    if (sphereVao_ != 0)
    {
        glDeleteVertexArrays(1, &sphereVao_);
    }
}

void Player::initSphereMesh()
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= kSphereStacks; ++i)
    {
        const float stackAngle = kPi / 2.0f - i * kPi / kSphereStacks;
        const float xy = std::cos(stackAngle);
        const float y = std::sin(stackAngle);

        for (int j = 0; j <= kSphereSectors; ++j)
        {
            const float sectorAngle = j * 2.0f * kPi / kSphereSectors;
            const float x = xy * std::cos(sectorAngle);
            const float z = xy * std::sin(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for (int i = 0; i < kSphereStacks; ++i)
    {
        int k1 = i * (kSphereSectors + 1);
        int k2 = k1 + kSphereSectors + 1;

        for (int j = 0; j < kSphereSectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (kSphereStacks - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    sphereIndexCount_ = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &sphereVao_);
    glGenBuffers(1, &sphereVbo_);
    glGenBuffers(1, &sphereEbo_);

    glBindVertexArray(sphereVao_);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEbo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Player::initCubeMesh()
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

    if (glm::length(inputDirection) <= 0.0f)
    {
        return;
    }

    inputDirection = glm::normalize(inputDirection);

    const float movementStep = moveSpeed_ * deltaTime;

    const glm::vec3 moveX(inputDirection.x * movementStep, 0.0f, 0.0f);
    const glm::vec3 moveZ(0.0f, 0.0f, inputDirection.z * movementStep);

    const glm::vec3 candidateX = position_ + moveX;
    bool moved = false;
    if (!collidesWithWall(candidateX, map))
    {
        position_ = candidateX;
        moved = true;
    }

    const glm::vec3 candidateZ = position_ + moveZ;
    if (!collidesWithWall(candidateZ, map))
    {
        position_ = candidateZ;
        moved = true;
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

        forwardDirection_ = inputDirection;
        rotationYDegrees_ = glm::degrees(std::atan2(forwardDirection_.x, forwardDirection_.z));
    }
}

void Player::draw(GLuint shaderProgram) const
{
    const GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    const GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, position_);
    transform = glm::rotate(transform, glm::radians(rotationYDegrees_), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 bodyModel = glm::scale(transform, glm::vec3(halfExtents_ / 0.5f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &bodyModel[0][0]);
    glUniform3f(colorLoc, 1.0f, 0.9f, 0.1f);

    glBindVertexArray(sphereVao_);
    glDrawElements(GL_TRIANGLES, sphereIndexCount_, GL_UNSIGNED_INT, nullptr);

    glm::mat4 markerModel = glm::translate(transform, glm::vec3(0.0f, 0.08f, 0.36f));
    markerModel = glm::scale(markerModel, glm::vec3(0.14f, 0.10f, 0.18f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &markerModel[0][0]);
    glUniform3f(colorLoc, 0.9f, 0.25f, 0.1f);

    glBindVertexArray(cubeVao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
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

            const glm::vec3 wallMin(static_cast<float>(x) - 0.5f, 0.0f, static_cast<float>(z) - 0.5f);
            const glm::vec3 wallMax(static_cast<float>(x) + 0.5f, 1.0f, static_cast<float>(z) + 0.5f);

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
}
