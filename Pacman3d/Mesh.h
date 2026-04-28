#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    int boneIds[4];
    float weights[4];
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures, const glm::vec3& diffuseColor = glm::vec3(1.0f));

    void Draw(GLuint shaderProgram) const;

private:
    void SetupMesh();

    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
    std::vector<Texture> textures_;
    glm::vec3 diffuseColor_;

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
};
