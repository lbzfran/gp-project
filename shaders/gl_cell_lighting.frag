#version 330
// A fragment shader for rendering fragments in the Phong reflection model.
layout (location=0) out vec4 FragColor;

// Inputs: the texture coordinates, world-space normal, and world-space position
// of this fragment, interpolated between its vertices.
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragWorldPos;

// Uniforms: MUST BE PROVIDED BY THE APPLICATION.

// The mesh's base (diffuse) texture.
uniform sampler2D baseTexture;

// Material parameters for the whole mesh: k_a, k_d, k_s, shininess.
uniform vec4 material;

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
    float spec = pow(max(dot(reflectDir, eyeDir), 0.0), material.w);

    vec3 ambient = material.x * light.ambient;
    vec3 diffuse = material.y * light.diffuse * lambertFactor;
    vec3 specular = material.z * light.specular * spec;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 eyeDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float lambertFactor = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(reflectDir, eyeDir), 0.0), material.w);

    vec3 ambient = material.x * light.ambient;
    vec3 diffuse = material.y * light.diffuse * lambertFactor;
    vec3 specular = material.z * light.specular * spec;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 eyeDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float lambertFactor = max(dot(normal, lightDir), 0.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));


    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(reflectDir, eyeDir), 0.0), material.w);

    float theta = dot(lightDir, normalize(-light.direction));

    vec3 ambient = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    if (theta > light.cutOff) {
        ambient = material.x * light.ambient;
        diffuse = material.y * light.diffuse * lambertFactor;
        specular = material.z * light.specular * spec;

        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }

    return (ambient + diffuse + specular);
}

void main() {

    vec3 norm = normalize(Normal);
    vec3 eyeDir = normalize(viewPos - FragWorldPos);

    vec3 result = ambientColor * material.x;
    result += CalcDirLight(dirLight, norm, eyeDir);
    result += CalcPointLight(pointLight, norm, FragWorldPos, eyeDir);
    result += CalcSpotLight(spotLight, norm, FragWorldPos, eyeDir);

    FragColor = vec4(result, 1.0) * texture(baseTexture, TexCoord);
}
