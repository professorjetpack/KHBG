#pragma once
#ifdef GUI_VISUAL_STYLES_6
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <initializer_list>
namespace gui {
#define GUI_PARAM_AND |
#define GUI_PARAM_EXCLUDE ^
#ifndef GUI_MACRO_DESC_ONLY
#define and GUI_PARAM_AND
#define exclude GUI_PARAM_EXCLUDE
#endif //GUI_MACRO_DESC_ONLY
	namespace {
		std::string loadFromTextFileToWin32ControlTextFormat(char * file) {
			std::ifstream input;
			input.open(file);
			std::string buffer = "";
			std::string fileText = "";
			if (input.is_open()) {
				while (std::getline(input, buffer)) {
					fileText += buffer += "\r\n";
				}
				input.close();
			}
			return fileText;
		}
	}
	class AbstractionException : public std::exception {
		const char * what() const {
			return "Attempting to break gui abstraction and type safety. Consider using a different control or define GUI_SECURE_NO_WARNINGS";
		}
	};
	struct Dimensions {
		int x;
		int y;
		int width;
		int height;
	};
	class GUI {
	private:
		GUI() {};
		static HWND window;
		static int _width;
		static int _height;
	public:
		inline static void bindWindow(HWND wnd) { window = wnd; }
		inline static HWND useWindow() { return window; }
		inline static char * textFieldLoadFromFile(char * file) { return (char*)loadFromTextFileToWin32ControlTextFormat(file).c_str(); }
		inline static void setDimensions(int width, int height) {
			_width = width;
			_height = height;
		}
		inline static std::pair<int, int> getDimensions() {
			return std::make_pair(_width, _height);
		}
	};
	class Progress : public IBindStatusCallback {
	public:
		ULONG progress;
		ULONG progressMax;
	public:
		Progress() { progress = 0; progressMax = 0; };
		~Progress() {};
		STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText) {
			progress = ulProgress;
			progressMax = ulProgressMax;
			return S_OK;
		}
		STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR *pformatetc, STGMEDIUM __RPC_FAR *pstgmed)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding __RPC_FAR *pib)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetPriority)(LONG __RPC_FAR *pnPriority)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnLowResource)(DWORD reserved)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetBindInfo)(DWORD __RPC_FAR *grfBINDF, BINDINFO __RPC_FAR *pbindinfo)
		{
			return E_NOTIMPL;
		}


		STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown __RPC_FAR *punk)
		{
			return E_NOTIMPL;
		}

		// IUnknown stuff
		STDMETHOD_(ULONG, AddRef)()
		{
			return 0;
		}

		STDMETHOD_(ULONG, Release)()
		{
			return 0;
		}

		STDMETHOD(QueryInterface)(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
		{
			return E_NOTIMPL;
		}
	};
	class Control {
	protected:
		HWND handle;
		HWND parent;
		bool selfHandle;
		std::string name;
		Dimensions dimensions;
	protected:
		inline void setStyle(int params) {
			SetWindowLongPtr(handle, GWL_STYLE, params);
		}
	public:
		Control() : selfHandle(false) {}
		~Control() { DestroyWindow(handle); }
		inline virtual void hideComponent() {
			ShowWindow(handle, SW_HIDE);
		}
		inline virtual void showComponent() {
			ShowWindow(handle, SW_SHOW);
		}
		inline virtual void disableControl() {
			EnableWindow(handle, FALSE);
		}
		inline virtual void enableControl() {
			EnableWindow(handle, TRUE);
		}
		inline virtual bool msgFromControl(MSG * msg) {
			return msg->hwnd == handle;
		}
		inline void setPos(int x, int y, int width, int height) {
			MoveWindow(handle, x, y, width, height, TRUE);
			dimensions = { x, y, width, height };
		}
		inline bool doesDefaultMsgHandle() { return selfHandle; }
		virtual void handleMsg(MSG * msg) {};
		inline void addStyle(int params) {
			SetWindowLongPtr(handle, GWL_STYLE, GetWindowLongPtr(handle, GWL_STYLE) and params);
		}
		inline Dimensions getDimensions() { return dimensions; }
		inline HWND getHandle() { return handle; }
		inline std::string getName() { return name; }
	};
