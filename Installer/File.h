#pragma once
#include <Windows.h>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#define BINARY 1011
#define TEXT 53
#define STRFY(X) #X
bool isFile(std::string path) {
	struct stat fs;
	if (stat(path.c_str(), &fs) == 0) {
		if (fs.st_mode & S_IFDIR) {
			return false;
		}
		else if (fs.st_mode & S_IFREG) {
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}
bool fileOrFolder(std::string path) {
	struct stat fs;
	if (stat(path.c_str(), &fs) == 0) {
		if (fs.st_mode & S_IFDIR) {
			return true;
		}
		else if (fs.st_mode & S_IFREG) {
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}
std::vector<std::string> getDirectory(std::string path) {
	std::vector<std::string> files;
	WIN32_FIND_DATA fsearch;
	ZeroMemory(&fsearch, sizeof(WIN32_FIND_DATA));
	std::stringstream ss;
	ss << path << "\\*";
	HANDLE handle = FindFirstFile(ss.str().c_str(), &fsearch);
	while (handle != INVALID_HANDLE_VALUE) {
		std::stringstream test;
		test << path << "\\" << fsearch.cFileName;
		if (fileOrFolder(test.str())) {
			files.push_back(fsearch.cFileName);
		}
		if (FindNextFile(handle, &fsearch) == FALSE) break;
	}
	FindClose(handle);
	return files;
}
void getFileSize(std::string filePath, int & fileSize) {
	std::ifstream stream;
	stream.open(filePath, std::ios::binary | std::ios::ate);
	std::streamsize size = stream.tellg();
	fileSize = size;
	stream.close();

}
void loadFile(std::string filePath, char ** data, int fileSize, int flag) {
	std::ifstream reader;
	switch (flag) {
	case BINARY:
	{
		reader.open(filePath, std::ios::binary);
		printf("File size of read: %d \n", fileSize);
		char * file = new char[fileSize];
		reader.rdbuf()->sgetn(file, fileSize);
		reader.close();
		memcpy(*data, file, fileSize);
		delete[] file;
	}
	case TEXT:
		reader.open(filePath);
		std::vector<char> text;
		while (!reader.eof()) {
			text.push_back(reader.get());
		}
		memcpy(*data, text.data(), text.size());
		reader.close();
	}
}
void writeFile(std::string name, char * data, int fileSize) {
	std::ofstream writer;
	std::string newName = name.substr(name.find_last_of('\\') + 1);
	writer.open(newName, std::ios::binary);
	writer.write(data, fileSize);
	writer.close();
}



