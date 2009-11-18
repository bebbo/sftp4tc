#ifndef _SESSION_CONFIG_DIALOG_H
#define _SESSION_CONFIG_DIALOG_H

#include <wx/wx.h>
#include "wx/xrc/xmlres.h"          // XRC XML resouces
#include "CommonDialog.h"
#include "sftp4tc_share.h"

class SessionConfigDialog : public CommonDialog
{
public: 
  // Constructor.
  /*
      \param parent The parent window. Simple constructor.
    */    
  SessionConfigDialog(ConfigPropertiesType *aProperties, wxWindow *aParent);
  
  // Destructor.                  
  ~SessionConfigDialog();

private:
  void chkShowHiddenFilesChecked( wxCommandEvent &event );
  void OnCancelButtonClicked( wxCommandEvent &event );
  void OnOKButtonClicked( wxCommandEvent &event );
  DECLARE_EVENT_TABLE()

  ConfigPropertiesType *mProperties;
  wxCheckBox *chkShowHiddenFiles;
};

#endif //_SESSION_CONFIG_DIALOG_H
