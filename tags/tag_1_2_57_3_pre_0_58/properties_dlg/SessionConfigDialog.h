#ifndef _SESSION_CONFIG_DIALOG_H
#define _SESSION_CONFIG_DIALOG_H

#include <wx/wx.h>
#include "wx/xrc/xmlres.h"          // XRC XML resouces
#include "CommonDialog.h"
#include "share.h"

class SessionConfigDialog : public CommonDialog
{
public: 
  // Constructor.
  /*
      \param parent The parent window. Simple constructor.
    */    
  SessionConfigDialog(struct config_properties *aProperties, wxWindow *aParent);
  
  // Destructor.                  
  ~SessionConfigDialog();

private:
  void chkShowHiddenFilesChecked( wxCommandEvent &event );
  void OnCancelButtonClicked( wxCommandEvent &event );
  void OnOKButtonClicked( wxCommandEvent &event );
  DECLARE_EVENT_TABLE()

  struct config_properties *mProperties;
  wxCheckBox *chkShowHiddenFiles;
};

#endif //_SESSION_CONFIG_DIALOG_H
