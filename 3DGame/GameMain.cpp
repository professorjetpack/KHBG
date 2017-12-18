#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#define _WINSOCKAPI_
#include <Windows.h>
#include "Scene.h"
#include <map>
#include <iostream>
#include <ft2build.h>
#include <freetype\freetype.h>
#include <thread>
#include <mutex>
#define FT_FREETYPE_H
using namespace Game;
#pragma region Utilities
#define SCR_QUAD_SIZE 1.01f
	bool firstMouse = true, addArrow = false;
	double lastMX, lastMY;
	float deltaTime = 0, lastTime = 0;
	Camera cam;
	Scene * scene;
	unsigned int quadVAO = 0;
	unsigned int quadVBO;
	unsigned int textVAO, textVBO;
	bool firstPress = true;
	double healTime = 0, shootTime = 0;
	bool healable = false, suppliable = false, scalable = false;
	bool winner = false;
	int arrows = 50;
	bool playing = false;
	char keyList[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	std::string command;
	bool typing = false;
	std::mutex mu;
	double lastKey = 0;
	int fps = 0;
	bool sendCommand;
	void renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				-10.0f,  SCR_HEIGHT + 10, 0.0f, 1.0f,
				-10.0f, -10.0f, 0.0f, 0.0f,
				SCR_WIDTH + 10,  SCR_HEIGHT + 10, 1.0f, 1.0f,
				SCR_WIDTH + 10, -10.0f, 1.0f, 0.0f,
			};
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
	struct Letter {
		unsigned int ID;
		glm::ivec2 size;
		glm::ivec2 bearing;
		unsigned int advance;
	};
	std::map<char, Letter> letters;
	void createBuffers() {
		glGenVertexArrays(1, &textVAO);
		glBindVertexArray(textVAO);
		glGenBuffers(1, &textVBO);
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glBindVertexArray(0);
	}
	void loadFont(std::string fontName) {
		FT_Library ft;
		if(FT_Init_FreeType(&ft)) printf("Could not init FreeType \n");
		FT_Face face;
		char path[MAX_PATH];
		sprintf_s(path, MAX_PATH, "Assets/fonts/%s", fontName.c_str());
		if(FT_New_Face(ft, path, 0, &face)) printf("Failed to load font %s \n", path);
		FT_Set_Pixel_Sizes(face, 0, 48);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //set packing to 1 byte alignment since we have grayscale images
		for (unsigned char c = 0; c < 128; c++) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) { printf("Failed to load glyph %s \n", c); continue; }
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			Letter let = { texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x };
			letters.insert(std::pair<char, Letter>(c, let));
		}
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}
	void renderText(Shader & s, std::string text, float x, float y, float scale, glm::vec3 color) {
		s.use();
		s.setVec3("textColor", color);
		glBindVertexArray(textVAO);
		glActiveTexture(GL_TEXTURE1);
		for (auto c = text.begin(); c != text.end(); c++) {
			Letter let = letters[*c];
			float xpos = x + let.bearing.x * scale;
			float ypos = y - (let.size.y - let.bearing.y) * scale;
			float width = let.size.x * scale;
			float height = let.size.y * scale;

			float vertices[6][4] = {
				{ xpos,     ypos + height,   0.0, 0.0 },
				{ xpos,     ypos,       0.0, 1.0 },
				{ xpos + width, ypos,       1.0, 1.0 },

				{ xpos,     ypos + height,   0.0, 0.0 },
				{ xpos + width, ypos,       1.0, 1.0 },
				{ xpos + width, ypos + height,   1.0, 0.0 }
			};
			glBindTexture(GL_TEXTURE_2D, let.ID);
			glBindBuffer(GL_ARRAY_BUFFER, textVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			x += (let.advance >> 6) * scale;
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE, 0);
	}
	void changeSizeCallback(GLFWwindow * window, int width, int height) {
		glViewport(0, 0, width, height);
	}
	void mouseMoveCallback(GLFWwindow * window, double xpos, double ypos) {
		if (scene->isDead() || typing) return;
		if (firstMouse) {
			lastMX = xpos;
			lastMY = ypos;
			firstMouse = false;
		}
		cam.ProcessMouseMovement(xpos - lastMX, ypos - lastMY);
		lastMX = xpos;
		lastMY = ypos;
	}
	void getCommand() {
		while (true) {
			for (int i = 0; i < 37; i++) {
				if (GetAsyncKeyState(keyList[i]) && GetAsyncKeyState(VK_SHIFT)) {
					std::lock_guard<std::mutex> guard(mu);
					command += keyList[i];
				}
				else if (GetAsyncKeyState(keyList[i])) {
					std::lock_guard<std::mutex> guard(mu);
					command += tolower(keyList[i]);
				}
			}
			if (GetAsyncKeyState(VK_RETURN)) {
				std::lock_guard<std::mutex> guard(mu);
				sendCommand = true;
				typing = false;
				return;
			}
			else if (GetAsyncKeyState(VK_BACK) && command.size() > 0) {
				std::lock_guard<std::mutex> guard(mu);
				std::string buffer;
				for (int i = 0; i < command.size() - 1; i++) {
					buffer += command[i];
				}
				command = buffer;
			}
			else if (GetAsyncKeyState(VK_SPACE)) {
				std::lock_guard<std::mutex> guard(mu);
				command += " ";
			}
			Sleep(80);
		}
	}
	void processInput(GLFWwindow * window) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			printf("Escape!\n");
			client::shutdown();
			glfwSetWindowShouldClose(window, true);
		}
		else if (glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS && !typing) {			
			std::thread t(getCommand);
			t.detach();
			std::lock_guard<std::mutex> guard(mu);
			typing = true;
		}
		if (scene->isDead() || typing) return;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			if (healable) {
				if (glfwGetTime() > healTime + 1) {
					scene->setHealth(scene->getHealth() + 2);
					scene->getSoundEngine()->play2D("Assets/sounds/water-swirl.wav");
					healTime = glfwGetTime();
				}
			}
			else if (suppliable) {
				if (glfwGetTime() > healTime + 0.6) {
					arrows++;
					scene->getSoundEngine()->play2D("Assets/sounds/arrow-nock.wav");
					healTime = glfwGetTime();
				}
			}
			else if (scalable) {
				cam.Position = glm::vec3(-0.13, 2, 7.2);
				cam.oldPos = cam.Position;
			}
		}
		cam.move = false; cam.fastMove = false;
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
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
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
		if (winner || scene->isDead() || typing) return;
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			if (glfwGetTime() > shootTime + 0.5 && arrows > 0) {
				shootTime = glfwGetTime();
				arrows--;
				addArrow = true;
			}
		}
