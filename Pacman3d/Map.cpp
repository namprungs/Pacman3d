#include "Map.h"
#include "Map.h"

#include <cmath>
#include <random>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;
    constexpr int kSphereStacks = 10;
    constexpr int kSphereSectors = 14;

    std::vector<std::vector<int>> createInitialGrid()
    {
        return {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 2, 2, 2, 2, 0, 0, 3, 1},
            {1, 0, 1, 1, 0, 0, 1, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 1, 0, 1, 1, 0, 1, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 0, 0, 1, 1, 0, 1},
            {1, 3, 0, 0, 2, 2, 0, 0, 3, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    }
}

Map::Map()
    : grid_(createInitialGrid())
{
    initCubeMesh();
    initSphereMesh();
}

Map::~Map()
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

void Map::initCubeMesh()
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Map::initSphereMesh()
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

void Map::drawMap(GLuint shaderProgram) const
{
    const GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    const GLint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    for (std::size_t z = 0; z < grid_.size(); ++z)
    {
        for (std::size_t x = 0; x < grid_[z].size(); ++x)
        {
            const int cell = grid_[z][x];
            if (cell == 0)
            {
                continue;
            }

            glm::mat4 model(1.0f);
            model = glm::translate(model, glm::vec3(static_cast<float>(x), 0.0f, static_cast<float>(z)));

            if (cell == 1)
            {
                model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
                glUniform3f(colorLoc, 0.1f, 0.4f, 1.0f);

                glBindVertexArray(cubeVao_);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            else if (cell == 2 || cell == 3)
            {
                model = glm::translate(model, glm::vec3(0.0f, 0.25f, 0.0f));
                model = glm::scale(model, glm::vec3(cell == 3 ? 0.32f : 0.2f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
                glUniform3f(colorLoc, 1.0f, cell == 3 ? 0.3f : 0.9f, 0.2f);

                glBindVertexArray(sphereVao_);
                glDrawElements(GL_TRIANGLES, sphereIndexCount_, GL_UNSIGNED_INT, nullptr);
            }
            else if (cell == 4)
            {
                model = glm::translate(model, glm::vec3(0.0f, 0.3f, 0.0f));
                model = glm::scale(model, glm::vec3(0.3f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
                glUniform3f(colorLoc, 0.95f, 0.1f, 0.1f);

                glBindVertexArray(sphereVao_);
                glDrawElements(GL_TRIANGLES, sphereIndexCount_, GL_UNSIGNED_INT, nullptr);
            }
        }
    }

    glBindVertexArray(0);
}

const std::vector<std::vector<int>>& Map::getGrid() const
{
    return grid_;
}

int Map::consumeItemAt(int gridX, int gridZ)
{
    if (gridZ < 0 || gridZ >= static_cast<int>(grid_.size()))
    {
        return 0;
    }
    if (gridX < 0 || gridX >= static_cast<int>(grid_[gridZ].size()))
    {
        return 0;
    }

    const int value = grid_[gridZ][gridX];
    if (value != 2 && value != 3 && value != 4)
    {
        return 0;
    }

    grid_[gridZ][gridX] = 0;
    return value;
}

bool Map::hasRemainingDots() const
{
    for (const auto& row : grid_)
    {
        for (const int cell : row)
        {
            if (cell == 2 || cell == 3)
            {
                return true;
            }
        }
    }
    return false;
}

bool Map::spawnRandomFruit()
{
    std::vector<glm::ivec2> availableCells;
    for (std::size_t z = 0; z < grid_.size(); ++z)
    {
        for (std::size_t x = 0; x < grid_[z].size(); ++x)
        {
            if (grid_[z][x] == 0)
            {
                availableCells.emplace_back(static_cast<int>(x), static_cast<int>(z));
            }
        }
    }

    if (availableCells.empty())
    {
        return false;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, availableCells.size() - 1);
    const glm::ivec2 cell = availableCells[dist(rng)];
    grid_[cell.y][cell.x] = 4;
    return true;
}

void Map::reset()
{
    grid_ = createInitialGrid();
}
