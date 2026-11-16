// shaders/shader.fs

# version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

struct Material {
    vec3 ambient;
    sampler2D diffuse;
    sampler2D specularMap;
    sampler2D normalMap;
    sampler2D displacementMap;
    sampler2D ambientMap;
    float shininess;
    float reflectivity;
    float refractiveIndex;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

# define NR_POINT_LIGHTS 4

uniform vec3 viewPos;

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform samplerCube skybox;

uniform usampler3D voxelGrid;
uniform ivec3 voxelGridOrigin;
uniform int voxelGridSize;
uniform sampler2DArray blockTextures;

uniform bool drawSingleColor;
uniform vec3 singleColor;

uniform float heightScale;

const bool DISPLACEMENT_IS_HEIGHT = true;
const bool DISCARD_OUTSIDE_UV = false;

vec3 aoTex = vec3(1.0);

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffTex, vec3 specTex);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 FragPos, vec3 viewDir, vec3 diffTex, vec3 specTex);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 FragPos, vec3 viewDir, vec3 diffTex, vec3 specTex);

float SampleDepth(vec2 uv)
{
    float h = textureLod(material.displacementMap, uv, 0.0).r;
    if (DISPLACEMENT_IS_HEIGHT)
        return 1.0 - h;
    else
        return h;
}

vec2 ParallaxMapping(vec2 uv, vec3 viewDirTS)
{
    if (heightScale <= 0.0)
        return uv;

    const float minLayers = 8.0, maxLayers = 64.0;
    float layerDepth = 1.0 / mix(maxLayers, minLayers, abs(viewDirTS.z));

    vec2 deltaUV = (viewDirTS.xy / max(viewDirTS.z, 0.2)) * heightScale / mix(maxLayers, minLayers, abs(viewDirTS.z));

    vec2 currentUV = uv;
    float currentDepth = 0.0, mapDepth = SampleDepth(currentUV);

    for (int i = 0; i < 64 && currentDepth < mapDepth; i++)
    {
        currentUV -= deltaUV;
        mapDepth = SampleDepth(currentUV);
        currentDepth += layerDepth;
    }

    vec2 prevUV = currentUV + deltaUV; // just imagine the seconds saved if we could use the greek keyboard and just use the delta symbol instead of "delta" every time
    float afterDepth = mapDepth - currentDepth, beforeDepth = SampleDepth(prevUV) - currentDepth + layerDepth;
    float weight = (abs(afterDepth - beforeDepth) > 0.0001) ? clamp(afterDepth / (afterDepth - beforeDepth), 0.0, 1.0) : 0.0;

    return prevUV * weight + currentUV * (1.0 - weight);
}

vec3 TraceVoxelRay(vec3 worldOrigin, vec3 rayDir, bool startsIn)
{
    vec3 gridPos = (worldOrigin + vec3(0.5)) - vec3(voxelGridOrigin);
    ivec3 voxel = ivec3(floor(gridPos));

    ivec3 stepDir = ivec3(sign(rayDir));
    vec3 tDelta = 1.0 / max(abs(rayDir), vec3(1e-5)); // Epsilon Safety Guard

    vec3 nextBoundary = vec3(voxel) + max(vec3(stepDir), vec3(0.0));
    vec3 tMax = abs(nextBoundary - gridPos) * tDelta;

    vec3 hitNormal = vec3(0.0, 1.0, 0.0);
    float tHit = 0.0;
    const int MAX_STEPS = 128;

    for (int i = 0; i < MAX_STEPS; i++)
    {
        if (any(lessThan(voxel, ivec3(0))) || any(greaterThanEqual(voxel, ivec3(voxelGridSize))))
            break;

        uint blockID = texelFetch(voxelGrid, voxel, 0).r;
        bool ignoreCell = (startsIn && i == 0);
        if (blockID != 0u && !ignoreCell)
        {
            vec3 hitPos = gridPos + rayDir * tHit;
            vec2 hitUV;

            if (abs(hitNormal.x) > 0.5)
                hitUV = fract(hitPos.zy);
            else if (abs(hitNormal.y) > 0.5)
                hitUV = fract(hitPos.xz);
            else
                hitUV = fract(hitPos.xy);

            float lod = clamp(log2(1.0 + tHit * 0.1), 0.0, 4.0);
            vec3 texColor = textureLod(blockTextures, vec3(hitUV, float(blockID)), lod).rgb;

            float diff = max(dot(hitNormal, normalize(-dirLight.direction)), 0.0);
            vec3 litColor = texColor * max((dirLight.ambient * 2.5 + dirLight.diffuse * diff * 1.5), vec3(0.25));

            float fade = clamp(float(i) / float(MAX_STEPS), 0.0, 1.0);
            return mix(litColor, texture(skybox, rayDir).rgb, fade * 0.35);
        }

        if (tMax.x < tMax.y)
        {
            if (tMax.x < tMax.z)
            {
                tHit = tMax.x;
                voxel.x += stepDir.x;
                tMax.x += tDelta.x;
                hitNormal = vec3(-stepDir.x, 0.0, 0.0);
            }
            else
            {
                tHit = tMax.z;
                voxel.z += stepDir.z;
                tMax.z += tDelta.z;
                hitNormal = vec3(0.0, 0.0, -stepDir.z);
            }
        }
        else
        {
            if (tMax.y < tMax.z)
            {
                tHit = tMax.y;
                voxel.y += stepDir.y;
                tMax.y += tDelta.y;
                hitNormal = vec3(0.0, -stepDir.y, 0.0);
            }
            else
            {
                tHit = tMax.z;
                voxel.z += stepDir.z;
                tMax.z += tDelta.z;
                hitNormal = vec3(0.0, 0.0, -stepDir.z);
            }
        }
    }

    return texture(skybox, rayDir).rgb;
}

