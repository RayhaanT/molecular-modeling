#version 400 core

out vec4 FragColour;

uniform vec3 objectColor;
uniform vec3 lightColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;

void main()
{
    float specularStrength = 0.5;
    vec3 viewPos = vec3(0.0, 0.0, 0.0);
    float ambient = 0.1;
    vec3 norm = normalize(Normal);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(LightPos - FragPos);
    //Negate lightDir because vector is currently pointing from cube to light
    //Reflect needs vector to be pointing to cube surface
    vec3 reflectDir = reflect(-lightDir, norm);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColour = vec4(result, 1);
}