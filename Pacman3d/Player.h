#pragma once
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>

class Map;
class Model;
class Animation;
class Animator;

class Player
{
public:
    Player();
    ~Player();

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    void update(GLFWwindow* window, float deltaTime, const Map& map);
    void updateAnimation(float deltaTime);
    void draw(GLuint shaderProgram) const;

    glm::vec3 getPosition() const;
    glm::ivec2 getGridPosition() const;
    glm::vec3 getForwardDirection() const;

    int getLives() const;
    void loseLife();
    void reset();

private:
    void setMovingState(bool moving);

    bool collidesWithWall(const glm::vec3& candidatePosition, const Map& map) const;
    bool intersectsAabb(const glm::vec3& minA, const glm::vec3& maxA, const glm::vec3& minB, const glm::vec3& maxB) const;

    glm::vec3 position_;
    glm::vec3 spawnPosition_;
    glm::vec3 halfExtents_;
    glm::vec3 forwardDirection_;
    float rotationYDegrees_;
    float moveSpeed_;
    float hitCooldown_;
    int lives_;

    std::unique_ptr<Model> model_;
    std::unique_ptr<Animation> idleAnimation_;
    std::unique_ptr<Animation> walkAnimation_;
    std::unique_ptr<Animator> animator_;
    bool isMoving_ = false;
    float modelScale_ = 0.90f;
};
