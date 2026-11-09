// shaders/screen.fs

# version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTex;
uniform int effectMode;

// 0: No Effect
// 1: Inverse
// 2: Grayscale
// 3: Sharpen Kernel
// 4: Blur Kernel

void main()
{
    if (effectMode == 0)
    {
        FragColor = texture(screenTex, TexCoords);
    }
    else if (effectMode == 1)
    {
        FragColor = vec4(vec3(1.0 - texture(screenTex, TexCoords)), 1.0);
    }
    else if (effectMode == 2)
    {
        FragColor = texture(screenTex, TexCoords);
        float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
        FragColor = vec4(average, average, average, 1.0);
    }
    else if (effectMode == 3)
    {
        const float offset = 1.0 / 300.0;

        vec2 offsets[9] = vec2[](
            vec2(-offset, offset), vec2(0.0f, offset), vec2(offset, offset),
            vec2(-offset, 0.0f), vec2(0.0f, 0.0f), vec2(offset, 0.0f),
            vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)
        );

        float kernel[9] = float[](
            -1, -1, -1,
            -1, 9, -1,
            -1, -1, -1
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTex, TexCoords.st + offsets[i]));
        }

        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
    else if (effectMode == 4)
    {
        const float offset = 1.0 / 300.0;

        vec2 offsets[9] = vec2[](
            vec2(-offset, offset), vec2(0.0f, offset), vec2(offset, offset),
            vec2(-offset, 0.0f), vec2(0.0f, 0.0f), vec2(offset, 0.0f),
            vec2(-offset, -offset), vec2(0.0f, -offset), vec2(offset, -offset)
        );

        float kernel[9] = float[](
            1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
            2.0 / 16.0,  4.0 / 16.0, 2.0 / 16.0,
            1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTex, TexCoords.st + offsets[i]));
        }

        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
}