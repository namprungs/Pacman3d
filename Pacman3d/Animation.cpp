#include "Animation.h"

#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

Animation::Animation(const std::string& animationPath, Model* model)
{
    if (!model)
    {
        return;
    }

    namespace fs = std::filesystem;
    std::string resolvedPath = animationPath;
    const std::vector<fs::path> candidates = {
#ifdef PACMAN3D_SOURCE_DIR
        fs::path(PACMAN3D_SOURCE_DIR) / animationPath,
#endif
        fs::path(animationPath),
        fs::path("Pacman3d") / animationPath,
        fs::path("..") / "Pacman3d" / animationPath,
        fs::path("..") / ".." / "Pacman3d" / animationPath
    };

    for (const auto& candidate : candidates)
    {
        if (fs::exists(candidate))
        {
            resolvedPath = candidate.string();
            break;
        }
    }
	std::cout << "Loading animation from: " << resolvedPath << "\n";

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(resolvedPath,
                                             aiProcess_Triangulate |
                                                 aiProcess_FlipUVs |
                                                 aiProcess_LimitBoneWeights);
    if (!scene || !scene->mRootNode || scene->mNumAnimations == 0)
    {
        std::cerr << "Failed to load animation: " << resolvedPath
                  << " (" << importer.GetErrorString() << ")\n";
        return;
    }
    std::cout << "Successfully loaded animation: " << resolvedPath << " (" << scene->mAnimations[0]->mName.C_Str() << ")" << std::endl;
    const aiAnimation* animation = scene->mAnimations[0];
    duration_ = static_cast<float>(animation->mDuration);
    ticksPerSecond_ = static_cast<float>(animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f);

    ReadHierarchyData(rootNode_, scene->mRootNode);
    ReadMissingBones(animation, *model);
}

Bone* Animation::FindBone(const std::string& name)
{
    for (auto& bone : bones_)
    {
        if (bone.GetBoneName() == name)
        {
            return &bone;
        }
    }
    return nullptr;
}

float Animation::GetTicksPerSecond() const
{
    return ticksPerSecond_;
}

float Animation::GetDuration() const
{
    return duration_;
}

const AssimpNodeData& Animation::GetRootNode() const
{
    return rootNode_;
}

const std::unordered_map<std::string, BoneInfo>& Animation::GetBoneIDMap() const
{
    return boneInfoMap_;
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
    int& boneCount = model.GetBoneCount();
    std::unordered_map<std::string, BoneInfo>& boneInfoMap = model.GetBoneInfoMap();

    for (unsigned int i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim* channel = animation->mChannels[i];
        const std::string boneName(channel->mNodeName.C_Str());

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].id = boneCount;
            boneInfoMap[boneName].offset = glm::mat4(1.0f);
            ++boneCount;
        }

        bones_.emplace_back(boneName, boneInfoMap[boneName].id, channel);
    }

    boneInfoMap_ = boneInfoMap;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    dest.name = src->mName.C_Str();
    dest.transformation = Model::ConvertMatrixToGlm(src->mTransformation);
    dest.childrenCount = static_cast<int>(src->mNumChildren);
    dest.children.resize(dest.childrenCount);

    for (int i = 0; i < dest.childrenCount; ++i)
    {
        ReadHierarchyData(dest.children[i], src->mChildren[i]);
    }
}
