#pragma once

#include "AnimationData.h"
#include "Animation.h"

#include <glm/glm.hpp>

#include <vector>

class Animator
{
public:
    explicit Animator(Animation* animation);

    void UpdateAnimation(float deltaTime);
    void PlayAnimation(Animation* animation);

    const std::vector<glm::mat4>& GetFinalBoneMatrices() const;

private:
    void CalculateBoneTransform(const AssimpNodeData& node, const glm::mat4& parentTransform);

    std::vector<glm::mat4> finalBoneMatrices_;
    Animation* currentAnimation_ = nullptr;
    float currentTime_ = 0.0f;
};
