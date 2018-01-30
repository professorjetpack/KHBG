#define _CRT_SECURE_NO_WARNINGS
#include "File.h"
#define GUI_VISUAL_STYLES_6
#include "gui.h"
#include <WinNls.h>
#include <ShObjIdl.h>
#include <objbase.h>
#include <ShlGuid.h>
#include <WinInet.h>
#include <atlbase.h>
#include <mutex>
#include <thread>
void removeDirectory(char * directory) {
	char dirPath[MAX_PATH];
	sprintf_s(dirPath, MAX_PATH, "%s/Documents/%s", getenv("USERPROFILE"), directory);
	std::vector<std::string> files;
	files = getDirectory(dirPath);
	char filePath[MAX_PATH];
	for (int i = 0; i < files.size(); i++) {
		sprintf_s(filePath, MAX_PATH, "%s\\%s", dirPath, files[i].c_str());
		remove(filePath);
	}
	RemoveDirectory(dirPath);
}
int unzipTo(BSTR zipFile, BSTR folder) {
	HRESULT result;
	IShellDispatch * pISD;
	Folder * toFolder = NULL;
	IDispatch * pItem;
	VARIANT vDir, vFile, vOpt;
	CoInitialize(NULL);
	result = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void**)&pISD);
	if (SUCCEEDED(result)) {
		VariantInit(&vDir);
		vDir.vt = VT_BSTR;
		vDir.bstrVal = folder;
		result = pISD->NameSpace(vDir, &toFolder);
		if (SUCCEEDED(result)) {
			Folder * fromFolder = NULL;
			VariantInit(&vFile);
			vFile.vt = VT_BSTR;
			vFile.bstrVal = zipFile;
			pISD->NameSpace(vFile, &fromFolder);
			FolderItems * items = NULL;
			fromFolder->Items(&items);
			items->QueryInterface(IID_IDispatch, (void**)&pItem);

			VariantInit(&vOpt);
			vOpt.vt = VT_I4;
			vOpt.lVal = FOF_NO_UI;

			VARIANT newV;
			VariantInit(&newV);
			newV.vt = VT_DISPATCH;
			newV.pdispVal = pItem;//items;

			result = toFolder->CopyHere(newV, vOpt);
			fromFolder->Release();
			toFolder->Release();
			CoUninitialize();
			return 0;
		}
		else {
			pISD->Release();
			CoUninitialize();
			return -1;
		}
	}
	else return -2;
}
gui::MainPage * main;
bool downloadFinish = false;
std::string messages;
std::mutex mu, statusMu;
int status = 0;
bool update = false;
#define STATUS_DOWNLOADING 1
#define STATUS_UNZIPPING 2
void setStatus(int s) {
	std::lock_guard<std::mutex> guard(statusMu);
	status = s;
}
int getStatus() {
	std::lock_guard<std::mutex> guard(statusMu);
	return status;
}
void setFinish() {
	std::lock_guard<std::mutex> guard(mu);
	downloadFinish = true;
}
bool getFinish() {
	std::lock_guard<std::mutex> guard(mu);
	return downloadFinish;
}
void addMessage(char * msg) {
	std::lock_guard<std::mutex> guard(mu);
	update = true;
	messages += msg;
}
std::string getMessages() {
	std::lock_guard<std::mutex> guard(mu);
	update = false;
	return messages;
}
bool isUpdate() {
	std::lock_guard<std::mutex> guard(mu);
	return update;
}
#define printf(msg) (addMessage(msg))
gui::Progress * progress = new gui::Progress();
void download() {
	printf("Thank you for installing King Frederick's Battle Royale by Stephen Verderame \r\n");
	Sleep(2000);
	printf("I hope it will provide you with at least 10 minutes of fun! \r\n");
	Sleep(1500);
	printf("Installation start \r\n");
	struct stat exists;
	char dirPath[MAX_PATH];
	sprintf_s(dirPath, MAX_PATH, "%s\\Documents\\KFBR", getenv("USERPROFILE"));
	if (stat(dirPath, &exists) == 0) {
		//if already downloaded;
		printf("Deleting previous install \r\n");
		removeDirectory("KFBR/Assets/fonts");
		removeDirectory("KFBR/Assets/skybox");
		removeDirectory("KFBR/Assets/sounds");
		removeDirectory("KFBR/Assets/well");
		removeDirectory("KFBR/Assets");
		removeDirectory("KFBR");
		printf("Removed old data! \r\n");
	}
	struct stat safety;
	if (stat(dirPath, &safety) == 0) {
		char assetsPath[MAX_PATH];
		sprintf_s(assetsPath, MAX_PATH, "%s\\Assets", dirPath);
		if (stat(assetsPath, &safety) == 0) {
			RemoveDirectory(assetsPath);
		}
		RemoveDirectory(dirPath);
	}
	char url[] = "https://dl.dropboxusercontent.com/s/cutdr5fss2oorme/KFBR.zip?dl=0";
	char downloadPath[MAX_PATH];
	sprintf_s(downloadPath, MAX_PATH, "%s\\Downloads\\Frederick.zip", getenv("USERPROFILE"));
	if (stat(downloadPath, &safety) == 0) {
		remove(downloadPath);
	}
	printf("Downloading files. This may take a while. \r\n");
	DeleteUrlCacheEntry(url);
	setStatus(STATUS_DOWNLOADING);
	HRESULT check = URLDownloadToFile(NULL, url, downloadPath, 0, progress);
	if (check == S_OK) printf("Downloaded files! \r\n");
	else printf("Download failed \r\n");
	printf("Decompressing files. This may take a while \r\n");
	char folderPath[MAX_PATH];
	sprintf_s(folderPath, MAX_PATH, "%s\\Documents", getenv("USERPROFILE"));
	CComBSTR file(downloadPath);
	CComBSTR folder(folderPath);
	setStatus(STATUS_UNZIPPING);
	if (unzipTo(file, folder) == 0) printf("Unzip success! \r\n");
	else printf("Unzip failed! \r\n");
	setStatus(0);

	char exePath[MAX_PATH];
	sprintf_s(exePath, MAX_PATH, "%s\\Documents\\KFBR\\Launcher.exe", getenv("USERPROFILE"));
	char lnkPath[MAX_PATH];
	sprintf_s(lnkPath, MAX_PATH, "%s\\Desktop\\KFBR.lnk", getenv("USERPROFILE"));
	char icoPath[MAX_PATH];
	sprintf_s(icoPath, MAX_PATH, "%s\\Documents\\KFBR\\bow.ico", getenv("USERPROFILE"));
	CoInitialize(NULL);
	HRESULT res;
	IShellLink * psl;
	res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(res)) {
		IPersistFile * ppf;
		psl->SetPath(exePath);
		psl->SetWorkingDirectory(dirPath);
		psl->SetIconLocation(icoPath, 0);
		res = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(res)) {
			WCHAR formattedPath[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, lnkPath, -1, formattedPath, MAX_PATH);
			ppf->Save(formattedPath, TRUE);
			ppf->Release();
			psl->Release();
			CoUninitialize();
			printf("Desktop shortcut created \r\n");
		}
		else {
			printf("Couldn't create shortcut \r\n");
			psl->Release();
			CoUninitialize();
		}
	}
	else printf("Couldn't create shortcut. Could not instantiate \n");

	printf("Thank you for installing KFBR. Main directory in Documents\\KFBR \r\n");
	printf("If there were any issues with this installation contact me immediatly \r\n");
	printf("Press ok to quit... \r\n");
	setFinish();
}
struct Ball {
	int x;
	int y;
	int yvel;
	int xvel;
} ball{ 200, 200, 7, 7};
struct Paddle {
	int x;
	int y;
} paddle{ 200, 340};
RECT border, wBorder;
clock_t dt;
bool paintPending = false;
PAINTSTRUCT paint;
HBRUSH brush, drawBrush;
HPEN pen;
HDC hdc;
HDC backBuffer;
RECT size;
unsigned int score = 0;
int speed = 0;
bool state = 0;
bool hidden = false;
void collisions() {
	if (ball.x + 12 >= border.right || ball.x - 12 <= border.left) {
		ball.xvel *= -1;		
	}
	if (ball.y - 12 <= border.top) {
		ball.yvel *= -1;
	}
	if (ball.x - 12 >= paddle.x - 50 && ball.x + 12 <= paddle.x + 50 && ball.y + 12 >= paddle.y - 5 && ball.y - 12 <= paddle.y + 5) {
		score += 10;
		ball.yvel *= -1;
		ball.y = paddle.y - 18;
		MessageBeep(MB_ICONHAND);
	}
	if (ball.y + 12 >= border.bottom) {
		ball.y = 200;
		ball.x = 200;
		ball.yvel = -7;
		ball.xvel = 7;
		char msg[50];
		sprintf_s(msg, 50, "Ball Dropped! Your score was: %d", score);
		MessageBeep(MB_ICONWARNING);
//		MessageBox(NULL, msg, "KFBR Installer Game", MB_OK);
		speed = 0;
		state = 0;
	}
	if (paddle.x - 50 <= border.left) paddle.x = border.left + 50;
	else if (paddle.x + 50 >= border.right) paddle.x = border.right - 50;

	if (score == 150 && speed == 0) {
		speed++;
		if (ball.yvel < 0) ball.yvel -= 3;
		else ball.yvel += 3;
		if (ball.xvel < 0) ball.xvel -= 3;
		else ball.xvel += 3;
	}
	else if (score == 220 && speed == 1) {
		speed++;
		if (ball.yvel < 0) ball.yvel -= 3;
		else ball.yvel += 3;
		if (ball.xvel < 0) ball.xvel -= 3;
		else ball.xvel += 3;
	}
	else if (score == 300 && speed == 2) {
		speed++;
		if (ball.yvel < 0) ball.yvel -= 5;
		else ball.yvel += 5;
		if (ball.xvel < 0) ball.xvel -= 5;
		else ball.xvel += 5;
	}
	else if (score == 350 && speed == 3) {
		speed++;
		if (ball.yvel < 0) ball.yvel -= 5;
		else ball.yvel += 5;
		if (ball.xvel < 0) ball.xvel -= 5;
		else ball.xvel += 5;
	}
}
void draw(HDC hdc) {
	FillRect(hdc, &wBorder, (HBRUSH)COLOR_WINDOW);

	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, border.left, border.top, border.right, border.bottom);

	brush = CreateSolidBrush(RGB(255, 0, 0));
	pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	SelectObject(hdc, pen);
	SelectObject(hdc, brush);
	Ellipse(hdc, ball.x - 10, ball.y + 10, ball.x + 10, ball.y - 10);
	DeleteBrush(brush);
	

	brush = CreateSolidBrush(RGB(0, 0, 255));
	SelectObject(hdc, brush);
	SelectObject(hdc, pen);
	Ellipse(hdc, paddle.x - 50, paddle.y - 5, paddle.x + 50, paddle.y + 5);
	DeletePen(pen);
	DeleteBrush(brush);

	if (!state) {
		if (hidden) {
			gui::Label * lbl = (gui::Label*)main->getCurrentPage()->getControl("startMsg");
			lbl->showComponent();
			hidden = false;
		}
		if (GetAsyncKeyState(VK_SPACE) < 0) {
			score = 0;
			state = 1;
		}
		return;
	}
	if (!hidden) {
		gui::Label * lbl = (gui::Label*)main->getCurrentPage()->getControl("startMsg");
		lbl->hideComponent();
		hidden = true;
	}
	if (GetAsyncKeyState('A') < 0) paddle.x -= 10;
	else if (GetAsyncKeyState('D') < 0) paddle.x += 10;

	ball.x += ball.xvel;
	ball.y += ball.yvel;

	collisions();


