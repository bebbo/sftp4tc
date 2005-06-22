#ifndef _COMMON_DIALOG_H
#define _COMMON_DIALOG_H

#include <wx/wx.h>

class CommonDialog : public wxDialog
{
public: 
  bool loaded() {return dlgres;};

protected:
  bool dlgres;
};

#endif //_COMMON_DIALOG_H