#define GUI_BUTTON_LIGHT_BORDER (BS_DEFPUSHBUTTON)
	class Button : public Control {
	private:
		void(*onClick)();
	protected:
		void init() {
			handle = CreateWindow("BUTTON", "", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 0, 0, 100, 50, parent, NULL, NULL, NULL);
			selfHandle = false;
		}
	public:
		Button(char * id, int x, int y, int width, int height, char * title, void(*onClick)() = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			parent = window;
			name = id; dimensions = {x, y, width, height};
			if (onClick != NULL) {
				selfHandle = true;
				this->onClick = onClick;
			}
			if (params & GUI_BUTTON_LIGHT_BORDER) {
				handle = CreateWindow("BUTTON", title, WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
			}
			else
				handle = CreateWindow("BUTTON", title, WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
		}
		Button() {
			init();
		}
		inline void setClick(void(*click)()) {
			selfHandle = true;
			onClick = click;
		}
		void handleMsg(MSG * msg) {
			if (msg->hwnd != handle || msg->message != WM_LBUTTONDOWN) return;
			Sleep(100);
			onClick();
		}
	};
#define GUI_TEXTFIELD_VERT_SCROLL (ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL)
#define GUI_TEXTFIELD_HOR_SCROLL (ES_AUTOHSCROLL | WS_HSCROLL)
#define GUI_TEXTFIELD_MULTILINE (ES_MULTILINE)
#define GUI_TEXTFIELD_PASSFIELD (ES_PASSWORD)
	class TextField : public Control {
	protected:
		void init() {
			handle = CreateWindow("EDIT", "", WS_CHILD, 0, 0, 100, 50, parent, NULL, NULL, NULL);
		}
	public:
		TextField(char * id, int x, int y, int width, int height, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = {x, y, width, height};
			parent = window;
			handle = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
		}
		TextField(char * id, int x, int y, int width, int height, char * text, bool typing = true, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = {x, y, width, height};
			parent = window;
			if (!typing) selfHandle = true;
			handle = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
			Edit_SetText(handle, text);
		}
		TextField() {
			init();
		}
		void disableTyping() {
			selfHandle = true;
		}
		void enableTyping() {
			selfHandle = false;
		}
		void handleMsg(MSG * msg) {
			if (msg->message == WM_LBUTTONDOWN && msg->hwnd == handle)
				SetFocus(parent);

		}
		void setPasswordChar(char echo) {
			SendMessage(handle, EM_SETPASSWORDCHAR, echo, NULL);
		}
		void setText(char * text) {
			SetWindowText(handle, text);
		}
		std::string getText() {
			size_t size = GetWindowTextLength(handle);
			char * text = new char[size + 1];
			GetWindowText(handle, text, size + 1);
			std::string ret(text);
			delete[] text;
			return ret;
		}
	};
	class Label : public Control {
	protected:
		void init() {
			handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 100, 50, parent, NULL, NULL, NULL);
			SetWindowText(handle, "Label:");
		}
	public:
		Label(char * id, int x, int y, char * text, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; 
			dimensions = {x, y, (int)strlen(text) * 10, 20};
			parent = window;
			handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | params, x, y, strlen(text) * 10, 20, parent, NULL, NULL, NULL);
			SetWindowText(handle, text);
		}
		Label(char * id, int x, int y, int width, int height, char * text, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; 
			dimensions = {x, y, width, height};
			parent = window;
			handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
			SetWindowText(handle, text);
		}
		Label() {
			init();
		}
		void setText(char * text) {
			SetWindowText(handle, text);
		}

	};
	class Radiobutton : public Control {
	private:
		std::vector<HWND> handles;
		int lastY;
		int X;
	public:
		Radiobutton(char * id, int x, int y, char * text, int params = NULL, HWND window = GUI::useWindow()) : lastY(y), X(x) {
			name = id; 
			selfHandle = true;
			parent = window;
			handle = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | params, x, y, (strlen(text) * 10) + 30, 20, window, NULL, NULL, NULL);
			handles.push_back(handle);
			setCheck(0, true);
		}
		~Radiobutton() {
			for (int i = 0; i < handles.size(); i++) {
				DestroyWindow(handles[i]);
			}
		}
		void addChoice(char * text, int params = NULL) {
			lastY += 30;
			HWND handle = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | params, X, lastY, (strlen(text) * 10) + 30, 20, parent, NULL, NULL, NULL);
			handles.push_back(handle);
		}
		Radiobutton(char * id, int x, int y, std::initializer_list<char*> choices, int params = NULL, HWND window = GUI::useWindow()) : lastY(y), X(x) {
			name = id; 
			std::vector<char*> titles;
			for (char * text : choices) {
				titles.push_back(text);
			}
			selfHandle = true;
			parent = window;
			handle = CreateWindow("BUTTON", titles[0], WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | params, x, y, (strlen(titles[0]) * 10) + 30, 20, window, NULL, NULL, NULL);
			handles.push_back(handle);
			for (int i = 1; i < titles.size(); i++) {
				addChoice(titles[i], params);
			}
		}
		void handleMsg(MSG * msg) {
			bool cont = false;
			int caller = 0;
			for (int i = 0; i < handles.size(); i++) {
				if (msg->hwnd == handles[i]) {
					cont = true;
					caller = i;
				}
			}
			if (!cont) return;
			if (msg->message == WM_LBUTTONDOWN) {
				if (SendMessage(msg->hwnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
					SendMessage(msg->hwnd, BM_SETCHECK, BST_CHECKED, 1);
					for (int i = 0; i < handles.size(); i++) {
						if (i == caller) continue;
						SendMessage(handles[i], BM_SETCHECK, BST_UNCHECKED, 1);
					}
				}
			}
		}
		bool msgFromControl(MSG * msg) {
			for (int i = 0; i < handles.size(); i++) {
				if (msg->hwnd == handles[i]) return true;
			}
			return false;
		}
		void hideComponent() {
			for (int i = 0; i < handles.size(); i++) {
				ShowWindow(handles[i], SW_HIDE);
			}
		}
		void showComponent() {
			for (int i = 0; i < handles.size(); i++) {
				ShowWindow(handles[i], SW_SHOW);
			}
		}
		int getChecked() {
			for (int i = 0; i < handles.size(); i++) {
				if (SendMessage(handles[i], BM_GETCHECK, 0, 0) == BST_CHECKED)
					return i;
			}
		}
		void setCheck(int id, bool isChecked) {
			if (isChecked) {
				SendMessage(handles[id], BM_SETCHECK, BST_CHECKED, 1);
			}
			else {
				SendMessage(handles[id], BM_SETCHECK, BST_UNCHECKED, 1);
			}
		}
	};
	class Checkbox : public Control {
	private:
		void(*onClick)();
	public:
		Checkbox(char * id, int x, int y, char * text, void(*click)() = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; 
			dimensions = {x, y, 30 + ((int)strlen(text) * 10), 20};
			parent = window;
			handle = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | params, x, y, 30 + (strlen(text) * 10), 20, window, NULL, NULL, NULL);
			if (click != NULL) {
				selfHandle = true;
				onClick = click;
			}
		}
		bool isChecked() {
			return SendMessage(handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
		}
		void setCheck(bool check) {
			if (check) SendMessage(handle, BM_SETCHECK, BST_CHECKED, 1);
			else SendMessage(handle, BM_SETCHECK, BST_UNCHECKED, 1);
		}
		void handleMsg(MSG * msg) {
			if (msg->hwnd != handle || msg->message != WM_LBUTTONDOWN) return;
			if(onClick != NULL) onClick();
		}
	};
	class Combobox : public Control {
	private:
		void(*onSelect)(std::string, int);
		std::string selected;
		int selectedIndex;
	public:
		Combobox(char * id, int x, int y, int width, int height, void(*select)(std::string, int) = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = {x, y, width, height};
			parent = window;
			selfHandle = true;
			onSelect = select;
			handle = CreateWindow("COMBOBOX", "Test", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE | params, x, y, width, height, window, NULL, NULL, NULL);
		}
		inline void addChoice(char * choice) {
			SendMessage(handle, CB_ADDSTRING, 0, (LPARAM)choice);
		}
		Combobox(char * id, int x, int y, std::initializer_list<char*> list, int width, int defaultSelection = 0,
			void(*select)(std::string, int) = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; 
			dimensions = {x, y, width, (int)list.size() * 50};
			parent = window;
			onSelect = select;
			handle = CreateWindow("COMBOBOX", "Test", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | params, x, y, width, list.size() * 50, window, NULL, NULL, NULL);
			std::vector<char*> items;
			for (char * item : list) {
				items.push_back(item);
				addChoice(item);
			}
			SendMessage(handle, CB_SETCURSEL, defaultSelection, 0);
		}
		inline void selectChoice(int choice) {
			SendMessage(handle, CB_SETCURSEL, choice, 0);
		}
		int getSelected(std::string & selectedName) {
			int index = SendMessage(handle, CB_GETCURSEL, 0, 0);
			char listItem[256];
			SendMessage(handle, CB_GETLBTEXT, index, (LPARAM)listItem);
			selectedName = listItem;
			return index;
		}
		void handleMsg(MSG * msg) {
			if (msg->hwnd == handle && msg->message == WM_COMMAND && HIWORD(msg->wParam) == CBN_SELCHANGE) {
				std::string name;
				int index = getSelected(name);
				onSelect(name, index);
			}
		}
	};
	class Slider : public Control {
	public:
		Slider(char * id, int x, int y, int width, int height, int g_min, int g_max, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = {x, y, width, height};
			handle = CreateWindow(TRACKBAR_CLASS, "", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | params, x, y, width, height, window, NULL, NULL, NULL);
			SendMessage(handle, TBM_SETRANGE, TRUE, MAKELONG(g_min, g_max));
			SendMessage(handle, TBM_SETPOS, TRUE, g_max);

		}
		inline void setPos(int pos) {
			SendMessage(handle, TBM_SETPOS, TRUE, pos);
		}
		inline int getPos() {
			return SendMessage(handle, TBM_GETPOS, 0, 0);
		}
		inline void setRange(int g_min, int g_max) {
			SendMessage(handle, TBM_SETRANGE, TRUE, MAKELONG(g_min, g_max));
		}
		inline void setStep(int interval) {
			SendMessage(handle, TBM_SETTICFREQ, interval, 0);
		}
	};
