#include "Map.h"

#include "Model.h"

#include <cmath>
#include <iostream>
#include <memory>
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
        std::vector<std::vector<int>> grid = {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 2, 2, 2, 2, 0, 0, 3, 1},
            {1, 0, 1, 1, 0, 0, 1, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 1, 0, 1, 1, 0, 1, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 1, 1, 0, 0, 1, 1, 0, 1},
            {1, 3, 0, 0, 2, 2, 4, 0, 3, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

        for (auto& row : grid)
        {
            for (int& cell : row)
            {
                if (cell == 0)
                {
                    cell = 2;
                }
            }
        }

        return grid;
    }
}

Map::Map()
    : grid_(createInitialGrid())
{
    initCubeMesh();
    initSphereMesh();
    coinModel_ = std::make_unique<Model>("resources/objects/items/Coin_obj/Coin.obj");
    powerItemModel_ = std::make_unique<Model>("resources/objects/items/Shine_Sprite/Shine_Sprite.obj");
    bonusItemModel_ = std::make_unique<Model>("resources/objects/items/Cute_Tomato_Buddy_OBJ/Cute_Tomato_Buddy_OBJ.obj");

    if (!coinModel_->IsLoaded())
    {
        std::cerr << "Failed to load coin model\n";
    }
    if (!powerItemModel_->IsLoaded())
    {
        std::cerr << "Failed to load power item model\n";
    }
    if (!bonusItemModel_->IsLoaded())
    {
        std::cerr << "Failed to load bonus item model\n";
    }
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
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEbo_);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
        indices.data(),
        GL_STATIC_DRAW);

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

            const float worldX = static_cast<float>(x);
            const float worldZ = static_cast<float>(z);

            // ----------------------------------------------------
            // 1) วาดพื้นทุกช่องก่อน
            // ทำให้ map ไม่ดูเป็นบล็อกลอย ๆ
            // ----------------------------------------------------
            {
                glm::mat4 floorModel(1.0f);
                floorModel = glm::translate(
                    floorModel,
                    glm::vec3(worldX, -0.04f, worldZ));

                // แผ่นพื้นบาง ๆ ขนาดเกือบเต็มช่อง
                floorModel = glm::scale(
                    floorModel,
                    glm::vec3(0.96f, 0.06f, 0.96f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &floorModel[0][0]);

                // สีพื้นเข้มแบบ arcade
                glUniform3f(colorLoc, 0.015f, 0.015f, 0.06f);

                glBindVertexArray(cubeVao_);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            // ช่องว่างมีแค่พื้น ไม่ต้องวาด object เพิ่ม
            if (cell == 0)
            {
                continue;
            }

            // ----------------------------------------------------
            // 2) วาดกำแพง
            // จากเดิมเป็น cube ใหญ่เต็มช่อง
            // เปลี่ยนเป็นกำแพงเตี้ยกว่า และเล็กกว่าช่องนิดหนึ่ง
            // ----------------------------------------------------
            if (cell == 1)
            {
                glm::mat4 wallModel(1.0f);

                wallModel = glm::translate(
                    wallModel,
                    glm::vec3(worldX, 0.22f, worldZ));

                // x/z = 0.88 ทำให้มีร่องระหว่างช่องเล็กน้อย
                // y = 0.50 ทำให้กำแพงไม่สูงเป็น block เกินไป
                wallModel = glm::scale(
                    wallModel,
                    glm::vec3(0.88f, 0.50f, 0.88f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &wallModel[0][0]);

                // สีฟ้าเข้ม ดูเป็น maze มากขึ้น
                glUniform3f(colorLoc, 0.03f, 0.18f, 0.95f);

                glBindVertexArray(cubeVao_);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // ------------------------------------------------
                // 2.1) วาด top highlight ด้านบนกำแพง
                // ทำให้กำแพงดูมีมิติ ไม่เป็น cube แบน ๆ
                // ------------------------------------------------
                glm::mat4 topModel(1.0f);

                topModel = glm::translate(
                    topModel,
                    glm::vec3(worldX, 0.49f, worldZ));

                topModel = glm::scale(
                    topModel,
                    glm::vec3(0.82f, 0.05f, 0.82f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &topModel[0][0]);

                // สีด้านบนสว่างกว่า
                glUniform3f(colorLoc, 0.12f, 0.45f, 1.0f);

                glBindVertexArray(cubeVao_);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            // ----------------------------------------------------
            // 3) coin และ power item
            // cell == 2 คือ coin จุดธรรมดา
            // cell == 3 คือ shine sprite ทำให้กินซอมบี้ได้
            // ----------------------------------------------------
            else if (cell == 2)
            {
                glm::mat4 coinModel(1.0f);

                coinModel = glm::translate(
                    coinModel,
                    glm::vec3(worldX, 0.12f, worldZ));

                coinModel = glm::scale(
                    coinModel,
                    glm::vec3(0.065f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &coinModel[0][0]);

                // สีทอง fallback ถ้า model โหลดไม่ได้
                glUniform3f(colorLoc, 0.98f, 0.62f, 0.05f);

                if (coinModel_ && coinModel_->IsLoaded())
                {
                    coinModel_->Draw(shaderProgram);
                }
                else
                {
                    glBindVertexArray(sphereVao_);
                    glDrawElements(GL_TRIANGLES, sphereIndexCount_, GL_UNSIGNED_INT, nullptr);
                }
            }

            else if (cell == 3)
            {
                glm::mat4 powerModel(1.0f);

                powerModel = glm::translate(
                    powerModel,
                    glm::vec3(worldX, 0.24f, worldZ));

                powerModel = glm::scale(
                    powerModel,
                    glm::vec3(0.20f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &powerModel[0][0]);
                glUniform3f(colorLoc, 0.95f, 0.75f, 0.15f);

                if (powerItemModel_ && powerItemModel_->IsLoaded())
                {
                    powerItemModel_->Draw(shaderProgram);
                }
                else
                {
                    glBindVertexArray(sphereVao_);
                    glDrawElements(GL_TRIANGLES, sphereIndexCount_, GL_UNSIGNED_INT, nullptr);
                }
            }

            // ----------------------------------------------------
            // 4) bonus item
            // cell == 4
            // ----------------------------------------------------
            else if (cell == 4)
            {
                glm::mat4 itemModel(1.0f);

                itemModel = glm::translate(
                    itemModel,
                    glm::vec3(worldX + 0.18f, 0.12f, worldZ - 0.45f));

                itemModel = glm::scale(
                    itemModel,
                    glm::vec3(0.018f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &itemModel[0][0]);

                // สีแดง fallback ถ้า model โหลดไม่ได้
                glUniform3f(colorLoc, 1.0f, 0.3f, 0.2f);

                if (bonusItemModel_ && bonusItemModel_->IsLoaded())
                {
                    bonusItemModel_->Draw(shaderProgram);
                }
                else
                {
                    glBindVertexArray(sphereVao_);
                    glDrawElements(GL_TRIANGLES, sphereIndexCount_, GL_UNSIGNED_INT, nullptr);
                }
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
            if (cell == 2)
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
                availableCells.emplace_back(
                    static_cast<int>(x),
                    static_cast<int>(z));
            }
        }
    }

    if (availableCells.empty())
    {
        return false;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> cellDist(0, availableCells.size() - 1);

    const glm::ivec2 cell = availableCells[cellDist(rng)];
    grid_[cell.y][cell.x] = 4;

    return true;
}

bool Map::spawnRandomItem()
{
    std::vector<glm::ivec2> availableCells;

    for (std::size_t z = 0; z < grid_.size(); ++z)
    {
        for (std::size_t x = 0; x < grid_[z].size(); ++x)
        {
            if (grid_[z][x] == 0)
            {
                availableCells.emplace_back(
                    static_cast<int>(x),
                    static_cast<int>(z));
            }
        }
    }

    if (availableCells.empty())
    {
        return false;
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> cellDist(0, availableCells.size() - 1);
    std::uniform_int_distribution<int> itemDist(0, 1);

    const glm::ivec2 cell = availableCells[cellDist(rng)];
    grid_[cell.y][cell.x] = itemDist(rng) == 0 ? 3 : 4;

    return true;
}

void Map::reset()
{
    grid_ = createInitialGrid();
}
