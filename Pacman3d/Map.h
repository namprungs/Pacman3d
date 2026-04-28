#pragma once
#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

class Map
{
public:
    Map();
    ~Map();

    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    void drawMap(GLuint shaderProgram) const;
    const std::vector<std::vector<int>>& getGrid() const;
    int consumeItemAt(int gridX, int gridZ);
    bool hasRemainingDots() const;
    bool spawnRandomFruit();
    bool spawnRandomItem();
    void reset();

private:
    void initCubeMesh();
    void initSphereMesh();

    std::vector<std::vector<int>> grid_;

    GLuint cubeVao_ = 0;
    GLuint cubeVbo_ = 0;

    GLuint sphereVao_ = 0;
    GLuint sphereVbo_ = 0;
    GLuint sphereEbo_ = 0;
    GLsizei sphereIndexCount_ = 0;
};