#ifdef MAPPING
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			std::ofstream out;
			out.open("scene1Map.db", std::ios::binary | std::ios::app);
			if (!out.is_open()) printf("Cannot open file for output \n");
			else {
				out.write((char*)&cam.Position.x, sizeof(float));
				out.write((char*)&cam.Position.y, sizeof(float));
				out.write((char*)&cam.Position.z, sizeof(float));
				printf("Output point %f, %f, %f \n", cam.Position.x, cam.Position.y, cam.Position.z);
				out.close();
			}
		}
#endif
	}
	void loadSound(char * file, unsigned char ** data, unsigned int & size) {
		FILE * sound;
		fopen_s(&sound, file, "rb");
		fseek(sound, 0, SEEK_END);
		int totalSize = ftell(sound);
		size = totalSize;
		fseek(sound, 0, SEEK_SET);
		*data = new unsigned char[totalSize];
		fread_s(*data, totalSize, totalSize, 1, sound);
		fclose(sound);

	}
	void soundEffect(byte * data, int size, double percentDecrease) {
		unsigned char * oldData = new unsigned char[size];
		memcpy_s(oldData, size, data, size);
		int sampleCount = (size - 40) / sizeof(int16_t);
		int16_t * samp = (int16_t *)&(data[44]);
		for (int i = 0; i < sampleCount; i++) {
			samp[i] -= (int16_t)(samp[i] * percentDecrease);
		}
		PlaySound((LPCSTR)data, NULL, SND_MEMORY | SND_ASYNC);
		memcpy_s(data, size, oldData, size);
		delete[] oldData;
	}
	void seekPosition(byte * data, int size, int second, int totalSeconds, byte ** newData) {
		byte * dataOffset = (byte*)&data[44];
		byte * end = (byte*)&data[size - 1];
		unsigned int dataSize = end - dataOffset;
//		unsigned int bytesPerSecond = dataSize / totalSeconds;
		int byteRate = *((int*)&data[28]);
		unsigned int elapsedBytes = byteRate * second;
		*((int*)&data[40]) = dataSize - elapsedBytes;
		*newData = new byte[size - elapsedBytes];
		memcpy_s(*newData, size - elapsedBytes, (char*)data, 44);
		memcpy_s(*newData + 44, size - elapsedBytes, (char*)(dataOffset + elapsedBytes), dataSize - elapsedBytes);
//		memcpy_s(*newData, dataSize - elapsedBytes, (char*)(dataOffset + elapsedBytes), dataSize - elapsedBytes);


	}
