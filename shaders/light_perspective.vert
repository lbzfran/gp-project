#version 330
// A vertex shader for rendering vertices with normal vectors and texture coordinates,
// which creates outputs needed for a Phong reflection fragment shader.
layout (location=0) in vec3 vPosition;
layout (location=1) in vec3 vNormal;
layout (location=2) in vec2 vTexCoord;
layout (location=3) in vec3 vTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 FragWorldPos;
out vec3 Normal;
out mat3 TBN;

void main() {
    // Transform the vertex position from local space to clip space.
    gl_Position = projection * view * model * vec4(vPosition, 1.0);
    // Pass along the vertex texture coordinate.
    TexCoord = vTexCoord;
    // Transform the vertex normal from local space to world space, using the Normal matrix.
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * vNormal;

    FragWorldPos = vec3(model * vec4(vPosition, 1.0));

    // Gram-Schmidt optimization for TBN
    vec3 T = normalize(normalMatrix * vTangent);
    vec3 N = normalize(normalMatrix * vNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    TBN = mat3(T, B, N);
}
