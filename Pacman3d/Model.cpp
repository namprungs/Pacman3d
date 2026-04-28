#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
    std::string ResolveAssetPath(const std::string& path)
    {
        namespace fs = std::filesystem;

        const std::vector<fs::path> candidates = {
#ifdef PACMAN3D_SOURCE_DIR
            fs::path(PACMAN3D_SOURCE_DIR) / path,
#endif
            fs::path(path),
            fs::path("Pacman3d") / path,
            fs::path("..") / "Pacman3d" / path,
            fs::path("..") / ".." / "Pacman3d" / path
        };

        for (const auto& candidate : candidates)
        {
            if (fs::exists(candidate))
            {
                return candidate.string();
            }
        }

        return path;
    }

    unsigned int TextureFromFile(const char* path, const std::string& directory)
    {
        std::string filename = directory + "/" + path;

        unsigned int textureId = 0;
        glGenTextures(1, &textureId);

        int width = 0;
        int height = 0;
        int components = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &components, 0);
        if (!data)
        {
            std::cerr << "Failed to load texture: " << filename << "\n";
            return 0;
        }

        GLenum format = GL_RGB;
        if (components == 1)
        {
            format = GL_RED;
        }
        else if (components == 3)
        {
            format = GL_RGB;
        }
        else if (components == 4)
        {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return textureId;
    }
}

Model::Model(const std::string& path)
{
    LoadModel(path);
}

void Model::LoadModel(const std::string& path)
{
    meshes_.clear();
    texturesLoaded_.clear();
    boneInfoMap_.clear();
    boneCounter_ = 0;

    const std::string resolvedPath = ResolveAssetPath(path);

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(resolvedPath,
                                             aiProcess_Triangulate |
                                                 aiProcess_GenSmoothNormals |
                                                 aiProcess_FlipUVs |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_LimitBoneWeights);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Assimp load error: " << importer.GetErrorString() << "\n";
        return;
    }

    const std::size_t slash = resolvedPath.find_last_of("/\\");
    directory_ = (slash == std::string::npos) ? std::string(".") : resolvedPath.substr(0, slash);
    ProcessNode(scene->mRootNode, scene);
}

void Model::Draw(GLuint shaderProgram) const
{
    for (const auto& mesh : meshes_)
    {
        mesh.Draw(shaderProgram);
    }
}

bool Model::IsLoaded() const
{
    return !meshes_.empty();
}

std::size_t Model::GetMeshCount() const
{
    return meshes_.size();
}

std::unordered_map<std::string, BoneInfo>& Model::GetBoneInfoMap()
{
    return boneInfoMap_;
}

int& Model::GetBoneCount()
{
    return boneCounter_;
}

glm::mat4 Model::ConvertMatrixToGlm(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

void Model::ProcessNode(const aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(ProcessMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh Model::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex{};
        SetVertexBoneDataToDefault(vertex);

        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = mesh->mNormals ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                                        : glm::vec3(0.0f, 1.0f, 0.0f);
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0)
    {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    }

    ExtractBoneWeightForVertices(vertices, mesh, scene);

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(const aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<Texture> textures;
    const unsigned int count = mat->GetTextureCount(type);

    for (unsigned int i = 0; i < count; ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (const auto& loaded : texturesLoaded_)
        {
            if (loaded.path == str.C_Str())
            {
                textures.push_back(loaded);
                skip = true;
                break;
            }
        }
        if (skip)
        {
            continue;
        }

        Texture texture{};
        texture.id = TextureFromFile(str.C_Str(), directory_);
        texture.type = typeName;
        texture.path = str.C_Str();
        textures.push_back(texture);
        texturesLoaded_.push_back(texture);
    }

    return textures;
}

void Model::SetVertexBoneDataToDefault(Vertex& vertex) const
{
    for (int i = 0; i < 4; ++i)
    {
        vertex.boneIds[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}

void Model::SetVertexBoneData(Vertex& vertex, int boneId, float weight) const
{
    for (int i = 0; i < 4; ++i)
    {
        if (vertex.boneIds[i] < 0)
        {
            vertex.boneIds[i] = boneId;
            vertex.weights[i] = weight;
            return;
        }
    }
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, const aiMesh* mesh, const aiScene* scene)
{
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        const aiBone* bone = mesh->mBones[boneIndex];
        const std::string boneName(bone->mName.C_Str());

        int boneId = -1;
        auto iter = boneInfoMap_.find(boneName);
        if (iter == boneInfoMap_.end())
        {
            BoneInfo info;
            info.id = boneCounter_;
            info.offset = ConvertMatrixToGlm(bone->mOffsetMatrix);
            boneInfoMap_[boneName] = info;
            boneId = boneCounter_;
            ++boneCounter_;
        }
        else
        {
            boneId = iter->second.id;
        }

        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
        {
            const aiVertexWeight weight = bone->mWeights[weightIndex];
            if (weight.mVertexId >= vertices.size())
            {
                continue;
            }
            SetVertexBoneData(vertices[weight.mVertexId], boneId, weight.mWeight);
        }
    }
}