#pragma endregion
	int main() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		//this pc supports 4.4, set it to 3.3, the earliest version we can support to allow more ppl to play it
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); need this line in case of need to compile for Mac
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
		glEnable(GL_PROGRAM_POINT_SIZE);

		Shader depthShader("depth.glsl", "depth.frag");
		Shader shader("Shader.glsl", "Shader.frag");
		Shader skyBoxShader("sky.glsl", "sky.frag");
		Shader hdrFrame("fbo.glsl", "fbo.frag");
		Skybox sky("sandcastle");
		scene = sceneFactory(0);
		float red = 1.5;
		float clearTimes = 1;

		irrklang::ISoundEngine * mainEngine = irrklang::createIrrKlangDevice();
#pragma region FrameBuffers
		const unsigned int SHADOW_WIDTH = 12560, SHADOW_HEIGHT = 12560;
		unsigned int depthMatFramebuffer, depthTexture;
		glGenFramebuffers(1, &depthMatFramebuffer);
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
		hdrFrame.use();
		hdrFrame.setInt("hdrBuffer", 0);
		hdrFrame.setInt("text", 1);
		shader.use();
		shader.setInt("shadowMap", 0);
		shader.setInt("diffuseTex", 1);
		shader.setInt("normalMap", 2);
		shader.setInt("specularMap", 3);
		shader.setBool("useNormal", false);
		shader.setBool("useSpecular", false);
		float near_plane = 30.0f, far_plane = 120.0f;
		shader.setFloat("near_plane", near_plane);
		shader.setFloat("far_plane", far_plane);
		client::startup("127.0.0.1");
		client::sendName("Rex");


		createBuffers();
		loadFont("IMMORTAL.ttf");
		byte * intense;
		byte * newIntense;
		unsigned int intenseSize;
		loadSound("Assets/sounds/timeLeft.wav", &intense, intenseSize);
		bool once = false;
		bool heartBeat = false;
		float frames = 1;
		double fpsChange = 0;
		auto x = glm::vec4(1, 2, 3, 4);
		auto y = glm::mat4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
		glm::vec3 result = GameMath::vectorMatrixMultiply(x, y);
		printf("result: %f, %f, %f \n", result.x, result.y, result.z);
#pragma region GameLoop
		while (!glfwWindowShouldClose(window)) {
			deltaTime = glfwGetTime() - lastTime;
			lastTime = glfwGetTime();
			client::startLoop();
			glClearColor(0.f, 0.f, 0.f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glm::mat4 lightProj, lightView, lightSpaceTransform;
			lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane); 
//			lightProj = glm::perspective(45.0f, (float)SHADOW_WIDTH / SHADOW_HEIGHT, near_plane, far_plane);
			lightView = glm::lookAt(sunPos, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));
			lightSpaceTransform = lightProj * lightView;
			depthShader.use();
			depthShader.setMat4("lightSpaceMat", lightSpaceTransform);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMatFramebuffer);
			glClear(GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			scene->renderScene(depthShader, true, cam, deltaTime);
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
			scene->renderScene(shader, false, cam, deltaTime, view, projection);
//			printf("Pos: %f, %f, %f \n", cam.Position.x, cam.Position.y, cam.Position.z);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClear(GL_COLOR_BUFFER_BIT);
			hdrFrame.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);
