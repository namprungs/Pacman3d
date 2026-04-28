#include "Mesh.h"

#include <cstddef>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures, const glm::vec3& diffuseColor)
    : vertices_(vertices), indices_(indices), textures_(textures), diffuseColor_(diffuseColor)
{
    SetupMesh();
}

void Mesh::Draw(GLuint shaderProgram) const
{
    bool hasDiffuse = false;
    for (const Texture& texture : textures_)
    {
        if (texture.type == "texture_diffuse" && texture.id != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture.id);
            glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 0);
            hasDiffuse = true;
            break;
        }
    }

    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), hasDiffuse ? 1 : 0);
    if (!hasDiffuse)
    {
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), diffuseColor_.r, diffuseColor_.g, diffuseColor_.b);
    }

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::SetupMesh()
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices_.size() * sizeof(Vertex)), vertices_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices_.size() * sizeof(unsigned int)), indices_.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIds));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

    glBindVertexArray(0);
}
