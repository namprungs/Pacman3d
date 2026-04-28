#pragma once
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Map;

class Player
{
public:
    Player();
    ~Player();

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    void update(GLFWwindow* window, float deltaTime, const Map& map);
    void draw(GLuint shaderProgram) const;

    glm::vec3 getPosition() const;
    glm::ivec2 getGridPosition() const;
    glm::vec3 getForwardDirection() const;

    int getLives() const;
    void loseLife();
    void reset();

private:
    void initSphereMesh();
    void initCubeMesh();

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

    GLuint sphereVao_ = 0;
    GLuint sphereVbo_ = 0;
    GLuint sphereEbo_ = 0;
    GLsizei sphereIndexCount_ = 0;

    GLuint cubeVao_ = 0;
    GLuint cubeVbo_ = 0;
};