#define GUI_PROGRESSBAR_MARQUEE (PBS_MARQUEE)
	class Progressbar : public Control {
	public:
		Progressbar(char * id, int x, int y, int width, int height, int g_min, int g_max, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; 
			dimensions = {x, y, width, height};
			handle = CreateWindow(PROGRESS_CLASS, "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, window, NULL, NULL, NULL);
			if(!(params & GUI_PROGRESSBAR_MARQUEE))
				SendMessage(handle, PBM_SETRANGE, NULL, MAKELONG(g_min, g_max));
		}
		inline void setPos(int pos) {
			SendMessage(handle, PBM_SETPOS, pos, NULL);
		}
		inline int getPos() {
			return SendMessage(handle, PBM_GETPOS, NULL, NULL);
		}
		inline void setRange(int low, int high) {
			SendMessage(handle, PBM_SETRANGE, 0, MAKELONG(low, high));
		}
		inline void setMarquee(bool enable) {
			if (enable) {
				this->addStyle(PBS_MARQUEE);
				SendMessage(handle, PBM_SETMARQUEE, TRUE, 30);
			}
			else {
				this->setStyle(WS_CHILD and WS_VISIBLE);
				SendMessage(handle, PBM_SETMARQUEE, FALSE, 30);
			}
		}
	};
	class Page;
	class Layout {
	protected:
		Page * owner;
	public:
		virtual void resize(LPARAM size) = 0; //Loword of LPARAM of WM_SIZE is width
		virtual void addControl(Control * control) = 0;
	};
	class AbsoluteLayout;
#define GUI_LAYOUT_ABS 0b001
	class Page {
		friend class Layout;
		friend class AbsoluteLayout;
	private:
		std::vector<Control*> controls;
		std::map<std::string, int> controlList;
		std::string name;
		bool shown;
		Layout * organization;
	public:
		Page(std::string name) : name(name), shown(false) {}
		Page(std::string name, std::initializer_list<Control*> list) : name(name), shown(false) {
			for (Control * c : list) {
				controls.push_back(c);
				controlList.insert(std::pair<std::string, int>(c->getName(), controls.size() - 1));
			}
		}
		~Page() {
			for (int i = 0; i < controls.size(); i++) {
				delete controls[i];
			}
			if (organization != NULL) delete organization;
		}
		void setLayout(Layout * layout) {
			organization = layout;
		}
		void resize(LPARAM newSize) {
			if (organization != NULL) organization->resize(newSize);
		}
		void addControl(Control * control) {
			if (organization != NULL) organization->addControl(control);
			controls.push_back(control);
			controlList.insert(std::pair<std::string, int>(control->getName(), controls.size() - 1));
		}
		void showPage(bool show) {
			shown = show;
			if (show) {
				for (int i = 0; i < controls.size(); i++) {
					controls[i]->showComponent();
				}
			}
			else {
				for (int i = 0; i < controls.size(); i++) {
					controls[i]->hideComponent();
				}
			}
		}
		void handleMessages(MSG * msg) {
			for (int i = 0; i < controls.size(); i++) {
				if (controls[i]->doesDefaultMsgHandle()) {
					controls[i]->handleMsg(msg);
				}
			}
		}
		Control * getControl(int i) {
			if (i < controls.size()) return controls[i];
			else return NULL;
		}
		Control * getControl(std::string name) {
			return controls[controlList[name]];
		}
		inline std::string getName() { return name; }
		inline bool isCurrentPage() { return shown; }
	};
#define GUI_ABS_LAYOUT_PRESERVER_LOCATION 1
#define GUI_ABS_LAYOUT_PRESERVE_DIMENSIONS 2
	class AbsoluteLayout : public Layout {
	public:
		AbsoluteLayout(Page * p) {
			owner = p;
		}
		void resize(LPARAM size) {
			int oldWidth = GUI::getDimensions().first;
			int oldHeight = GUI::getDimensions().second;
			for (Control * control : owner->controls) {
				int newx, newy;
				Dimensions dimensions = control->getDimensions();
				if (oldWidth == 0) oldWidth = 1;
				if (oldHeight == 0) oldHeight = 1;
				newx = ((double)dimensions.x * LOWORD(size)) / (double)oldWidth;
				newy = ((double)dimensions.y * HIWORD(size)) / (double)oldHeight;
				control->setPos(newx, newy, dimensions.width, dimensions.height);
			}
		}
		void addControl(Control * control) {};
	};
	class MainPage {
	private:
		std::vector<Page*> pages;
		std::map<std::string, int> pageList;
	public:
		MainPage() {}
		MainPage(std::initializer_list<Page*> list) {
			for (Page* p : list) {
				pages.push_back(p);
				pageList.insert(std::pair<std::string, int>(p->getName(), pages.size() - 1));
			}
		}
		~MainPage() {
			for (int i = 0; i < pages.size(); i++) {
				delete pages[i];
			}
		}
		void addPage(Page* p) {
			pages.push_back(p);
			pageList.insert(std::pair<std::string, int>(p->getName(), pages.size() - 1));
		}
		void navigateTo(int i) {
			//			if (i == 2) MessageBox(NULL, "Navigation to 2", "", MB_OK);
			for (int j = 0; j < pages.size(); j++) {
				if (j == i) {
					pages[j]->showPage(true);
					//					MessageBox(NULL, "Showing page", "KHBR", MB_OK);
				}
				else pages[j]->showPage(false);
			}
		}
		inline void navigateTo(std::string page) {
			int i = pageList[page];
			navigateTo(i);
		}

		std::string getCurrentPageName() {
			for (int i = 0; i < pages.size(); i++) {
				if (pages[i]->isCurrentPage()) {
					for (auto it = pageList.begin(); it != pageList.end(); it++) {
						if ((*it).second == i) return (*it).first;
					}
					break;
				}
			}
			return "";
		}
		Page * getCurrentPage() {
			for (int i = 0; i < pages.size(); i++) {
				if (pages[i]->isCurrentPage()) return pages[i];
			}
		}
		inline Page * getPage(std::string name) {
			return pages[pageList[name]];
		}
		void resize(LPARAM size) {
			for (int i = 0; i < pages.size(); i++) {
				pages[i]->resize(size);
			}
			GUI::setDimensions(LOWORD(size), HIWORD(size));
		}
		void handleMessage(MSG * msg) {
			for (int i = 0; i < pages.size(); i++) {
				if (pages[i]->isCurrentPage()) {
					pages[i]->handleMessages(msg);
				}
			}
		}
	};
#define GUI_CLEANUP(MAINPAGE) (delete (MAINPAGE))
}