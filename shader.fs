// shader.fs

# version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct Material {
    vec3 ambient;
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
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

uniform bool drawSingleColor;
uniform vec3 singleColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffTex, vec3 specTex);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 FragPos, vec3 viewDir, vec3 diffTex, vec3 specTex);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 FragPos, vec3 viewDir, vec3 diffTex, vec3 specTex);

void main()
{
    if (drawSingleColor)
    {
        FragColor = vec4(singleColor, 1.0f);
        return;
    }
    vec3 diffTex = vec3(texture(material.diffuse, TexCoord));
    vec3 specTex = vec3(texture(material.specular, TexCoord));

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, diffTex, specTex);

    for( int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, diffTex, specTex);

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir, diffTex, specTex);

    FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffTex, vec3 specTex)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient* diffTex * material.ambient;
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

    vec3 ambient = light.ambient * diffTex * material.ambient * attentuation;
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

    vec3 ambient = light.ambient * diffTex * material.ambient * attentuation;
    vec3 diffuse = light.diffuse * diff * diffTex * attentuation * intensity;
    vec3 specular = light.specular * spec * specTex * attentuation * intensity;

    return (ambient + diffuse + specular);
}