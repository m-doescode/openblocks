#version 330 core

// Implements the Phong lighting model with respect to materials and lighting materials

// Structs

struct Material {
    vec3 diffuse;
    vec3 specular;
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
    // vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};


// I/O

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoords;

out vec4 FragColor;

#define NR_POINT_LIGHTS 4

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int numPointLights;
uniform DirLight sunLight;
uniform Material material;

// Functions

vec3 calculateDirectionalLight(DirLight light);
vec3 calculatePointLight(PointLight light);


// Main

void main() {
    vec3 result = vec3(0.0);

    result += calculateDirectionalLight(sunLight);
    
    for (int i = 0; i < numPointLights; i++) {
        result += calculatePointLight(pointLights[i]);
    }
    
    FragColor = vec4(result, 1);
}

vec3 calculateDirectionalLight(DirLight light) {
    // Calculate diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);

    // Calculate specular
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    return (ambient + diffuse + specular);
}

vec3 calculatePointLight(PointLight light) {
    // Calculate ambient light

    // Calculate diffuse light
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(light.position - vPos);
    float diff = max(dot(norm, lightDir), 0.0);

    // Calculate specular
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // Calculate attenuation
    float distance = length(light.position - vPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    return (ambient + diffuse + specular) * attenuation;
}