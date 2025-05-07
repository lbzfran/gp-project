#version 330
// A fragment shader for rendering fragments in the Phong reflection model.
layout (location=0) out vec4 FragColor;

// Inputs: the texture coordinates, world-space normal, and world-space position
// of this fragment, interpolated between its vertices.
in vec2 TexCoord;
in vec3 FragWorldPos;
in mat3 TBN;
// in vec3 Normal;

// Uniforms: MUST BE PROVIDED BY THE APPLICATION.

struct Material {
    sampler2D normal;   // normalMap

    sampler2D diffuse;  // baseTexture
    sampler2D specular; // specularTexture
    float     shininess;
};
uniform Material material;

// Location of the camera.
uniform vec3 viewPos;

// Ambient light color.
uniform vec3 ambientColor;

struct DirLight {
    vec3 direction;

    // represents color
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

// Point Light
struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLight;

struct SpotLight {
    vec3 direction;
    vec3 position;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 eyeDir) {
    vec3 lightDir = normalize(-light.direction);

    float lambertFactor = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(reflectDir, eyeDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = vec3(0);
    if (lambertFactor > 0.75) {
        diffuse *= vec3(0.8);
        specular = light.specular * (spec / 2) * vec3(texture(material.specular, TexCoord));
    }
    else if (lambertFactor > 0.5) {
        diffuse *= vec3(0.6);
        // specular = light.specular * (spec / 2) * vec3(texture(material.specular, TexCoord));
    }
    else if (lambertFactor > 0.25) {
        diffuse *= vec3(0.2);
    }
    else {
        diffuse *= vec3(0);
    }

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 eyeDir) {
    vec3 lightDir = normalize(light.position - FragWorldPos);

    float lambertFactor = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position - FragWorldPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(reflectDir, eyeDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = vec3(0);
    if (lambertFactor > 0.8) {
        diffuse *= vec3(0.8);
        specular = light.specular * (spec / 2) * vec3(texture(material.specular, TexCoord));
    }
    else if (lambertFactor > 0.4) {
        diffuse *= vec3(0.6);
        // specular = light.specular * (spec / 2) * vec3(texture(material.specular, TexCoord));
    }
    else if (lambertFactor > 0.2) {
        diffuse *= vec3(0.2);
    }
    else {
        diffuse *= vec3(0);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 eyeDir) {
    // WARN(liam): could be incorrect
    // vec3 lightDir = normalize(-light.direction);
    vec3 lightDir = normalize(light.position - FragWorldPos);

    float lambertFactor = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position - FragWorldPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(reflectDir, eyeDir), 0.0), material.shininess);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = vec3(0);
    if (lambertFactor > 0.8) {
        diffuse *= vec3(0.8);
        specular = light.specular * (spec / 2) * vec3(texture(material.specular, TexCoord));
    }
    else if (lambertFactor > 0.4) {
        diffuse *= vec3(0.6);
        // specular = light.specular * (spec / 2) * vec3(texture(material.specular, TexCoord));
    }
    else if (lambertFactor > 0.2) {
        diffuse *= vec3(0.2);
    }
    else {
        diffuse *= vec3(0);
    }

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

void main() {

    // vec3 norm = normalize(Normal);
    vec3 norm = texture(material.normal, TexCoord).rgb;
    norm = normalize(TBN * (norm * 2.0 - 1.0));

    vec3 eyeDir = normalize(viewPos - FragWorldPos);

    vec3 result = vec3(0);
    result += CalcDirLight(dirLight, norm, eyeDir);
    result += CalcPointLight(pointLight, norm, eyeDir);
    result += CalcSpotLight(spotLight, norm, eyeDir);

    FragColor = vec4(result, 1.0);
}
