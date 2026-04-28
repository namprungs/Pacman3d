#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

constexpr int kMaxBones = 100;

struct BoneInfo
{
    int id = -1;
    glm::mat4 offset = glm::mat4(1.0f);
};
