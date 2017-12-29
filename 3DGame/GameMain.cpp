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
	double deltaTime = 0, lastTime = 0;
	Camera cam;
	Scene * scene;
	unsigned int quadVAO = 0;
	unsigned int quadVBO;
	unsigned int textVAO, textVBO;
	bool firstPress = true;
	double healTime = 0, shootTime = 0;
	bool healable = false, suppliable = false;
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
	int SCR_WIDTH, SCR_HEIGHT;
	int graphics;
	bool fullscreen;
	float volume;
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
		USE_SHADER(s.id);
		SHADER_SET_VEC3(s.id, "textColor", color);
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
		if (scene->player.isBlack() || typing) return;
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
		if (scene->player.isBlack() || typing) return;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			if (healable) {
				if (glfwGetTime() > healTime + 1) {
					scene->player.health += 2;
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
		if (winner || scene->player.isBlack() || typing) return;
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
	void lowerVolume(byte * data, int size, double percent) {
		int sampleCount = (size - 40) / sizeof(int16_t);
		int16_t * samp = (int16_t *)&(data[44]);
		for (int i = 0; i < sampleCount; i++) {
			samp[i] = (int16_t)(samp[i] * percent);
		}
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
	int readSettings() {
		std::ifstream in;
		in.open("settings.set", std::ios::binary);
		if (in.is_open()) {
			in.read((char*)&graphics, sizeof(int));
			in.read((char*)&fullscreen, sizeof(bool));
			int buf;
			in.read((char*)&buf, sizeof(int));
			in.read((char*)&SCR_WIDTH, sizeof(int));
			in.read((char*)&SCR_HEIGHT, sizeof(int));
			in.read((char*)&volume, sizeof(float));
			in.close();

//			graphics = 0;
			printf("Graphics: %d \n", graphics);
			printf("Fullscreen: %d \n", fullscreen);
			printf("Width: %d Height %d\n", SCR_WIDTH, SCR_HEIGHT);
			printf("Volume: %f \n", volume);

		}
		else {
			MessageBox(NULL, "Error reading settings!", "KFBR", MB_OK | MB_ICONERROR);
			return 1;
		}
		return 0;
	}
	char * username;
	char * ip;
	int readLogin() {
		std::ifstream in;
		in.open("login.set", std::ios::binary);
		if (in.is_open()) {
			int size;
			in.read((char*)&size, sizeof(int));
			username = new char[size + 1];
			in.read(username, size);
			username[size] = '\0';
			in.read((char*)&size, sizeof(int));
			ip = new char[size + 1];
			in.read(ip, size);
			ip[size] = '\0';
			printf("Read user: %s  Read ip: %s \n", username, ip);
			in.close();

		}
		else {
			MessageBox(NULL, "Error reading login.set!", "KFBR", MB_OK | MB_ICONERROR);
			return 1;
		}
		return 0;
	}
#pragma endregion
//	int main() {
	int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow){
		printf("Console on \n");
		if(readSettings() != 0) return 3;
		if(readLogin() != 0) return 4;
		FILE * feula;
		fopen_s(&feula, "eula.db", "rb");
		bool accepted;
		if (feula != NULL) {
			fread((char*)&accepted, sizeof(bool), 1, feula);
			fclose(feula);
		}
		if (!accepted || feula == NULL) {
			MessageBox(NULL, "You must agree to the eula!", "KFBR", MB_OK | MB_ICONERROR);
			return 2;
		}
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		//this pc supports 4.4, set it to 3.3, the earliest version we can support to allow more ppl to play it
		if (graphics == 2)
			glfwWindowHint(GLFW_SAMPLES, 4);
		else if(graphics == 1)
			glfwWindowHint(GLFW_SAMPLES, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); need this line in case of need to compile for Mac
		GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KFBR Game", NULL, NULL);
		if (fullscreen) {
			printf("Fullscreen \n");
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, SCR_WIDTH, SCR_HEIGHT, GLFW_DONT_CARE);
		}
		if (window == NULL) {
			MessageBox(NULL, "Failed to initialize window", "KFBR", MB_OK | MB_ICONERROR);
			printf("Failed to create window\n");
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			MessageBox(NULL, "Failed to initialize gl", "KFBR", MB_OK | MB_ICONERROR);
			glfwTerminate();
			return -2;
		}
		glfwSetFramebufferSizeCallback(window, changeSizeCallback);
		glfwSetCursorPosCallback(window, mouseMoveCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		if(graphics)
			glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_PROGRAM_POINT_SIZE);
		
		Shader depthShader("depth.glslbin", "depth.fragbin", true);
		Shader * shader = new Shader("Shader.glslbin", "Shader.fragbin", true);
		Shader skyBoxShader("sky.glslbin", "sky.fragbin", true);
		Shader hdrFrame("fbo.glslbin", "fbo.fragbin", true);
		Skybox sky("sandcastle");
		float red = 1.5;
		float clearTimes = 1;

		irrklang::ISoundEngine * mainEngine = irrklang::createIrrKlangDevice();
#pragma region FrameBuffers
		unsigned int SHADOW_WIDTH = 8560, SHADOW_HEIGHT = 8560;
		if (graphics == 1 || graphics == 0) {
			SHADOW_WIDTH = 2560; SHADOW_HEIGHT = 2560;
		}
		unsigned int depthMatFramebuffer, depthTexture;
//		if (graphics) {
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
//		}

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
		USE_SHADER(hdrFrame.id);
		SHADER_SET_INT(hdrFrame.id, "hdrBuffer", 0);
		SHADER_SET_INT(hdrFrame.id, "text", 1);
		USE_SHADER(shader->id);
		if (graphics) {
			SHADER_SET_INT(shader->id, "shadowMap", 0);
			SHADER_SET_INT(shader->id, "diffuseTex", 1);
			SHADER_SET_INT(shader->id, "normalMap", 2);
			SHADER_SET_INT(shader->id, "specularMap", 3);
		}
		else {
			SHADER_SET_INT(shader->id, "diffuseTex", 1);
		}
		float near_plane = 30.0f, far_plane = 120.0f;
		SHADER_SET_FLOAT(shader->id, "near_plane", near_plane);
		SHADER_SET_FLOAT(shader->id, "far_plane", far_plane);
		client::startup(ip);
		client::sendName(username);
		client::startLoop();
		scene = sceneFactory(0);


		createBuffers();
		loadFont("IMMORTAL.ttf");
		byte * intense;
		byte * newIntense;
		unsigned int intenseSize;
		loadSound("Assets/sounds/timeLeft.wav", &intense, intenseSize);
		lowerVolume(intense, intenseSize, volume);
		bool once = false;
		bool heartBeat = false;
		float frames = 1;
		double fpsChange = 0;
#pragma region GameLoop
		double time;
		USE_SHADER(depthShader.id);
		glm::mat4 lightProj, lightView, lightSpaceTransform;
		lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
		lightView = glm::lookAt(sunPos, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceTransform = lightProj * lightView;
		float max;
		switch (graphics) {
		case 2:
			max = 200.0f;
			break;
		case 1:
			max = 75.0f;
			break;
		case 0:
			max = 45.0f;
		}
		glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, max);
		USE_SHADER(skyBoxShader.id);
		SHADER_SET_MAT4(skyBoxShader.id, "projection", projection);
		USE_SHADER(shader->id);
		SHADER_SET_MAT4(shader->id, "projection", projection);
		if (graphics) {
			SHADER_SET_VEC3(shader->id, "lightPos", sunPos);
			SHADER_SET_MAT4(shader->id, "lightSpaceMatrix", lightSpaceTransform);
			if (graphics == 1) SHADER_SET_BOOL(shader->id, "medium", true);
		}
		else {
			SHADER_SET_BOOL(shader->id, "low", true);
		}
		glm::mat4 hdrProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
		while (!glfwWindowShouldClose(window)) {
			if (client::getServerTime(time) != 0) {
				time = glfwGetTime();
			}
			deltaTime = time - lastTime;
			lastTime = time;
			client::startLoop();			
//			if (graphics) {
			USE_SHADER(depthShader.id);
			SHADER_SET_MAT4(depthShader.id, "lightSpaceMat", lightSpaceTransform);
				glClearColor(0.f, 0.f, 0.f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				glBindFramebuffer(GL_FRAMEBUFFER, depthMatFramebuffer);
				glClear(GL_DEPTH_BUFFER_BIT);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, depthTexture);
				if (graphics != 0) {
					scene->renderScene(depthShader, true, cam, deltaTime, graphics);
				}
//			}
			glBindFramebuffer(GL_FRAMEBUFFER, hdrFrameBuffer);
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glm::mat4 view = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
			USE_SHADER(skyBoxShader.id);
			SHADER_SET_MAT4(skyBoxShader.id, "view", glm::mat4(glm::mat3(view)));
			sky.draw();
			USE_SHADER(shader->id);
			SHADER_SET_MAT4(shader->id, "view", view);
			if(graphics)
				SHADER_SET_VEC3(shader->id, "viewPos", cam.Position);
//			if (graphics) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, depthTexture);
//			}
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, groundTexture);
			scene->renderScene(*shader, false, cam, deltaTime, graphics, view, projection);
//			printf("Pos: %f, %f, %f \n", cam.Position.x, cam.Position.y, cam.Position.z);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClear(GL_COLOR_BUFFER_BIT);
			USE_SHADER(hdrFrame);
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
				PlaySound((LPCSTR)intense, NULL, SND_MEMORY | SND_ASYNC);
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
			SHADER_SET_FLOAT(hdrFrame, "exposure", 1.2f);
			SHADER_SET_BOOL(hdrFrame, "damage", scene->player.getDamage());
			if (scene->player.health<= 40) {
				if (!heartBeat) 
					mainEngine->play2D("Assets/sounds/heartbeat.wav", true); heartBeat = true;
			}
			else {
				if (heartBeat)
					heartBeat = false; mainEngine->stopAllSounds();
			}
			SHADER_SET_INT(hdrFrame, "health", scene->player.health);
			SHADER_SET_BOOL(hdrFrame, "shake", scene->player.getShake());
			SHADER_SET_FLOAT(hdrFrame, "shakeTime", scene->player.getShakeTime());
			SHADER_SET_BOOL(hdrFrame, "dead", scene->player.isBlack());
			if (scene->player.isBlack()) arrows = 50;

			SHADER_SET_MAT4(hdrFrame, "projection", hdrProjection);
			if (scene->player.isClearing()) {
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
			SHADER_SET_BOOL(hdrFrame, "clearUp", scene->player.isClearing());
			SHADER_SET_VEC4(hdrFrame, "red", glm::vec4(red, 1.f, 1.f, 1.f));
			char msg[50];
			sprintf_s(msg, 50, "Health: %d", scene->player.health);
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
				if (M_DISTANCE(glm::vec3(0, -2, 20), cam.Position + cam.Front) < 1 && scene->player.health < 84) {
					healable = true;
					suppliable = false;
					renderText(hdrFrame, "Hold e to heal at well", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
				}
				else if ((M_DISTANCE_SQUARED(glm::vec3(-9, -2, 20), cam.Position + cam.Front) < 2.89 || M_DISTANCE_SQUARED(glm::vec3(19, -2, -10), cam.Position + cam.Front) < 2.89)
					&& arrows < 50) {
					suppliable = true;
					healable = false;
					renderText(hdrFrame, "Hold e to resuply at fort", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
				}
				else {
					healable = false; suppliable = false;
				}
				if (M_DISTANCE_SQUARED(glm::vec3(0.038, 0.83, 5.87), cam.Position + cam.Front) < 2.89) {
					renderText(hdrFrame, "Press e to enter castle", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('E') < 0) { cam.Position = glm::vec3(-0.128, 1.3, 3.49); cam.oldPos = cam.Position; }
				}
				else if (M_DISTANCE_SQUARED(glm::vec3(1.43, 1.36, 2.9), cam.Position + cam.Front) < 2.25) {
					renderText(hdrFrame, "Press f to climb tower", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) { cam.Position = glm::vec3(2.169, 6.44, 4.02); cam.oldPos = cam.Position; }
				}
				else if (M_DISTANCE_SQUARED(glm::vec3(-2.68, 1.6, 3.1), cam.Position + cam.Front) < 2.25) {
					renderText(hdrFrame, "Press f to climb tower", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) { cam.Position = glm::vec3(-3.16, 8.8, -4.3); cam.oldPos = cam.Position; }
				}
				else if (M_DISTANCE_SQUARED(glm::vec3(0.77, 1.7, 1.12), cam.Position + cam.Front) < 2.25) {
					renderText(hdrFrame, "Press f to call doctor", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) scene->player.health = 120;
				}
				else if (M_DISTANCE_SQUARED(glm::vec3(-33, -2, 20), cam.Position + cam.Front) < 2.89) {
					renderText(hdrFrame, "Press f to climb tower", SCR_WIDTH / 3, SCR_HEIGHT / 2, SCR_WIDTH / 1920.0, glm::vec3(1));
					if (GetKeyState('F') < 0) { cam.Position = glm::vec3(-29, 6.2, 20); cam.oldPos = cam.Position; }
				}
				if (typing) {
					std::lock_guard<std::mutex> guard(mu);
					std::string buffer = "/";
					buffer.append(command);
					renderText(hdrFrame, buffer, SCR_WIDTH / 20.0, SCR_HEIGHT / 12.0, SCR_HEIGHT / 2160.0, glm::vec3(1));
				}
				char * name;
				if (client::getName(&name) == 0) {
					renderText(hdrFrame, name, SCR_WIDTH / 20.0, SCR_HEIGHT / 20.0, SCR_HEIGHT / 1080.0, glm::vec3(1, 1, 0));
					delete[] name;
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
			SHADER_SET_BOOL(hdrFrame, "mainQuad", true);
			renderQuad();
			SHADER_SET_BOOL(hdrFrame, "mainQuad", false);
			if (addArrow) {
				addArrow = false;
				scene->addArrow(cam);
			}
			{
				std::lock_guard<std::mutex> guard(mu);
				if (sendCommand && command.size() > 0) {
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
		delete shader;
		delete[] ip;
		delete[] username;
		delete[] intense;
//		if(newIntense != NULL) delete[] newIntense;
		delete scene;
		glfwTerminate();
		getchar();

		return 0;

	}