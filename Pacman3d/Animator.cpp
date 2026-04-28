#include "Animator.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

Animator::Animator(Animation* animation)
    : currentAnimation_(animation)
{
    finalBoneMatrices_.resize(kMaxBones, glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float deltaTime)
{
    if (!currentAnimation_)
    {
        return;
    }

    currentTime_ += currentAnimation_->GetTicksPerSecond() * deltaTime;
    const float duration = currentAnimation_->GetDuration();
    if (duration > 0.0f)
    {
        currentTime_ = std::fmod(currentTime_, duration);
    }

    CalculateBoneTransform(currentAnimation_->GetRootNode(), glm::mat4(1.0f));
}

void Animator::PlayAnimation(Animation* animation)
{
    currentAnimation_ = animation;
    currentTime_ = 0.0f;
}

const std::vector<glm::mat4>& Animator::GetFinalBoneMatrices() const
{
    return finalBoneMatrices_;
}

void Animator::CalculateBoneTransform(const AssimpNodeData& node, const glm::mat4& parentTransform)
{
    glm::mat4 nodeTransform = node.transformation;

    if (currentAnimation_)
    {
        Bone* bone = currentAnimation_->FindBone(node.name);
        if (bone)
        {
            bone->Update(currentTime_);
            nodeTransform = bone->GetLocalTransform();
            if (node.name == "mixamorig_Hips")
            {
                nodeTransform[3][0] = 0.0f;
                nodeTransform[3][2] = 0.0f;
            }
        }
    }

    const glm::mat4 globalTransformation = parentTransform * nodeTransform;

    if (currentAnimation_)
    {
        const auto& boneInfoMap = currentAnimation_->GetBoneIDMap();
        auto iter = boneInfoMap.find(node.name);
        if (iter != boneInfoMap.end())
        {
            const int index = iter->second.id;
            if (index >= 0 && index < kMaxBones)
            {
                finalBoneMatrices_[index] = globalTransformation * iter->second.offset;
            }
        }
    }

    for (int i = 0; i < node.childrenCount; ++i)
    {
        CalculateBoneTransform(node.children[i], globalTransformation);
    }
}
