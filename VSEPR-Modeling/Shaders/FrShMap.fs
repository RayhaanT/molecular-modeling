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
#define MAX_POINT_LIGHTS 36
uniform PointLight pointLights[MAX_POINT_LIGHTS];

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform PointLight light;
uniform vec3 viewPos;

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    //Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    //Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    //Attenuation
    float dis = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dis + light.quadratic * (dis * dis));

    //Combined calculation
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
    //Constants
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    //Calculate point lights
    int filled = MAX_POINT_LIGHTS;
    for(int i = 0; i < MAX_POINT_LIGHTS; i++) {
        if(length(pointLights[i].diffuse) > 0) { }
        else {
            filled = i;
            break;
        }
    }
    vec3 result = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < filled; i++) {
        result += calculatePointLight(pointLights[i], norm, FragPos, viewDir);
    }

    FragColour = vec4(result, 1);
}