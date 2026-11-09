// shaders/shader.vs

# version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in vec3 aInstanceOffset;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)) + aInstanceOffset;
    vec3 N = normalize(transpose(inverse(mat3(model))) * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(mat3(model) * aBitangent);

    TBN = mat3(T, B, N);

    Normal = N;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}