//	BitBlt(hdc, 0, 0, size.right - size.left, size.bottom - size.top, hdc, 0, 0, SRCCOPY);
//	DeleteDC(hdc);
}
bool enabled = false;
bool setMarquee = false;
LRESULT CALLBACK WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
	{
		border.bottom = 400; border.left = 20; border.top = 20; border.right = 760;
		wBorder.bottom = 420; wBorder.left = 0; wBorder.top = 0; wBorder.right = 800;
		gui::GUI::bindWindow(handle);
		gui::Page * drawPage = new gui::Page("p1", { new gui::Label("score", 700, 10, "Score: "), new gui::Label("startMsg", 350, 150, "Press SPACE to play installer game"), new gui::TextField("txt", 20, 500, 740, 150, (char*)messages.c_str(), false, GUI_TEXTFIELD_VERT_SCROLL), new gui::Button("ok", 350, 700, 100, 50, "OK", []() {PostQuitMessage(0); }),
			new gui::Progressbar("progressBar", 50, 450, 680, 25, 0, 100)});
		main = new gui::MainPage({ drawPage });
		main->navigateTo("p1");
		main->getCurrentPage()->getControl("ok")->disableControl();
//		EnableMenuItem(GetSystemMenu(handle, FALSE), SC_CLOSE, MF_GRAYED);
		return 0;
	}
	case WM_CLOSE:
		if (!getFinish()) {
			if (MessageBox(handle, "Your download is not complete. Are you sure you want to exit?", "KFBR Installer", MB_ICONQUESTION | MB_YESNO) == IDNO) {
				return 0;
			}
		}
		DestroyWindow(handle);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
	{
		if (!paintPending) {
			paintPending = true;
			InvalidateRect(handle, &wBorder, false);
			UpdateWindow(handle);
		    ValidateRect(handle, &wBorder);
		}
		if (getFinish() && !enabled) {
			enabled = true;
			main->getCurrentPage()->getControl("ok")->enableControl();

//			gui::Progressbar* bar = (gui::Progressbar*)main->getCurrentPage()->getControl("progressBar");
//			bar->hideComponent();
		}
		if (getStatus() == STATUS_DOWNLOADING) {
			double denomenator = progress->progressMax == 0 ? 1.0 : (double)progress->progressMax;
			gui::Progressbar* bar = (gui::Progressbar*)main->getCurrentPage()->getControl("progressBar");
			bar->setPos((progress->progress / denomenator) * 100.0);
		}
		else if (setMarquee == false && getStatus() == STATUS_UNZIPPING) {
			setMarquee = true;
			gui::Progressbar* bar = (gui::Progressbar*)main->getCurrentPage()->getControl("progressBar");

			bar->setMarquee(true);
		}
		else if(getStatus() == 0 && setMarquee){
			setMarquee = false;
			gui::Progressbar* bar = (gui::Progressbar*)main->getCurrentPage()->getControl("progressBar");
			bar->setMarquee(false);
			bar->setPos(100);
		}
		if (isUpdate()) {
			gui::TextField * field = (gui::TextField*)main->getCurrentPage()->getControl("txt");
			field->setText((char*)getMessages().c_str());
		}
		return 0;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(handle, &paint);
		draw(hdc);
		EndPaint(handle, &paint);
		gui::Label * lbl = (gui::Label*)main->getCurrentPage()->getControl("score");
		char scoreTxt[20];
		sprintf_s(scoreTxt, 20, "Score: %d", score);
		lbl->setText(scoreTxt);
		paintPending = false;
		return 0;
	}
	}
	return DefWindowProc(handle, message, wParam, lParam);
}
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND window;
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "KFBRInstaller";
	size = { 0, 0, 800, 800 };
	AdjustWindowRect(&size, WS_OVERLAPPED, FALSE);

	
	RegisterClassEx(&wc);
	window = CreateWindowEx(NULL, "KFBRInstaller", "King Frederick's Battle Royale Installer", WS_OVERLAPPEDWINDOW exclude WS_MAXIMIZEBOX exclude WS_THICKFRAME, 300, 300, size.right - size.left, size.bottom - size.top, NULL, NULL, hInstance, NULL);
	ShowWindow(window, nCmdShow);

	std::thread t(download);
	t.detach();

	MSG msg;
	hdc = GetDC(window);
	backBuffer = CreateCompatibleDC(hdc);
	SetTimer(window, NULL, 1000 / 60, NULL);
	gui::Progressbar * bar = (gui::Progressbar*)main->getCurrentPage()->getControl("progressBar");
	gui::TextField * field = (gui::TextField*)main->getCurrentPage()->getControl("txt");
	MessageBox(window, "Thank you for installing King Frederick's Battle Royale! You may play this game as you wait for the install!", "KFBR Installer", MB_OK);
	while (true) {
		clock_t time = clock();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
			main->handleMessage(&msg);
		}
		dt = clock() - time;
	}
	delete progress;
	ReleaseDC(window, hdc);
	DeleteDC(backBuffer);
	GUI_CLEANUP(main);
}