#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;

void main()
{
    vec4 totalPosition = vec4(0.0);
    float totalWeight = 0.0;

    if (boneIds[0] == -1)
    {
        totalPosition = vec4(aPos, 1.0);
        totalWeight = 1.0;
    }
    else
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (boneIds[i] == -1)
            {
                continue;
            }
            if (boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(aPos, 1.0);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0);
            totalPosition += localPosition * weights[i];
            totalWeight += weights[i];
        }
    }

    if (totalWeight <= 0.0)
    {
        totalPosition = vec4(aPos, 1.0);
    }

    TexCoords = aTexCoords;
    gl_Position = projection * view * model * totalPosition;
}
