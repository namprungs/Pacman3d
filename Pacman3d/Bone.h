#pragma once

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <vector>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
public:
    Bone(const std::string& name, int id, const aiNodeAnim* channel);

    void Update(float animationTime);

    const glm::mat4& GetLocalTransform() const;
    const std::string& GetBoneName() const;
    int GetBoneId() const;

private:
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const;

    int GetPositionIndex(float animationTime) const;
    int GetRotationIndex(float animationTime) const;
    int GetScaleIndex(float animationTime) const;

    glm::mat4 InterpolatePosition(float animationTime) const;
    glm::mat4 InterpolateRotation(float animationTime) const;
    glm::mat4 InterpolateScaling(float animationTime) const;

    std::vector<KeyPosition> positions_;
    std::vector<KeyRotation> rotations_;
    std::vector<KeyScale> scales_;

    int numPositions_ = 0;
    int numRotations_ = 0;
    int numScalings_ = 0;

    glm::mat4 localTransform_ = glm::mat4(1.0f);
    std::string name_;
    int id_ = -1;
};
