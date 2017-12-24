#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Winmm.lib")
#include "gui.h"
#include <windowsx.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#include <fstream>
using namespace gui;
bool accepted = false;
char * eula_string;
int eulaSize;
void eula() {
	std::ifstream in;
	in.open("eula.db", std::ios::binary);
	if (in.is_open()) {
		in.read((char*)&accepted, sizeof(bool));
		if (!accepted) {
			in.read((char*)&eulaSize, sizeof(eulaSize));
			eula_string = new char[eulaSize + 1];
			in.read(eula_string, eulaSize);
			eula_string[eulaSize] = '\0';
		}
	}
}
void agreeEula() {
	std::ofstream out;
	out.open("eula.db", std::ios::binary);
	if (out.is_open()) {
		accepted = true;
		out.write((char*)&accepted, sizeof(bool));
		out.write((char*)&eulaSize, sizeof(int));
		out.write(eula_string, eulaSize);
		out.close();
	}
}
HBITMAP image = NULL;
Page * agreePage;
Page * homePage;
Page * playPage;
Page * settingsPage;
MainPage * main;
char characterSet[] = "abcdefghijklmnopqrstuvwxyz1234567890";
void saveSettings() {
	std::ofstream out;
	out.open("settings.set", std::ios::binary);
	if (out.is_open()) {
		int graphics = ((Radiobutton*)settingsPage->getControl(1))->getChecked();
		out.write((char*)&graphics, sizeof(int));
		bool fullscreen = ((Checkbox*)settingsPage->getControl(3))->isChecked();
		out.write((char*)&fullscreen, sizeof(bool));
		std::string res;
		int index = ((Combobox*)settingsPage->getControl(5))->getSelected(res);
		int resX = atoi(res.substr(0, res.find(' ')).c_str());
		int resY = atoi(res.substr(res.find('x') + 2).c_str());
		out.write((char*)&index, sizeof(int));
		out.write((char*)&resX, sizeof(int));
		out.write((char*)&resY, sizeof(int));
		int pos = ((Slider*)settingsPage->getControl(8))->getPos();
		float volume = pos / 10.0f;
		out.write((char*)&volume, sizeof(float));
		out.close();
	}
}
void defaultSettings() {
	std::ofstream out;
	out.open("settings.set", std::ios::binary);
	if (out.is_open()) {
		int graphics = 2;
		out.write((char*)&graphics, sizeof(int));
		bool fullscreen = 0;
		out.write((char*)&fullscreen, sizeof(bool));
		std::string res = "1920 x 1080";
		int index = 3;
		int resX = atoi(res.substr(0, res.find(' ')).c_str());
		int resY = atoi(res.substr(res.find('x') + 2).c_str());
		out.write((char*)&index, sizeof(int));
		out.write((char*)&resX, sizeof(int));
		out.write((char*)&resY, sizeof(int));
		int pos = 10;
		float volume = pos / 10.0f;
		out.write((char*)&volume, sizeof(float));
		out.close();
	}
}
void readSettings() {
	std::ifstream in;
	in.open("settings.set", std::ios::binary);
	if (in.is_open()) {
		int graphics;
		in.read((char*)&graphics, sizeof(int));
		((Radiobutton*)settingsPage->getControl(1))->setCheck(graphics, true);
		bool fullscreen;
		in.read((char*)&fullscreen, sizeof(bool));
		((Checkbox*)settingsPage->getControl(3))->setCheck(fullscreen);
		int index, buf1, buf2;
		in.read((char*)&index, sizeof(int));
		in.read((char*)&buf1, sizeof(int));
		in.read((char*)&buf2, sizeof(int));
		((Combobox*)settingsPage->getControl(5))->selectChoice(index);
		float volume;
		in.read((char*)&volume, sizeof(float));
		int pos = volume * 10.0f;
		((Slider*)settingsPage->getControl(8))->setPos(pos);
		in.close();


	}
}
char ipCharSet[] = "1234567890.";
void playGame() {
	struct stat buffer;
	if (stat("settings.set", &buffer)) {
		MessageBox(NULL, "Using defaults", "", MB_OK);
		defaultSettings();
	}
	std::ofstream out;
	out.open("login.set", std::ios::binary);
	if (out.is_open()) {
		std::string username = ((TextField*)playPage->getControl(1))->getText();
		std::string ip = ((TextField*)playPage->getControl(3))->getText();
		if (username.size() == 0) {
			MessageBox(NULL, "You must enter a username!", "KFBR", MB_OK | MB_ICONERROR);
			return;
		}
		if (ip.size() == 0) {
			MessageBox(NULL, "You must enter an ip!", "KFBR", MB_OK | MB_ICONERROR);
			return;
		}
		bool valid = true;
		int dots = 0;
		for (char c : ip) {
			bool inset = false;
			for (char letter : ipCharSet) {
				if (c == letter) {
					inset = true;
					if (c == '.') dots++;
					break;
				}
			}
			if (!inset) valid = false;
		}
		if (!valid || dots != 3) {
			MessageBox(NULL, "You must enter a valid ip!", "KFBR", MB_OK | MB_ICONERROR);
			((TextField*)playPage->getControl(3))->setText("");
			return;
		}
		for (char c : username) {
			bool inSet = false;
			for (char letter : characterSet) {
				if (tolower(c) == letter) { inSet = true; break; }
			}
			if (!inSet) {
				MessageBox(NULL, "Username must be alphanumeric", "KFBR", MB_OK | MB_ICONERROR);
				((TextField*)playPage->getControl(1))->setText("");
				return;
			}
		}
		int userSize = username.size();
		int ipSize = ip.size();
		out.write((char*)&userSize, sizeof(int));
		out.write((char*)username.c_str(), userSize);
		out.write((char*)&ipSize, sizeof(int));
		out.write((char*)ip.c_str(), ipSize);
		out.close();

		STARTUPINFO info;
		PROCESS_INFORMATION pinfo;
		SECURITY_ATTRIBUTES attr;
		ZeroMemory(&attr, sizeof(SECURITY_ATTRIBUTES));
		attr.bInheritHandle = FALSE;
		attr.nLength = sizeof(SECURITY_ATTRIBUTES);
		attr.lpSecurityDescriptor = NULL;
		ZeroMemory(&info, sizeof(STARTUPINFO));
		info.cb = sizeof(STARTUPINFO);
		ZeroMemory(&pinfo, sizeof(PROCESS_INFORMATION));
		char file[MAX_PATH];
		sprintf_s(file, MAX_PATH, "%s\\Documents\\KFBR\\3DGame.exe", getenv("USERPROFILE"));
		CreateProcess(file, NULL, &attr, &attr, FALSE, NULL, NULL, NULL, &info, &pinfo);
	}
}
LRESULT CALLBACK WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
	int i = 2;
	switch (message) {
	case WM_CREATE:
		image = (HBITMAP)LoadImage(NULL, "bannerSmall.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		agreePage = new Page("agreePage", { new Button(280, 700, 50, 20, "Agree", handle, []() {agreeEula(); main->navigateTo(1); }),  new TextField(10, 300, 500, 300, eula_string, handle, false, GUI_TEXTFIELD_VERT_SCROLL), new Label(10, 250, "Please agree to the eula:", handle) });
		homePage = new Page("homePage", { new Button(200, 300, 200, 50, "Play", handle, []() {main->navigateTo(2); }), new Button(200, 400, 200, 50, "Settings", handle, []() {main->navigateTo(3); }) });
		playPage = new Page("playPage", { new Label(200, 250, "Username:", handle), new TextField(200, 300, 200, 20, handle, WS_BORDER), new Label(200, 450, "Ip address:", handle), new TextField(200, 500, 200, 20, handle, WS_BORDER),
			new Button(200, 700, 200, 50, "Join", handle, playGame), new Button(20, 300, 100, 20, "Back", handle, []() {main->navigateTo(1); }) });
		settingsPage = new Page("settingsPage", { new Label(10, 250, "Graphics:", handle), new Radiobutton(10, 300, {"Low", "Medium", "High"}, handle), new Button(10, 700, 50, 25, "Back", handle, []() {main->navigateTo(1); }),
			new Checkbox(300, 300, "Fullscreen", handle), new Label(300, 370, "Resolution: ", handle), new Combobox(300, 400, {"1152 x 648", "1280 x 720", "1366 x 768", "1920 x 1080", "2560 x 1440", "3840 x 2160"}, 100, handle, 3),
			new Button(250, 700, 100, 50, "Save", handle, []() {saveSettings(); }), new Label(300, 450, "Music Volume: ", handle), new Slider(300, 500, 150, 40, 0, 10, handle) });
		main = new MainPage({ agreePage, homePage, playPage, settingsPage });
		readSettings();
		if (!accepted) {
			main->navigateTo(0);
		}
		else {
			main->navigateTo(1);
		}
		break;
	case WM_DESTROY:
		DeleteObject(image);
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		BITMAP img;
		PAINTSTRUCT paint;
		ZeroMemory(&paint, sizeof(PAINTSTRUCT));
		ZeroMemory(&img, sizeof(BITMAP));
		HDC hdc = BeginPaint(handle, &paint);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP oldImg = (HBITMAP)SelectObject(hdcMem, image);
		GetObject(image, sizeof(img), &img);
		BitBlt(hdc, 0, 0, img.bmWidth, img.bmHeight, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, oldImg);
		DeleteDC(hdcMem);
		EndPaint(handle, &paint);
	}
	}
	return DefWindowProc(handle, message, wParam, lParam);
}
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND window;
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "KFBRLauncher";
	RECT size = { 0, 0, 600, 800 };
	AdjustWindowRect(&size, WS_OVERLAPPED, FALSE);

	eula();

	RegisterClassEx(&wc);
	window = CreateWindowEx(NULL, "KFBRLauncher", "King Frederick's Battle Royale", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, 300, 300, size.right - size.left, size.bottom - size.top, NULL, NULL, hInstance, NULL);
	ShowWindow(window, nCmdShow);

	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
			main->handleMessage(&msg);
		}
	}
	GUI_CLEANUP(main);
	return 0;
}