#pragma once
#define SV_LOAD_WITH_CODE 0x202
#define SV_LOAD_WITH_FILE 0x203
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
namespace Game {
	class Shader {
	public:
		unsigned int id;
		Shader() {};	
		Shader(const char * vertexPath, const char * fragPath, bool mangled = false) {
			std::string vertexCode;
			std::string fragmentCode;
			std::ifstream vertex;
			std::ifstream fragment;

//			vertex.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//			fragment.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			if (!mangled) {
				try {
					vertex.open(vertexPath);
					fragment.open(fragPath);
					std::stringstream svertex, sfrag;
					svertex << vertex.rdbuf();
					sfrag << fragment.rdbuf();
					vertex.close();
					fragment.close();
					vertexCode = svertex.str();
					fragmentCode = sfrag.str();

				}
				catch (std::ifstream::failure e) {
					printf_s("Error reading shader\n");
				}
			}
			else {
				vertex.open(vertexPath, std::ios::binary);
				int noise, size;
				vertex.read((char*)&noise, sizeof(int));
				vertex.read((char*)&noise, sizeof(int));
				vertex.read((char*)&noise, sizeof(int));
				vertex.read((char*)&size, sizeof(int));
				vertex.read((char*)&noise, sizeof(int));
				char * buffer = new char[size+1];
				vertex.read(buffer, size);
				buffer[size] = '\0';
				vertexCode = buffer;
				delete[] buffer;
				vertex.close();
				vertex.open(fragPath, std::ios::binary);
				vertex.read((char*)&noise, sizeof(int));
				vertex.read((char*)&noise, sizeof(int));
				vertex.read((char*)&noise, sizeof(int));
				vertex.read((char*)&size, sizeof(int));
				vertex.read((char*)&noise, sizeof(int));
				buffer = new char[size+1];
				vertex.read(buffer, size);
				buffer[size] = '\0';
				fragmentCode = buffer;
				delete[] buffer;
			}
			const char * vShader = vertexCode.c_str();
			const char * fShader = fragmentCode.c_str();
			unsigned int vRes, fRes;
			int success;
			char infoLog[512];
			vRes = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vRes, 1, &vShader, NULL);
			glCompileShader(vRes);
			glGetShaderiv(vRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vRes, 512, NULL, infoLog);
				printf_s("Error compiling vertex shader\n %s \n", infoLog);
			};

			fRes = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fRes, 1, &fShader, NULL);
			glCompileShader(fRes);
			glGetShaderiv(fRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fRes, 512, NULL, infoLog);
				printf_s("Error compiling fragment shader\n %s \n", infoLog);
			};

			this->id = glCreateProgram();
			glAttachShader(this->id, vRes);
			glAttachShader(this->id, fRes);
			glLinkProgram(this->id);
			glGetProgramiv(this->id, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(this->id, 512, NULL, infoLog);
				printf_s("Program linking failed\n %s\n", infoLog);
			}

			glDeleteShader(vRes);
			glDeleteShader(fRes);


		}
		Shader(const char * vertexPath, const char * fragmentPath, std::string geometryPath) {
			std::string geomCode, vertCode, fragCode;
			std::ifstream vertext, geometry, fragment;
			vertext.open(vertexPath);
			if (vertext.is_open()) {
				std::stringstream stream;
				stream << vertext.rdbuf();
				vertCode = stream.str();
				vertext.close();
			}
			else {
				printf_s("Error reading vertex shader");
			}
			geometry.open(geometryPath);
			if (geometry.is_open()) {
				std::stringstream stream;
				stream << geometry.rdbuf();
				geomCode = stream.str();
				geometry.close();
			}
			else {
				printf_s("Error reading geometry shader");
			}
			fragment.open(fragmentPath);
			if (fragment.is_open()) {
				std::stringstream stream;
				stream << fragment.rdbuf();
				fragCode = stream.str();
				fragment.close();
			}
			else {
				printf_s("Error reading fragment shader");
			}
			unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
			unsigned int gShader = glCreateShader(GL_GEOMETRY_SHADER);
			unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
			const char * vchar = vertCode.c_str();
			const char * fchar = fragCode.c_str();
			const char * gchar = geomCode.c_str();
			int success;
			char infoLog[512];
			glShaderSource(vShader, 1, &vchar, NULL);
			glCompileShader(vShader);
			glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vShader, 512, NULL, infoLog);
				printf_s("Error compiling vertex shader\n %s\n", infoLog);
			}
			glShaderSource(gShader, 1, &gchar, NULL);
			glCompileShader(gShader);
			glGetShaderiv(gShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(gShader, 512, NULL, infoLog);
				printf_s("Error compiling geometry shader\n %s\n", infoLog);
			}
			glShaderSource(fShader, 1, &fchar, NULL);
			glCompileShader(fShader);
			glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fShader, 512, NULL, infoLog);
				printf_s("Error compiling fragment shader\n %s\n", infoLog);
			}

			this->id = glCreateProgram();
			glAttachShader(id, vShader);
			glAttachShader(id, gShader);
			glAttachShader(id, fShader);
			glLinkProgram(id);
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(id, 512, NULL, infoLog);
				printf_s("Error linking program\n %s\n", infoLog);
			}

			glDeleteShader(vShader);
			glDeleteShader(gShader);
			glDeleteShader(fShader);
		}
		Shader(const char * vertexPath, char * fragmentCode, int code, int code2) {
			std::string vertexCode;
			std::ifstream vertex;
			std::ifstream fragment;

			vertex.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fragment.exceptions(std::ifstream::failbit | std::ifstream::badbit); 
			try {
				vertex.open(vertexPath);
				std::stringstream svertex, sfrag;
				svertex << vertex.rdbuf(); 
				sfrag << fragment.rdbuf();
				vertex.close();
				fragment.close();
				vertexCode = svertex.str();

			}
			catch (std::ifstream::failure e) {
				printf_s("Error reading shader\n");
			}
			const char * vShader = vertexCode.c_str();
			const char * fShader = fragmentCode;
			unsigned int vRes, fRes;
			int success;
			char infoLog[512];
			vRes = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vRes, 1, &vShader, NULL);
			glCompileShader(vRes);
			glGetShaderiv(vRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vRes, 512, NULL, infoLog);
				printf_s("Error compiling vertex shader\n %s \n", infoLog);
			};

			fRes = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fRes, 1, &fShader, NULL);
			glCompileShader(fRes);
			glGetShaderiv(fRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fRes, 512, NULL, infoLog);
				printf_s("Error compiling fragment shader\n %s \n", infoLog);
			};

			this->id = glCreateProgram();
			glAttachShader(this->id, vRes);
			glAttachShader(this->id, fRes);
			glLinkProgram(this->id);
			glGetProgramiv(this->id, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(this->id, 512, NULL, infoLog);
				printf_s("Program linking failed\n %s\n", infoLog);
			}

			glDeleteShader(vRes);
			glDeleteShader(fRes);
		}
		Shader(char * vertexCode, char * fragmentCode, bool c, bool d, bool a) {
			const char * vShader = vertexCode;
			const char * fShader = fragmentCode;
			unsigned int vRes, fRes;
			int success;
			char infoLog[512];
			vRes = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vRes, 1, &vShader, NULL);
			glCompileShader(vRes);
			glGetShaderiv(vRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vRes, 512, NULL, infoLog);
				printf_s("Error compiling vertex shader\n %s \n", infoLog);
			};

			fRes = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fRes, 1, &fShader, NULL);
			glCompileShader(fRes);
			glGetShaderiv(fRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fRes, 512, NULL, infoLog);
				printf_s("Error compiling fragment shader\n %s \n", infoLog);
			};

			this->id = glCreateProgram();
			glAttachShader(this->id, vRes);
			glAttachShader(this->id, fRes);
			glLinkProgram(this->id);
			glGetProgramiv(this->id, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(this->id, 512, NULL, infoLog);
				printf_s("Program linking failed\n %s\n", infoLog);
			}

			glDeleteShader(vRes);
			glDeleteShader(fRes);
		}
		Shader(char * vertexCode, const char * fragmentPath, float d) {
			std::string fragmentCode;
			std::ifstream fragment;

			fragment.exceptions(std::ifstream::failbit | std::ifstream::badbit); 
			try {
				fragment.open(fragmentPath);
				std::stringstream sfrag;
				sfrag << fragment.rdbuf();
				fragment.close();
				fragmentCode = sfrag.str();

			}
			catch (std::ifstream::failure e) {
				printf_s("Error reading shader\n");
			}
			const char * vShader = vertexCode;
			const char * fShader = fragmentCode.c_str();
			unsigned int vRes, fRes;
			int success;
			char infoLog[512];
			vRes = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vRes, 1, &vShader, NULL);
			glCompileShader(vRes);
			glGetShaderiv(vRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vRes, 512, NULL, infoLog);
				printf_s("Error compiling vertex shader\n %s \n", infoLog);
			};

			fRes = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fRes, 1, &fShader, NULL);
			glCompileShader(fRes);
			glGetShaderiv(fRes, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fRes, 512, NULL, infoLog);
				printf_s("Error compiling fragment shader\n %s \n", infoLog);
			};

			this->id = glCreateProgram();
			glAttachShader(this->id, vRes);
			glAttachShader(this->id, fRes);
			glLinkProgram(this->id);
			// print linking errors if any
			glGetProgramiv(this->id, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(this->id, 512, NULL, infoLog);
				printf_s("Program linking failed\n %s\n", infoLog);
			}

			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vRes);
			glDeleteShader(fRes);
		}
		void use()
		{
			glUseProgram(id);
		}
		void setBool(const std::string &name, bool value) const
		{
			glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
		}
		void setInt(const std::string &name, int value) const
		{
			glUniform1i(glGetUniformLocation(id, name.c_str()), value);
		}
		void setFloat(const std::string &name, float value) const
		{
			glUniform1f(glGetUniformLocation(id, name.c_str()), value);
		}
		void setMat4(const std::string &name, glm::mat4 value) const {
			glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
		}
		void setVec3(const std::string &name, glm::vec3 value) const {
			glUniform3f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z);
		}
		

	};
}