#ifndef WXMYAPP_H
#define WXMYAPP_H

#include <wx/wx.h>

class wxMyApp: public wxApp {
public:
  virtual bool OnInit();
};

DECLARE_APP(wxMyApp);

#endif //WXMYAPP_H