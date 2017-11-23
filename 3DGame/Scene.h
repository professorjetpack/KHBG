#pragma once
#include "Assimp.h"
#include "Camera.h"
#include <vector>
#include <cstdio>
#include <glfw3.h>
#include "Shader.h"
#include <gtc/type_ptr.hpp>
#include "Client.h"
#define skySize 100.0f
namespace Game {
#pragma region Util
	unsigned int loadTexture(char const *path, bool repeat = false)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format; //determines format
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			if (nrComponents == 4 && repeat == false) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			printf("Texture failed to load at path: %s \n", path);
			stbi_image_free(data);
		}

		return textureID;
	}
	static unsigned int loadSky(std::vector<std::string> faces) {
		unsigned int texture;
		printf_s("allocated texture \n");
		glGenTextures(1, &texture);
		if (texture == NULL) {
			printf_s("Texture is null \n");
			system("pause");
		}
		else {
			printf_s("Texture is not null \n");
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char * data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				printf_s("data is not null \n");
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else {
				printf_s("Cubemap failes to load: %s\n", faces[i]);
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		printf("set texture parameters \n");
		return texture;
	}
	class Skybox {
	private:
		float skyboxVertices[144] = {
			// positions          
			-skySize,  skySize, -skySize,
			-skySize, -skySize, -skySize,
			skySize, -skySize, -skySize,
			skySize, -skySize, -skySize,
			skySize,  skySize, -skySize,
			-skySize, skySize, -skySize,

			-skySize, -skySize,  skySize,
			-skySize, -skySize, -skySize,
			-skySize,  skySize, -skySize,
			-skySize,  skySize, -skySize,
			-skySize,  skySize,  skySize,
			-skySize, -skySize,  skySize,

			skySize, -skySize, -skySize,
			skySize, -skySize,  skySize,
			skySize,  skySize,  skySize,
			skySize,  skySize, skySize,
			skySize,  skySize, -skySize,
			skySize, -skySize, -skySize,

			-skySize, -skySize,  skySize,
			-skySize,  skySize,  skySize,
			skySize,  skySize,  skySize,
			skySize,  skySize,  skySize,
			skySize, -skySize,  skySize,
			-skySize, -skySize,  skySize,

			-skySize,  skySize, -skySize,
			skySize,  skySize, -skySize,
			skySize,  skySize, skySize,
			skySize,  skySize,  skySize,
			-skySize,  skySize,  skySize,
			-skySize,  skySize, -skySize,

			-skySize, -skySize, -skySize,
			-skySize, -skySize,  skySize,
			skySize, -skySize, -skySize,
			skySize, -skySize, -skySize,
			-skySize, -skySize,  skySize,
			skySize, -skySize,  skySize
		};
		unsigned int skyVAO, skyVBO;
		unsigned int skyCube;
	public:
		Skybox(std::string baseName) {
			glGenVertexArrays(1, &skyVAO);
			glGenBuffers(1, &skyVBO);
			glBindVertexArray(skyVAO);
			glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
			glEnableVertexAttribArray(0);
			std::vector<std::string> faces;
			faces.push_back("Assets/skybox/" + baseName + "_rt.tga");
			faces.push_back("Assets/skybox/" + baseName + "_lf.tga");
			faces.push_back("Assets/skybox/" + baseName + "_up.tga");
			faces.push_back("Assets/skybox/" + baseName + "_dn.tga");
			faces.push_back("Assets/skybox/" + baseName + "_bk.tga");
			faces.push_back("Assets/skybox/" + baseName + "_ft.tga");
			skyCube = loadSky(faces);
			glBindVertexArray(0);
		}
		void draw() {
			glDepthMask(GL_FALSE);
			glBindVertexArray(skyVAO);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyCube);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthMask(GL_TRUE);

		}
	};
	class Quad {
	private:
		unsigned int texture;
		float vertices[48] =
		{
			25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
			-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

			25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
			25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
		};
		unsigned int VAO, VBO;
	public:
		Quad(char * texture = "none") {
			if (strcmp(texture, "none") != 0) {
				this->texture = loadTexture(texture);
			}
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glBindVertexArray(0);
		}
		void init(char * texture = "none") {
			if (strcmp(texture, "none") != 0) {
				this->texture = loadTexture(texture);
			}
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glBindVertexArray(0);
		}
		void draw() {
			glBindVertexArray(VAO);
			if (texture != NULL) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture);
			}
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	};
	class Arrow {
	private:
		glm::vec3 pos;
		Model arrow;
		float initialAngle;
		glm::vec3 velocity;
		double clock;
	public:
		Arrow() {};
		Arrow(const Camera & cam, const Model arrow) : arrow(arrow) {
			int xfactor = (cam.Front.z > 0) ? -1 : 1;
			int zfactor = (cam.Front.x < 0) ? -1 : 1;
			pos = (cam.Position - glm::vec3(0, 0.2, 0)) + glm::vec3(xfactor * 0.05, 0, zfactor * 0.05);
			if (!cam.look) {
				velocity = cam.Front;
			}
			else {
				velocity = cam.lastFront;
			}
			clock = glfwGetTime();
		}
		void draw(Shader shader, float dt, bool depthPass) {
			glm::mat4 model;
			double gravity = 16;			
			if(pos.y - 0.13 >= -3 && !depthPass){
				velocity.y = velocity.y - (gravity * ((glfwGetTime() - clock) / 1000.0));
				pos += velocity * glm::vec3(20) * glm::vec3(dt);
			}			
			double nextVelY = velocity.y - (gravity * (((glfwGetTime() + dt) - clock) / 1000.0));
			glm::vec3 nextVelocity = velocity;
			nextVelocity.y = pos.y - 0.5 > -3 ? nextVelY : velocity.y;

			model = glm::translate(model, pos);
			model = glm::scale(model, glm::vec3(0.05));

			glm::vec3 lookat = glm::vec3(nextVelocity * glm::vec3(20) * glm::vec3(dt));
			glm::vec3 axis = glm::normalize(glm::cross(velocity, glm::vec3(0, 1, 0)));
			float angle;
			glm::vec3 test = glm::normalize(velocity);
			if (1.0 - abs(test.z) < 0.4) {
				angle = atan(abs(nextVelocity.y / nextVelocity.z));
			}
			else {
				angle = atan(abs(nextVelocity.y / nextVelocity.x));
			}
			model = glm::rotate(model, angle + glm::radians(270.0f), axis);

			shader.setMat4("model", model);
			arrow.Draw(shader);
		}
	};
