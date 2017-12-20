#include <Windows.h>
#include <vector>
namespace gui{
  class Control{
    protected:
      HWND handle;
      HWND parent;
    protected:
      virtual void init() = 0;
    public:
      void hideComponent(){
        ShowWindow(handle, SW_HIDE);
      }
      void showComponent(){
        ShowWindow(handle. SW_SHOW);
      }
      void setParent(HWND window){
        parent = window;
      }
      UINT getMessage(MSG msg){
        if(msg.hwnd == handle) return msg.message;
        else return NULL;
      }
  }
  class Button : public Control{
    protected:
      void init(){
        handle = CreateWindow("BUTTON", "", WS_CHILD | BS_DEFPUSHBUTTOn, 0, 0, 100, 50, parent, NULL, NULL, NULL);
      }
    public:
      Button(int x, int y, int width, int height, char * title, int params = NULL){
        handle = CreateWindow("BUTTON", title, WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);        
      }
      Button(){
        init();
      } 
  }
  class TextField : public Control{
  protected:
    void init(){
      handle = CreateWindow("EDIT", "", WS_CHILD, 0, 0, 100, 50, parent, NULL, NULL, NULL);
    }
  public:
    TextField(int x, int y, int width, int height, int params = NULL){
      handle = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
    }
    TextField(){
      init();
    }
  }
  class Label : public Control{
    protected:
      void init(){
        handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 100, 50, parent, NULL, NULL, NULL);
        SetWindowText(handle, "Label:");
      }
     public:
      Label(int x, int y, int params, char * text){      
        handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | params, x, y, strlen(text) * 10, 20, parent, NULL, NULL, NULL);
        SetWindowText(handle, text);
      }
      Label(){
        init();
      }
      
  }
  
  class Page(){
    private:
      std::vector<HWND> controls;
      HWND window;
    public:
      Page(HWND window): window(window){}
      void addControl(HWND control){
        controls.push_back(control);
        controls[controls.size() - 1].setParent(window);
      }
      void showPage(bool show){
        if(show){
          for(int i = 0; i < controls.size(); i++){
            ShowWindow(controls[i], SW_SHOW);
          }
        }else{
          for(int i = 0; i < controls.size(); i++){
            ShowWindows(controls[i], SW_HIDE);
          }
        }
      }
  }
  class MainPage{
  private:
    std::vector<Page> pages;
   public:
    void addPage(Page p){
      pages.push_back(p);
    }
    void navigateTo(int i){
      for(int j = 0; j < pages.size(); page++){
        if(j == i) pages[j].showPage(true);
        else pages[j].showPage(false);
      }
    }
  }
}
