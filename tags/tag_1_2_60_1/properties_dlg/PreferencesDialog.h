#ifndef _PREFERENCES_DIALOG_H
#define _PREFERENCES_DIALOG_H

#include <wx/wx.h>
#include "wx/xrc/xmlres.h"          // XRC XML resouces
#include "CommonDialog.h"
#include "sftp4tc_share.h"

class PreferencesDialog : public CommonDialog
{
public: 
  // Constructor.
  /*
      \param parent The parent window. Simple constructor.
    */    
  PreferencesDialog(ConfigPropertiesType *aProperties, wxWindow *aParent);
  
  // Destructor.                  
  ~PreferencesDialog();

private:
  // Stuff to do when "My Button" gets clicked
  void OnNewButtonClicked( wxCommandEvent &event );
  void OnDuplicateButtonClicked( wxCommandEvent &event );
  void OnDeleteButtonClicked( wxCommandEvent &event );
  void OnCancelButtonClicked( wxCommandEvent &event );
  void OnOKButtonClicked( wxCommandEvent &event );
  void OnListBox( wxCommandEvent &event );
  void OnTitleTextChange( wxCommandEvent &event );
  void OnHostTextChange( wxCommandEvent &event );
  void OnPortTextChange( wxCommandEvent &event );
  void OnUsernameTextChange( wxCommandEvent &event );
  void OnPasswordTextChange( wxCommandEvent &event );
  void OnHomedirTextChange( wxCommandEvent &event );
  void OnChmodPutTextChange( wxCommandEvent &event );
  void OnChmodDirTextChange( wxCommandEvent &event );
  void OnKeyFilenameTextChange( wxCommandEvent &event );
  void OnProxyHostTextChange( wxCommandEvent &event );
  void OnProxyPortTextChange( wxCommandEvent &event );
  void OnProxyUsernameTextChange( wxCommandEvent &event );
  void OnProxyPasswordTextChange( wxCommandEvent &event );
  void OnProxyTelnetCommandTextChange( wxCommandEvent &event );
  void chkAsk4UsernameChecked( wxCommandEvent &event );
  void chkAsk4PasswordChecked( wxCommandEvent &event );
  void chkCompressionChecked( wxCommandEvent &event );
  void chkSetMTimeChecked( wxCommandEvent &event );
  void chkChmodPutChecked( wxCommandEvent &event );
  void chkChmodDirChecked( wxCommandEvent &event );
  void chkUseKeyAuthChecked( wxCommandEvent &event );
  void chkAsk4PassphraseChecked( wxCommandEvent &event );
  void cboProxyTypeChoiceSelected( wxCommandEvent &event );
  void cboImportPuttySelected( wxCommandEvent &event );
  void edtImportSSHcomTextChange( wxCommandEvent &event );
  DECLARE_EVENT_TABLE()

  void ClearAndDisable();
  void Enable();

  ConfigPropertiesType *mProperties;

  SftpServerAccountInfo *mCurrentServer;
  int mPos;
  bool mSave;

  wxListBox *lstConnections;

  wxButton *btnDelete;

  //connections details
  wxTextCtrl *edtTitle, *edtHost, *edtPort, *edtUsername, *edtPassword,
    *edtHomedir, *edtChmodPut, *edtChmodDir, *edtKeyFilename, *edtProxyHost, 
    *edtProxyPort, *edtProxyUsername, *edtProxyPassword, *edtProxyTelnetCommand;
  wxCheckBox *chkAsk4Username, *chkAsk4Password, *chkCompression, *chkSetMTime, 
    *chkChmodPut, *chkChmodDir, *chkUseKeyAuth, *chkAsk4Passphrase;
  wxChoice *cboProxyType;

  wxChoice *cboImportPutty;
  wxTextCtrl *edtImportSSHcom;
};

#endif //_PREFERENCES_DIALOG_H