#pragma endregion
	class Scene {
	private:
		float lastYaw;
		float lastPitch;
		Model castle, street, house2, village, tower, cathedral, well, forum, bow, arrow, house, knight;
		Quad sun;
		Quad ground;
		glm::mat4 _model;
//		Ringbuffer arrows;
		std::vector<Arrow> arrows;
		int overflow = 0;
		std::vector<std::vector<glm::vec4>> triangles;
		std::vector<BoundingBox> hitBoxes;
		bool firstPass = true;
		std::vector<position> players;
		int err = 1;
	public:
		Scene() {
			castle.init("Assets/Castle X6.obj");
			house.init("Assets/house_obj.obj");
			street.init("Assets/medstreet.obj");
			sun.init("Assets/sun.png");
			forum.init("Assets/GothicSquare.3ds");
			ground.init("Assets/Grass.jpg");
			house2.init("Assets/Medieval-House.obj");
			village.init("Assets/Village.obj");
			tower.init("Assets/saintriqT3DS.obj");
			cathedral.init("Assets/kosciol.3ds");
			well.init("Assets/Well/well.obj");
			bow.init("Assets/Merciless Crossbow.obj");
			arrow.init("Assets/Arrow.fbx");
			knight.init("Assets/chevalier.obj");
			triangles = house.triangles;
			arrows.resize(30);

		}
		void addArrow(Camera & cam) {
/*			if (arrows.size() >= 50) {
				arrows.at(overflow) = Arrow(cam, arrow);
				overflow++;
				if (overflow >= 50) {
					overflow = 0;
				}
				return;
			}
			*/
			arrows.push_back(Arrow(cam, arrow));
		}
		void renderScene(Shader shader, bool depthPass, Camera & cam, float dt, glm::mat4 viewProj) {
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0.0));
			shader.setMat4("model", model);
			castle.Draw(shader);
#pragma region DrawHouse1
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-25, -3.0, 0));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			BoundingBox test;
			test.min = glm::vec3(-30, -3.0, -10);
			test.max = glm::vec3(-20, 3, 10);
			hitBoxes.push_back(test);
			_model = model;
			shader.setMat4("model", model);
