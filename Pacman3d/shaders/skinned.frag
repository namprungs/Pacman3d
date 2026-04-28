#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture;
uniform vec3 objectColor;
uniform bool useTexture;

void main()
{
    vec4 baseColor = useTexture ? texture(diffuseTexture, TexCoords) : vec4(objectColor, 1.0);
    FragColor = baseColor;
}
