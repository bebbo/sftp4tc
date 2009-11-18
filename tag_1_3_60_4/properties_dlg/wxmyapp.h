#ifndef WXMYAPP_H
#define WXMYAPP_H

#include <wx/wx.h>

class wxMyApp: public wxApp {
public:
  virtual bool OnInit();

  void SetHWND(HWND aMainWindow);
  wxWindow *GetHostWindow();
  void ReleaseHostWindow();
private:
  HWND mMainWindow;
  wxWindow *mHostWindow;
};

DECLARE_APP(wxMyApp);

#endif //WXMYAPP_H
