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

const int FaceRight = 0;
const int FaceTop = 1;
const int FaceBack = 2;	
const int FaceLeft = 3;	
const int FaceBottom = 4;
const int FaceFront	= 5;

// I/O

in vec3 vPos;
in vec3 lPos;
in vec3 vNormal;
in vec3 lNormal;
flat in int vSurfaceZ;

out vec4 FragColor;

#define NR_POINT_LIGHTS 4

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int numPointLights;
uniform DirLight sunLight;
uniform Material material;
uniform sampler2DArray studs;
uniform samplerCube skybox;
uniform float transparency;
uniform float reflectance;
uniform vec3 texScale;


// Functions
vec3 calculateReflection();
vec3 calculateDirectionalLight(DirLight light);
vec3 calculatePointLight(PointLight light);
mat3 lookAlong(vec3 pos, vec3 forward, vec3 up);

// Main

void main() {
    vec3 result = vec3(0.0);

    result += calculateReflection();
    result += calculateDirectionalLight(sunLight);
    
    for (int i = 0; i < numPointLights; i++) {
        result += calculatePointLight(pointLights[i]);
    }

    vec3 otherVec = abs(dot(lNormal, vec3(0, 1, 0))) > 0.99 ? vec3(0, 0, 1)
                    : abs(dot(lNormal, vec3(0, 0, 1))) > 0.99 ? vec3(1, 0, 0)
                    : vec3(0, 1, 0);
                    // We use abs(lNormal) so opposing sides "cut" from the same side
    mat3 transform = transpose(inverse(lookAlong(vec3(0, 0, 0), abs(lNormal), otherVec)));
    
    vec2 texCoords = vec2((transform * lPos) * (transform * texScale) / 2) - vec2(mod((transform * texScale) / 4, 1));

    vec4 studPx = texture(studs, vec3(texCoords, vSurfaceZ));
    FragColor = vec4(mix(result, vec3(studPx), studPx.w), 1) * (1-transparency);
}

mat3 lookAlong(vec3 pos, vec3 forward, vec3 up) {
    vec3 f = normalize(forward); // Forward/Look
	vec3 u = normalize(up); // Up
	vec3 s = normalize(cross(f, u)); // Right
	u = normalize(cross(s, f));

	return mat3(s, u, f);
}


vec3 sampleSkybox()
{
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectDir = reflect(-viewDir, norm);

    return textureLod(skybox,reflectDir, 5.0 * (1.0-material.shininess)).rgb;
}

vec3 calculateReflection() {
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(viewPos - vPos);
    vec3 reflectDir = reflect(viewDir, norm);
    float fresnel = (pow(1.0-max(dot(viewDir, norm), 0.0), 5.0));
    vec3 result = sampleSkybox() * mix(fresnel * material.specular, vec3(1.0), reflectance);
    return result;
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
    // float fresnel = (pow(1.0-max(dot(viewDir, norm), 0.0), 5.0));
    
    
    vec3 ambient = light.ambient * (material.diffuse * (1.0-reflectance));
    vec3 diffuse = light.diffuse * diff * (material.diffuse * (1.0-reflectance));
    vec3 specular = light.specular * spec * material.specular;
    // specular += sampleSkybox() * fresnel * material.specular;
    
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