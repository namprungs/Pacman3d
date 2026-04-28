#include "Bone.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Bone::Bone(const std::string& name, int id, const aiNodeAnim* channel)
    : name_(name), id_(id)
{
    if (!channel)
    {
        return;
    }

    numPositions_ = static_cast<int>(channel->mNumPositionKeys);
    positions_.reserve(numPositions_);
    for (int i = 0; i < numPositions_; ++i)
    {
        const aiVector3D value = channel->mPositionKeys[i].mValue;
        const float timeStamp = static_cast<float>(channel->mPositionKeys[i].mTime);
        positions_.push_back({ glm::vec3(value.x, value.y, value.z), timeStamp });
    }

    numRotations_ = static_cast<int>(channel->mNumRotationKeys);
    rotations_.reserve(numRotations_);
    for (int i = 0; i < numRotations_; ++i)
    {
        const aiQuaternion value = channel->mRotationKeys[i].mValue;
        const float timeStamp = static_cast<float>(channel->mRotationKeys[i].mTime);
        rotations_.push_back({ glm::quat(value.w, value.x, value.y, value.z), timeStamp });
    }

    numScalings_ = static_cast<int>(channel->mNumScalingKeys);
    scales_.reserve(numScalings_);
    for (int i = 0; i < numScalings_; ++i)
    {
        const aiVector3D value = channel->mScalingKeys[i].mValue;
        const float timeStamp = static_cast<float>(channel->mScalingKeys[i].mTime);
        scales_.push_back({ glm::vec3(value.x, value.y, value.z), timeStamp });
    }
}

void Bone::Update(float animationTime)
{
    const glm::mat4 translation = InterpolatePosition(animationTime);
    const glm::mat4 rotation = InterpolateRotation(animationTime);
    const glm::mat4 scale = InterpolateScaling(animationTime);
    localTransform_ = translation * rotation * scale;
}

const glm::mat4& Bone::GetLocalTransform() const
{
    return localTransform_;
}

const std::string& Bone::GetBoneName() const
{
    return name_;
}

int Bone::GetBoneId() const
{
    return id_;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const
{
    const float scale = (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
    return glm::clamp(scale, 0.0f, 1.0f);
}

int Bone::GetPositionIndex(float animationTime) const
{
    for (int index = 0; index < numPositions_ - 1; ++index)
    {
        if (animationTime < positions_[index + 1].timeStamp)
        {
            return index;
        }
    }
    return numPositions_ - 1;
}

int Bone::GetRotationIndex(float animationTime) const
{
    for (int index = 0; index < numRotations_ - 1; ++index)
    {
        if (animationTime < rotations_[index + 1].timeStamp)
        {
            return index;
        }
    }
    return numRotations_ - 1;
}

int Bone::GetScaleIndex(float animationTime) const
{
    for (int index = 0; index < numScalings_ - 1; ++index)
    {
        if (animationTime < scales_[index + 1].timeStamp)
        {
            return index;
        }
    }
    return numScalings_ - 1;
}

glm::mat4 Bone::InterpolatePosition(float animationTime) const
{
    if (numPositions_ == 0)
    {
        return glm::mat4(1.0f);
    }
    if (numPositions_ == 1)
    {
        return glm::translate(glm::mat4(1.0f), positions_[0].position);
    }

    const int index = GetPositionIndex(animationTime);
    const int nextIndex = index + 1;
    if (nextIndex >= numPositions_)
    {
        return glm::translate(glm::mat4(1.0f), positions_[index].position);
    }
    const float scale = GetScaleFactor(positions_[index].timeStamp, positions_[nextIndex].timeStamp, animationTime);
    const glm::vec3 finalPosition = glm::mix(positions_[index].position, positions_[nextIndex].position, scale);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime) const
{
    if (numRotations_ == 0)
    {
        return glm::mat4(1.0f);
    }
    if (numRotations_ == 1)
    {
        const glm::quat rotation = glm::normalize(rotations_[0].orientation);
        return glm::toMat4(rotation);
    }

    const int index = GetRotationIndex(animationTime);
    const int nextIndex = index + 1;
    if (nextIndex >= numRotations_)
    {
        return glm::toMat4(glm::normalize(rotations_[index].orientation));
    }
    const float scale = GetScaleFactor(rotations_[index].timeStamp, rotations_[nextIndex].timeStamp, animationTime);
    const glm::quat finalRotation = glm::slerp(rotations_[index].orientation, rotations_[nextIndex].orientation, scale);
    return glm::toMat4(glm::normalize(finalRotation));
}

glm::mat4 Bone::InterpolateScaling(float animationTime) const
{
    if (numScalings_ == 0)
    {
        return glm::mat4(1.0f);
    }
    if (numScalings_ == 1)
    {
        return glm::scale(glm::mat4(1.0f), scales_[0].scale);
    }

    const int index = GetScaleIndex(animationTime);
    const int nextIndex = index + 1;
    if (nextIndex >= numScalings_)
    {
        return glm::scale(glm::mat4(1.0f), scales_[index].scale);
    }
    const float scale = GetScaleFactor(scales_[index].timeStamp, scales_[nextIndex].timeStamp, animationTime);
    const glm::vec3 finalScale = glm::mix(scales_[index].scale, scales_[nextIndex].scale, scale);
    return glm::scale(glm::mat4(1.0f), finalScale);
}
