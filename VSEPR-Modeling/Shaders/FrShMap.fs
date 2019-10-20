#version 400 core

out vec4 FragColour;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
#define MAX_POINT_LIGHTS 18
uniform PointLight pointLights[MAX_POINT_LIGHTS];

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform PointLight light;
uniform vec3 viewPos;

// vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
//     vec3 lightDir = normalize(light.position - fragPos);

// }

void main()
{
    //Ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    //Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    //Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    //Attenuation
    float dis = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dis + light.quadratic * (dis * dis));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    FragColour = vec4(ambient + diffuse + specular, 1);
}