//			glBindTexture(GL_TEXTURE_2D, depthTexture);
			t_clock time{ -2, 0 };
			client::getTime(time);
			if (time.minutes == 0 && time.seconds == 0) {
				if (!winner) PlaySound("Assets/sounds/victory.wav", NULL, SND_FILENAME | SND_ASYNC);
				winner = true;
			}
			else if (time.minutes == 1 && time.seconds == 21) {
				PlaySound("Assets/sounds/timeLeft.wav", NULL, SND_FILENAME | SND_ASYNC);
				playing = true;
			}
			else if (!playing) {
				if ((time.minutes == 1 && time.seconds < 21)) {
					int elapsed = 21 - time.seconds;
					seekPosition(intense, intenseSize, elapsed, 81, &newIntense);
					PlaySound((LPCSTR)newIntense, NULL, SND_MEMORY | SND_ASYNC);
					playing = true;
				}
				else if (time.minutes == 0) {
					int elapsed = 60 - (time.seconds - 21);
					seekPosition(intense, intenseSize, elapsed, 81, &newIntense);
					PlaySound((LPCSTR)newIntense, NULL, SND_MEMORY | SND_ASYNC);
					playing = true;
				}
			}
			if (winner && time.minutes != 0) {
				winner = false;
				scene->reset(cam);
				arrows = 50;
			}
			hdrFrame.setFloat("exposure", 1.2f);
			hdrFrame.setBool("damage", scene->getDamage());
			if (scene->getHealth() <= 40) {
				if (!heartBeat) 
					mainEngine->play2D("Assets/sounds/heartbeat.wav", true); heartBeat = true;
			}
			else {
				if (heartBeat)
					heartBeat = false; mainEngine->stopAllSounds();
			}
			hdrFrame.setInt("health", scene->getHealth());
			hdrFrame.setBool("shake", scene->getShake());
			hdrFrame.setFloat("shakeTime", scene->getShakeTime());
			hdrFrame.setBool("dead", scene->isDead());
			projection = glm::mat4();
			projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
			hdrFrame.setMat4("projection", projection);
			if (scene->isClearing()) {
				red -= 0.01;// *clearTimes;
				if (red < 1.0) {
					red = 1.0;
					clearTimes++;
					if (clearTimes > 4) clearTimes = 1;
				}
			}
			else {
				red = 1.3;
			}
			hdrFrame.setBool("clearUp", scene->isClearing());
			hdrFrame.setFloat("red", red);
			char msg[50];
			sprintf_s(msg, 50, "Health: %d", scene->getHealth());
			renderText(hdrFrame, msg, 10, SCR_HEIGHT - 100, SCR_WIDTH / 1920, glm::vec3(1, 0, 0));
			sprintf_s(msg, 50, "Arrows: %d", arrows);
			renderText(hdrFrame, msg, SCR_WIDTH - 500, SCR_HEIGHT - 100, SCR_WIDTH / 1920.0, glm::vec3(1, 0, 0));
			sprintf_s(msg, 50, "Kills: %d", scene->getKills());
			renderText(hdrFrame, msg, SCR_WIDTH - 500, SCR_HEIGHT - 50, SCR_WIDTH / 1920.0, glm::vec3(0, .75, 1));
			int leaderKills;
			std::string leaderName;
			if (client::getLeader(leaderKills, leaderName) != 0) {
				leaderKills = 0;
				leaderName = "offline";
			}
			if (!winner) {
				sprintf_s(msg, 50, "Leader: %s : %d kills", leaderName.c_str(), leaderKills);
				renderText(hdrFrame, msg, 10, SCR_HEIGHT - 50, SCR_WIDTH / 1920, glm::vec3(0, .75, 1));
				if (M_DISTANCE(glm::vec3(0, -2, 20), cam.Position + cam.Front) < 1 && scene->getHealth() < 84) {
					healable = true;
					suppliable = false;
					renderText(hdrFrame, "Hold e to heal at well", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
				}
				else if ((M_DISTANCE(glm::vec3(-9, -2, 20), cam.Position + cam.Front) < 1.7 || M_DISTANCE(glm::vec3(19, -2, -10), cam.Position + cam.Front) < 1.7)
					&& arrows < 50) {
					suppliable = true;
					healable = false;
					renderText(hdrFrame, "Hold e to resuply at fort", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
				}
				else if (M_DISTANCE(glm::vec3(19.36, -2, -1.6), cam.Position + cam.Front) < 1.7) {
					healable = false;
					suppliable = false;
					scalable = true;
					renderText(hdrFrame, "Press e to scale cliff", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
				}
				else {
					healable = false; suppliable = false;
				}
				if (M_DISTANCE(glm::vec3(0.038, 0.83, 5.87), cam.Position + cam.Front) < 1.7) {
					renderText(hdrFrame, "Press e to enter castle", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('E') < 0) cam.Position = glm::vec3(-0.128, 1.3, 3.49); cam.oldPos = cam.Position;
				}
				else if (M_DISTANCE(glm::vec3(1.43, 1.36, 2.9), cam.Position + cam.Front) < 0.5) {
					renderText(hdrFrame, "Press f to climb tower", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) cam.Position = glm::vec3(2.169, 6.44, 4.02); cam.oldPos = cam.Position;
				}
				else if (M_DISTANCE(glm::vec3(-2.68, 1.6, 3.1), cam.Position + cam.Front) < 0.5) {
					renderText(hdrFrame, "Press f to climb tower", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) cam.Position = glm::vec3(-3.16, 8.8, -4.3); cam.oldPos = cam.Position;
				}
				else if (M_DISTANCE(glm::vec3(0.77, 1.7, 1.12), cam.Position + cam.Front) < 0.5) {
					renderText(hdrFrame, "Press f to call doctor", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) scene->setHealth(120);
				}
				else if (M_DISTANCE(glm::vec3(-33, -2, 20), cam.Position + cam.Front) < 1.7) {
					renderText(hdrFrame, "Press e to climb tower", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('E') < 0) cam.Position = glm::vec3(-29, 6.2, 20); cam.oldPos = cam.Position;
				}
				if (typing) {
					std::lock_guard<std::mutex> guard(mu);
					std::string buffer = "/";
					buffer.append(command);
					renderText(hdrFrame, buffer, SCR_WIDTH / 12.0, SCR_HEIGHT / 12.0, SCR_HEIGHT / 2160.0, glm::vec3(1));
				}
				
			}
			else {
				sprintf_s(msg, 50, "%s won with %d kills!!!", leaderName.c_str(), leaderKills);
				renderText(hdrFrame, msg, SCR_WIDTH / 2 - SCR_WIDTH / 8, SCR_HEIGHT /2 + SCR_HEIGHT / 4, SCR_WIDTH / 1920.0, glm::vec3(.83, .68, .22));
				playing = false;
			}
			if (time.minutes != -2) {
				if (time.seconds < 10) {
					sprintf_s(msg, 50, "%d:0%d", time.minutes, time.seconds);
				}
				else {
					sprintf_s(msg, 50, "%d:%d", time.minutes, time.seconds);
				}
				renderText(hdrFrame, msg, SCR_WIDTH / 2, SCR_HEIGHT - 50, SCR_WIDTH / 1920.0, glm::vec3(0, 1, 0));
			}
			if (frames >= fps) {
				fps = frames / (glfwGetTime() - fpsChange);
				frames = 0;
				fpsChange = glfwGetTime();
			}
			sprintf_s(msg, 50, "Fps: %d", fps);
			renderText(hdrFrame, msg, SCR_WIDTH - SCR_WIDTH / 10, SCR_HEIGHT / 13, SCR_WIDTH / 1920, glm::vec3(1));
			hdrFrame.setBool("mainQuad", true);
			renderQuad();
			hdrFrame.setBool("mainQuad", false);

			if (addArrow) {
				addArrow = false;
				scene->addArrow(cam);
			}
			{
				std::lock_guard<std::mutex> guard(mu);
				if (sendCommand) {
					client::sendCommand(command);
					command = "";
					sendCommand = false;
				}
			}
			processInput(window);
			glfwSwapBuffers(window);
			glfwPollEvents();
			frames++;
		}
#pragma endregion
		delete[] intense;
//		if(newIntense != NULL) delete[] newIntense;
		delete scene;
		glfwTerminate();
		getchar();

		return 0;




	}