//			shader.setBool("useNormal", true);
//			shader.setBool("useSpecular", true);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-25, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-25, -3.0, -5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -30.0, 0));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-30, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(25.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -3.0, 30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(76.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -3.0, 17));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -3.0, -30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-30, -3.0, -30));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.0, 5));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(33, -3.0, 17));
			model = glm::scale(model, glm::vec3(.003f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house.Draw(shader);
			
#pragma endregion
#pragma region DrawVillage
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -3.2, 30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, 30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, 30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, 10));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, -5));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-18, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			village.Draw(shader);
#pragma endregion
#pragma region DrawStreet
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(30, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(30, -3.2, -20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-20, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, 0));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, 13));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.2, -30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-20, -3.2, -30));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			street.Draw(shader);
#pragma endregion
#pragma region DrawHouse2
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(10, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(15, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.2, -20));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, 20));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-5, -3.2, -30));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(15, -3.2, -20));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(40, -3.2, -10));
			model = glm::scale(model, glm::vec3(.5f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			house2.Draw(shader);
#pragma endregion
#pragma region DrawMisc
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(25, -1, 0));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0, 0.0, 0.0));
			shader.setMat4("model", model);
			cathedral.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-10, -3.2, 20));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0, 0.0, 0.0));
			shader.setMat4("model", model);
			tower.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(20, -3.2, -10));
			model = glm::scale(model, glm::vec3(.3f));
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			tower.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(0, -3.0, 13));
			model = glm::scale(model, glm::vec3(.009f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0));
			shader.setMat4("model", model);
			well.Draw(shader);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(-30, -4, 20));
			model = glm::scale(model, glm::vec3(.029f));
			model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0, 0.0, 0.0));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0));
			shader.setMat4("model", model);
			forum.Draw(shader);
//			shader.setBool("useNormal", false);
//			shader.setBool("useSpecular", false);
#pragma endregion
#pragma region DrawRects
			glm::mat4 sunModel;
			sunModel = glm::mat4();
			sunModel = glm::translate(sunModel, glm::vec3(0.0, -2.0, 0.0));
			sunModel = glm::scale(sunModel, glm::vec3(2));
//			sunModel = glm::rotate(sunModel, glm::radians(90.0f), glm::vec3(0.0, 0.0, 0.0));
			shader.setMat4("model", sunModel);
			ground.draw();
			sunModel = glm::mat4();
			sunModel = glm::translate(sunModel, glm::vec3(0.0, 6.9, 0.0));
			sunModel = glm::scale(sunModel, glm::vec3(20));
			shader.setMat4("model", sunModel);
			ground.draw();
#pragma endregion
#pragma region DrawBow/Arrows
			model = glm::mat4();
			int xfactor = (cam.Front.z > 0) ? -1 : 1;
			int zfactor = (cam.Front.x < 0) ? -1 : 1;
			model = glm::translate(model, (cam.Position - glm::vec3(0, 0.2, 0)) + glm::vec3(xfactor * 0.05, 0, zfactor * 0.05));
			model = glm::scale(model, glm::vec3(0.009));
			if (!cam.look) {
				model = glm::rotate(model, glm::radians(-cam.Yaw), glm::vec3(0.0, 1.0, 0.0));
				model = glm::rotate(model, glm::radians(cam.Pitch), glm::vec3(0.0, 0.0, 1.0));
				lastYaw = -cam.Yaw;
				lastPitch = cam.Pitch;
			}
			else {
				model = glm::rotate(model, glm::radians(lastYaw), glm::vec3(0.0, 1.0, 0.0));
				model = glm::rotate(model, glm::radians(lastPitch), glm::vec3(0.0, 0.0, 1.0));
			}
			model = glm::rotate(model, glm::radians(278.0f), glm::vec3(0.0, 1.0, 0.0));
			model = glm::rotate(model, glm::radians(-6.3f), glm::vec3(0.0, 0.0, 1.0));
			shader.setMat4("model", model);
			bow.Draw(shader);
			if (depthPass == false && firstPass == true) {
				firstPass = false;
//				for (int i = 0; i < triangles.size(); i++) {
//					for (int j = 0; j < triangles[i].size(); j++) {
//						triangles[i][j] = glm::vec4(_model * triangles[i][j]);
//						printf("Triangle %d: %f %f %f \n", i, triangles[i][j].x, triangles[i][j].y, triangles[i][j].z);

//					}
//					printf("\n");
//				}
				
			}
			for (int i = 0; i < arrows.size(); i++) {
				arrows[i].draw(shader, dt, depthPass);
			}
			cam.applyForces(dt);
/*			if (depthPass == false) {
				glm::vec3 ray[] = { cam.Position, cam.Position + cam.Front };
				for (int i = 0; i < triangles.size(); i++) {
						glm::vec3 triangle[] = { triangles[i][0], triangles[i][1], triangles[i][2] };
						if (GameMath::rayTriangleCol2(ray, triangle)) {
							cam.Position.y += 1;
							
						}
				}
			}
			*/
#pragma endregion
#pragma region drawPlayers
			if (depthPass) {
				client::sendPos(position{ cam.Position.x, cam.Position.y, cam.Position.z, cam.Yaw });

				err = client::getPos(players);
				if (err != 0) printf("Error code %i while getting player positions! \n", err);
			}
			if (err == 0) {
				for (position p : players) {
					model = glm::rotate(model, glm::radians(p.yaw), glm::vec3(0, 1, 0));
					model = glm::translate(model, glm::vec3(p.x, p.y, p.z));
					shader.setMat4("model", model);
					knight.Draw(shader);
				}
			}
#pragma endregion

		}
	};
}