#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#include "Scene.h"
#include <glfw3.h>
#include <iostream>
using namespace Game;
#pragma region Utilities
	bool firstMouse = true, addArrow = false;
	double lastMX, lastMY;
	float deltaTime = 0, lastTime = 0;
	Camera cam;
	unsigned int quadVAO = 0;
	unsigned int quadVBO;
	bool firstPress = true;
	void renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
	void changeSizeCallback(GLFWwindow * window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	void mouseMoveCallback(GLFWwindow * window, double xpos, double ypos) {
		if (firstMouse) {
			lastMX = xpos;
			lastMY = ypos;
			firstMouse = false;
		}
		cam.ProcessMouseMovement(xpos - lastMX, ypos - lastMY);
		lastMX = xpos;
		lastMY = ypos;
	}
	void processInput(GLFWwindow * window) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			printf("Escape!\n");
			glfwSetWindowShouldClose(window, true);
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && GetKeyState(VK_LSHIFT) < 0) {
			cam.ProcessKeyboard(FAST, deltaTime);
		}
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			cam.ProcessKeyboard(FORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && GetKeyState(VK_LSHIFT) < 0) {
			cam.ProcessKeyboard(FLEFT, deltaTime);
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			cam.ProcessKeyboard(LEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && GetKeyState(VK_LSHIFT) < 0) {
			cam.ProcessKeyboard(FRIGHT, deltaTime);
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			cam.ProcessKeyboard(RIGHT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			cam.ProcessKeyboard(BACKWARD, deltaTime);
		}
		if (GetKeyState(VK_SPACE) < 0) {
			cam.jump = true;
		}
		if (GetKeyState(VK_LMENU) < 0) {
			cam.look = true;
			if (firstPress) {
				cam.b4LookYaw = cam.Yaw;
				firstPress = false;
			}
		}
		else {
			cam.look = false;
			firstPress = true;
		}
	}
	void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			addArrow = true;
		}
	}
#pragma endregion
	int main() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Archer Game", NULL, NULL);
//		GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Archer Game", glfwGetPrimaryMonitor(), NULL); fullscreen
		if (window == NULL) {
			printf("Failed to create window\n");
			glfwTerminate();
			system("pause");
			return -1;
		}

		glfwMakeContextCurrent(window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			printf("Failed to initialize glad \n");
			glfwTerminate();
			system("pause");
			return -2;
		}
		glfwSetFramebufferSizeCallback(window, changeSizeCallback);
		glfwSetCursorPosCallback(window, mouseMoveCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);

		Shader depthShader("#version 440 core \n layout(location = 0) in vec3 pos;\n uniform mat4 lightSpaceMat;\n uniform mat4 model;\n void main()\n{ gl_Position = lightSpaceMat * model * vec4(pos, 1.0);\n }", "#version 440 core \n void main(){}", true, true, true);
		Shader shader("Shader.glsl", "Shader.frag");
		Shader skyBoxShader("sky.glsl", "sky.frag");
		Shader hdrFrame("fbo.glsl", "fbo.frag");
		Skybox sky("sandcastle");
		Scene scene;
#pragma region FrameBuffers
		const unsigned int SHADOW_WIDTH = 8560, SHADOW_HEIGHT = 8560;
		unsigned int depthMatFramebuffer, depthTexture;
		glGenFramebuffers(1, &depthMatFramebuffer);
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMatFramebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
		glDrawBuffer(GL_NONE); //not using draw or read, this is simply for depth
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		unsigned int hdrFrameBuffer, hdrTexture, hdrRenderBufferDepth;
		glGenFramebuffers(1, &hdrFrameBuffer);
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenRenderbuffers(1, &hdrRenderBufferDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, hdrRenderBufferDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

		glBindFramebuffer(GL_FRAMEBUFFER, hdrFrameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTexture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRenderBufferDepth);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion



		unsigned int groundTexture = loadTexture("Assets/render pebbles.jpg", true);

		glm::vec3 sunPos(0, 50, -50);
		shader.use();
		shader.setInt("shadowMap", 0);
		shader.setInt("diffuseTex", 1);
		shader.setInt("normalMap", 2);
		shader.setInt("specularMap", 3);
		shader.setBool("useNormal", false);
		shader.setBool("useSpecular", false);
		float near_plane = 50.0f, far_plane = 120.0f;
		shader.setFloat("near_plane", near_plane);
		shader.setFloat("far_plane", far_plane);
		client::startup("127.0.0.1");
#pragma region GameLoop
		while (!glfwWindowShouldClose(window)) {
			deltaTime = glfwGetTime() - lastTime;
			lastTime = glfwGetTime();
			client::startLoop();
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glm::mat4 lightProj, lightView, lightSpaceTransform;
//			lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 
			lightProj = glm::perspective(45.0f, (float)SHADOW_WIDTH / SHADOW_HEIGHT, near_plane, far_plane);
			lightView = glm::lookAt(sunPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			lightSpaceTransform = lightProj * lightView;
			depthShader.use();
			depthShader.setMat4("lightSpaceMat", lightSpaceTransform);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMatFramebuffer);
			glClear(GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			scene.renderScene(depthShader, true, cam, deltaTime, glm::mat4());
			glBindFramebuffer(GL_FRAMEBUFFER, hdrFrameBuffer);

			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 200.0f);
			glm::mat4 view = cam.GetViewMatrix();
			skyBoxShader.use();
			skyBoxShader.setMat4("view", glm::mat4(glm::mat3(view)));
			skyBoxShader.setMat4("projection", projection);
			sky.draw();
			shader.use();
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);
			shader.setVec3("viewPos", cam.Position);
			shader.setVec3("lightPos", sunPos);
			shader.setMat4("lightSpaceMatrix", lightSpaceTransform);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, groundTexture);
			scene.renderScene(shader, false, cam, deltaTime, glm::mat4(projection * view));
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			hdrFrame.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
//			glBindTexture(GL_TEXTURE_2D, depthTexture);
			hdrFrame.setFloat("exposure", 1.2f);
			renderQuad();

			if (addArrow) {
				addArrow = false;
				scene.addArrow(cam);
			}
			processInput(window);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
#pragma endregion
		glfwTerminate();
		getchar();

		return 0;




	}