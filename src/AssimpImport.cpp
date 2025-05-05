#include "AssimpImport.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <unordered_map>
#include <glad/glad.h>

const size_t FLOATS_PER_VERTEX = 3;
const size_t VERTICES_PER_FACE = 3;

void calculateTangents(std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices) {
    for (uint32_t i = 0; i < indices.size(); i += 3) {
        uint32_t i1 = indices[i];
        uint32_t i2 = indices[i + 1];
        uint32_t i3 = indices[i + 2];

        glm::vec3 v1 = glm::vec3(vertices[i1].x, vertices[i1].y, vertices[i1].z);
        glm::vec3 v2 = glm::vec3(vertices[i2].x, vertices[i2].y, vertices[i2].z);
        glm::vec3 v3 = glm::vec3(vertices[i3].x, vertices[i3].y, vertices[i3].z);

        glm::vec2 uv1 = glm::vec2(vertices[i1].u, vertices[i1].v);
        glm::vec2 uv2 = glm::vec2(vertices[i2].u, vertices[i2].v);
        glm::vec2 uv3 = glm::vec2(vertices[i3].u, vertices[i3].v);

        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = v3 - v1;

        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        vertices[i1].tangent += tangent;
        vertices[i2].tangent += tangent;
        vertices[i3].tangent += tangent;
    }

    for (auto& vertex : vertices) {
        vertex.tangent = glm::normalize(vertex.tangent);
    }
}

std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, const std::filesystem::path& modelPath,
	std::unordered_map<std::string, Texture>& loadedTextures) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString name;
		mat->GetTexture(type, i, &name);
		std::filesystem::path texPath = modelPath.parent_path() / name.C_Str();
		std::cout << "loading " << texPath << std::endl;

		auto existing = loadedTextures.find(texPath.string());
		if (existing != loadedTextures.end()) {
			textures.push_back(existing->second);
		}
		else {
			StbImage image;
			image.loadFromFile(texPath.string());
			Texture tex = Texture::loadImage(image, typeName);
			textures.push_back(tex);
			loadedTextures.insert(std::make_pair(texPath.string(), tex));
		}
	}
	return textures;
}

Mesh3D fromAssimpMesh(const aiMesh* mesh, const aiScene* scene, const std::filesystem::path& modelPath,
	std::unordered_map<std::string, Texture>& loadedTextures) {
	std::vector<Vertex3D> vertices;

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		auto& meshVertex = mesh->mVertices[i];
		auto& texCoord = mesh->mTextureCoords[0][i];
		auto& normal = mesh->mNormals[i];

        vertices.push_back(Vertex3D(
            meshVertex.x,
            meshVertex.y,
            meshVertex.z,

            normal.x,
            normal.y,
            normal.z,

            texCoord.x,
            texCoord.y
        ));
	}

	std::vector<uint32_t> faces;
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		auto& meshFace = mesh->mFaces[i];

        faces.push_back(meshFace.mIndices[0]);
        faces.push_back(meshFace.mIndices[1]);
        faces.push_back(meshFace.mIndices[2]);
	}

    calculateTangents(vertices, faces);

	// Load any base textures, specular maps, and normal maps associated with the mesh.
	std::vector<Texture> textures = {};
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
			aiTextureType_DIFFUSE, "material.diffuse", modelPath, loadedTextures);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = loadMaterialTextures(material,
			aiTextureType_SPECULAR, "material.specular", modelPath, loadedTextures);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		std::vector<Texture> normalMaps = loadMaterialTextures(material,
			aiTextureType_HEIGHT, "material.normal", modelPath, loadedTextures);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		normalMaps = loadMaterialTextures(material,
			aiTextureType_NORMALS, "material.normal", modelPath, loadedTextures);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}

	return Mesh3D(std::move(vertices), std::move(faces), std::move(textures));
}



Object3D assimpLoad(const std::string& path, bool flipTextureCoords) {
	Assimp::Importer importer;

	auto options = aiProcessPreset_TargetRealtime_MaxQuality |
        aiProcess_Triangulate |
        aiProcess_GenNormals;
	if (flipTextureCoords) {
		options |= aiProcess_FlipUVs;
	}
	const aiScene* scene = importer.ReadFile(path, options);

	// If the import failed, report it
	if (nullptr == scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		auto* error = importer.GetErrorString();
		std::cerr << "Error loading assimp file: " + std::string(error) << std::endl;
		throw std::runtime_error("Error loading assimp file: " + std::string(error));

	}
	else {

	}
	std::vector<Mesh3D> meshes;
	std::unordered_map<std::string, Texture> loadedTextures;
	auto ret = processAssimpNode(scene->mRootNode, scene, std::filesystem::path(path), loadedTextures);
	return ret;
}

Object3D processAssimpNode(aiNode* node, const aiScene* scene,
	const std::filesystem::path& modelPath,
	std::unordered_map<std::string, Texture>& loadedTextures) {

	// Load the aiNode's meshes.
	std::vector<Mesh3D> meshes;
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(fromAssimpMesh(mesh, scene, modelPath, loadedTextures));
	}

	std::vector<Texture> textures;
	for (auto& p : loadedTextures) {
		textures.push_back(p.second);
	}
	glm::mat4 baseTransform;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			baseTransform[i][j] = node->mTransformation[j][i];
		}
	}
	auto parent = Object3D(std::move(meshes), baseTransform);

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		Object3D child = processAssimpNode(node->mChildren[i], scene, modelPath, loadedTextures);
		parent.addChild(std::move(child));
	}

	return parent;
}
