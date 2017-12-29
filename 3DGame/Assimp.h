#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Util.h"
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gl\GL.h>
#include <string>
#include <vector>
#include "Shader.h"
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>
namespace Game {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
		Vertex & operator=(const Vertex & vert) {
			this->position = vert.position;
			this->normal = vert.normal;
			this->texCoords = vert.texCoords;
			this->tangent = vert.tangent;
			return *this;
		}
	};
	struct Texture {
		unsigned int id;
		std::string type;
		aiString path;
		Texture & operator=(const Texture & tex) {
			this->id = tex.id;
			this->type = tex.type;
			this->path = tex.path;
			return *this;
		}
	};
	class Mesh {
	public:
		std::vector<Vertex> verticies;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
	public:
		Mesh(std::vector<Vertex> verticies, std::vector<unsigned int> indices, std::vector<Texture> textures) : verticies(verticies), indices(indices), textures(textures) {
			setupMesh();
		}
		Mesh & operator=(const Mesh & mesh) {
			this->verticies = mesh.verticies;
			this->indices = mesh.indices;
			this->textures = mesh.textures;
			return *this;
		}
		void Draw(Shader shader) {
			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			unsigned int normalNr = 1;
			for (unsigned int i = 0; i < textures.size(); i++) {
				//we do not want to touch texture0, that is the shadow map
				glActiveTexture(GL_TEXTURE1 + i); 
				std::stringstream stream;
				std::string number;
				std::string name = textures[i].type;
				if (name != "textures_diffuse")
					stream << diffuseNr++;		
				else if (name == "textures_specular") {	
					SHADER_SET_INT(shader, "specularMap", i);
					stream << specularNr++;
				}
				else if (name == "textures_normal") {
					stream << normalNr++;
//					continue;
					SHADER_SET_INT(shader, "normalMap", i);
//					printf("Normal identified: %s\n", textures[i].path.C_Str());
				}
				
				number = stream.str(); 

				SHADER_SET_FLOAT(shader, ("material." + name + number).c_str(), i);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
			}
//			glActiveTexture(GL_TEXTURE1);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	private:
		unsigned int VAO, VBO, EBO;
	private:
		void setupMesh() {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(Vertex), &verticies[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal))); //offsetof determines memory between start of a struct to one of its data members
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoords)));
			glEnableVertexAttribArray(2);
//			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
//			glEnableVertexAttribArray(3);
		}
	};
	class Model {
	public:
		std::vector<std::vector<glm::vec3>> points;
		std::vector<std::vector<unsigned int>> ids;
	public:
		Model(char * path) {
			loadModel(path);
		}
		Model() {};
		void Draw(Shader shader) {
			for (unsigned int i = 0; i < meshes.size(); i++) {
				meshes[i].Draw(shader);
			}
		}
		inline void init(char * path) {
			loadModel(path);
		}
		Model & operator=(const Model & mod) {
			this->meshes = mod.meshes;
			this->directory = mod.directory;
			this->textures_loaded = mod.textures_loaded;
			return *this;
		}
	private:
		std::vector<Mesh> meshes;
		std::string directory;
		std::vector<Texture> textures_loaded;
	private:
		void loadModel(std::string path) {
			Assimp::Importer importer;
			const aiScene * scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				printf_s("Assimp loading error \n%s\n", importer.GetErrorString());
				return;
			}
			directory = path.substr(0, path.find_last_of('/'));
			processNode(scene->mRootNode, scene);
		}
		void processNode(aiNode * node, const aiScene * scene) {
			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				//processes all the nodes meshes
				aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.push_back(processMesh(mesh, scene));
			}
			for (unsigned int i = 0; i < node->mNumChildren; i++) {
				processNode(node->mChildren[i], scene);
			}
		}
		Mesh processMesh(aiMesh * mesh, const aiScene * scene) {
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			std::vector<Texture> textures;
			std::vector<glm::vec3> nodes;
			std::vector<unsigned int> nodeIds;
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				glm::vec3 vector;
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.position = vector;
//				points.push_back(vector);
				if (mesh->mNormals != NULL) {
					vector.x = mesh->mNormals[i].x;
					vector.y = mesh->mNormals[i].y;
					vector.z = mesh->mNormals[i].z;
					vertex.normal = vector;
				}
				if (mesh->mTangents != NULL) {
					vector.x = mesh->mTangents[i].x;
					vector.y = mesh->mTangents[i].y;
					vector.z = mesh->mTangents[i].z;
					vertex.tangent = vector;
				} 
				if (mesh->mTextureCoords[0]) { 
											   
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.texCoords = vec;

				}
				else {
					vertex.texCoords = glm::vec2(0.0f);
				}
				vertices.push_back(vertex);
				nodes.push_back(vertex.position); 
			}
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);
					nodeIds.push_back(face.mIndices[j]);
//					points.push_back(vertices[face.mIndices[j]].position);
				}
			}
			if (mesh->mMaterialIndex >= 0) {
				aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
				std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "textures_normal");
				textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			}
			points.push_back(nodes);
			ids.push_back(nodeIds);
			return Mesh(vertices, indices, textures);
		}

		unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
		{
			std::string filename = std::string(path);
			filename = directory + '/' + filename;
			printf("Texture: %s \n", filename.c_str());

			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
			if (data)
			{
				GLenum format;
				if (nrComponents == 1)
					format = GL_RED;
				else if (nrComponents == 3)
					format = GL_RGB;
				else if (nrComponents == 4)
					format = GL_RGBA;

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
				if (nrComponents != 4) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				else {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else
			{
				printf_s("Texture failed to load at %s\n", path);
				stbi_image_free(data);
			}

			return textureID;
		}
		std::vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName) {
			std::vector<Texture> textures;
			for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
			{
				aiString str;
				mat->GetTexture(type, i, &str);
				bool skip = false;
				for (unsigned int j = 0; j < textures_loaded.size(); j++)
				{
					if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
					{
						textures.push_back(textures_loaded[j]);
						skip = true;
						break;
					}
				}
				if (!skip)
				{   
					Texture texture;
					texture.id = TextureFromFile(str.C_Str(), this->directory, true);
					texture.type = typeName;
					texture.path = str;
					textures.push_back(texture);
					textures_loaded.push_back(texture); 
				}
			}
			return textures;
		}
	};
}
