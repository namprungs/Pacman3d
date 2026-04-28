#pragma once

#include "AnimationData.h"
#include "Mesh.h"

#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>

class Model
{
public:
    Model() = default;
    explicit Model(const std::string& path);

    void LoadModel(const std::string& path);
    void Draw(GLuint shaderProgram) const;
    bool IsLoaded() const;
    std::size_t GetMeshCount() const;

    std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap();
    int& GetBoneCount();

    static glm::mat4 ConvertMatrixToGlm(const aiMatrix4x4& from);

private:
    void ProcessNode(const aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(const aiMaterial* mat, aiTextureType type, const std::string& typeName);

    void SetVertexBoneDataToDefault(Vertex& vertex) const;
    void SetVertexBoneData(Vertex& vertex, int boneId, float weight) const;
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, const aiMesh* mesh, const aiScene* scene);

    std::vector<Texture> texturesLoaded_;
    std::vector<Mesh> meshes_;
    std::string directory_;

    std::unordered_map<std::string, BoneInfo> boneInfoMap_;
    int boneCounter_ = 0;
};
