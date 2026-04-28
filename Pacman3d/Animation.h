#pragma once

#include "AnimationData.h"
#include "Bone.h"

#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

class Model;

struct AssimpNodeData
{
    glm::mat4 transformation = glm::mat4(1.0f);
    std::string name;
    int childrenCount = 0;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation(const std::string& animationPath, Model* model);

    Bone* FindBone(const std::string& name);

    float GetTicksPerSecond() const;
    float GetDuration() const;
    const AssimpNodeData& GetRootNode() const;
    const std::unordered_map<std::string, BoneInfo>& GetBoneIDMap() const;

private:
    void ReadMissingBones(const aiAnimation* animation, Model& model);
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

    float duration_ = 0.0f;
    float ticksPerSecond_ = 0.0f;
    std::vector<Bone> bones_;
    AssimpNodeData rootNode_;
    std::unordered_map<std::string, BoneInfo> boneInfoMap_;
};