void main()
{
    if (drawSingleColor)
    {
        FragColor = vec4(singleColor, 1.0f);
        return;
    }

    vec3 viewDir = normalize(viewPos - FragPos);
    vec2 uv = ParallaxMapping(TexCoord, normalize(transpose(TBN) * viewDir));

    if(DISCARD_OUTSIDE_UV && heightScale > 0.0 && (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0))
        discard;

    vec4 texColor = texture(material.diffuse, uv);

    if(texColor.a < 0.1)
        discard;

    vec3 diffTex = vec3(texColor);
    vec3 specTex = vec3(texture(material.specularMap, uv));

    aoTex = vec3(texture(material.ambientMap, uv));
    
    vec3 nmap = vec3(texture(material.normalMap, uv)) * 2.0 - 1.0;
    vec3 norm = normalize(TBN * nmap);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, diffTex, specTex);

    for( int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, diffTex, specTex);

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, diffTex, specTex);

    if (material.reflectivity > 0.0)
    {
        vec3 I = -viewDir;
        vec3 normN = normalize(Normal);
        vec3 R = (material.refractiveIndex > 0.0) ? refract(I, norm, 1.0 / material.refractiveIndex) : reflect(I, norm);

        if (dot(R, R) < 1e-5)
            R = reflect(I, norm);
        else if (!(material.refractiveIndex > 0.0) && dot(R, normN) < 0.0)
            R = reflect(R, normN);

        vec3 rayOrigin = FragPos + normN * ((material.refractiveIndex > 0.0) ? -0.05 : 0.05);
        vec3 reflectedColor = TraceVoxelRay(rayOrigin, normalize(R), material.refractiveIndex > 0.0);
        result = mix(result, reflectedColor, material.reflectivity);
    }

    FragColor = vec4(result, texColor.a);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffTex, vec3 specTex)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient* diffTex * material.ambient * aoTex;
    vec3 diffuse = light.diffuse * diff * diffTex;
    vec3 specular = light.specular * spec * specTex;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 FragPos, vec3 viewDir, vec3 diffTex, vec3 specTex)
{
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float dist = length(light.position - FragPos);
    float attentuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    vec3 ambient = light.ambient * diffTex * material.ambient * aoTex * attentuation;
    vec3 diffuse = light.diffuse * diff * diffTex * attentuation;
    vec3 specular = light.specular * spec * specTex * attentuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 FragPos, vec3 viewDir, vec3 diffTex, vec3 specTex)
{
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float dist = length(light.position - FragPos);
    float attentuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * diffTex * material.ambient * aoTex * attentuation;
    vec3 diffuse = light.diffuse * diff * diffTex * attentuation * intensity;
    vec3 specular = light.specular * spec * specTex * attentuation * intensity;

    return (ambient + diffuse + specular);
}