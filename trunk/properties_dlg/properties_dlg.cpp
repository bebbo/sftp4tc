#include "properties_dlg.h"
#include "wxmyapp.h"
#include "wx/xrc/xmlres.h"          // XRC XML resouces
#include "putty_proxy.h"

#define DefaultXrcFileName "rc/wfx_sftp_cfg.xrc"
#define lstConnections_ID "lstConnections"
//TextBoxes
#define edtTitle_ID "edtTitle"
#define edtHost_ID "edtHost"
#define edtPort_ID "edtPort"
#define edtUsername_ID "edtUsername"
#define edtPassword_ID "edtPassword"
#define edtPort_ID "edtPort"
#define edtHomedir_ID "edtHomedir"
#define edtChmodPut_ID "edtChmodPut"
#define edtChmodDir_ID "edtChmodDir"
#define edtKeyFilename_ID "edtKeyFilename"
#define edtProxyHost_ID "edtProxyHost"
#define edtProxyPort_ID "edtProxyPort"
#define edtProxyUsername_ID "edtProxyUsername"
#define edtProxyPassword_ID "edtProxyPassword"
#define edtProxyTelnetCommand_ID "edtProxyTelnetCommand"
//CheckBoxes
#define chkAsk4Username_ID "chkAsk4Username"
#define chkAsk4Password_ID "chkAsk4Password"
#define chkCompression_ID "chkCompression"
#define chkSetMTime_ID "chkSetMTime"
#define chkChmodPut_ID "chkChmodPut"
#define chkChmodDir_ID "chkChmodDir"
#define chkUseKeyAuth_ID "chkUseKeyAuth"
#define chkAsk4Passphrase_ID "chkAsk4Passphrase"
//Choices
#define cboProxyType_ID "cboProxyType"

//Proxy choices
#define CHOICE_PROXY_NONE "none"
#define CHOICE_PROXY_SOCKS4 "socks4"
#define CHOICE_PROXY_SOCKS5 "socks5"
#define CHOICE_PROXY_HTTP "http"
#define CHOICE_PROXY_TELNET "telnet"
#define CHOICE_PROXY_CMD "cmd"

class PreferencesDialog : public wxDialog
{
public: 
  // Constructor.
  /*
      \param parent The parent window. Simple constructor.
    */    
  PreferencesDialog(SftpServerAccountInfo *AllServers, int &count, int imported_sessions);
  
  // Destructor.                  
  ~PreferencesDialog();

  bool loaded() {return dlgres;};

private:
  // Stuff to do when "My Button" gets clicked
  void OnCloseButtonClicked( wxCommandEvent &event );
  void OnListBox( wxCommandEvent &event );
  void OnTitleTextChange( wxCommandEvent &event );
  void OnHostTextChange( wxCommandEvent &event );
  DECLARE_EVENT_TABLE()

  bool dlgres;

  SftpServerAccountInfo *mCurrentServer;
  int mPos;

  wxListBox *lstConnections;
  wxTextCtrl *edtTitle, *edtHost, *edtPort, *edtUsername, *edtPassword,
    *edtHomedir, *edtChmodPut, *edtChmodDir, *edtKeyFilename, *edtProxyHost, 
    *edtProxyPort, *edtProxyUsername, *edtProxyPassword, *edtProxyTelnetCommand;
  wxCheckBox *chkAsk4Username, *chkAsk4Password, *chkCompression, *chkSetMTime, 
    *chkChmodPut, *chkChmodDir, *chkUseKeyAuth, *chkAsk4Passphrase;
  wxChoice *cboProxyType;
};


//---------------------------------------------------------------------

BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
  EVT_BUTTON( XRCID( "btnClose" ), PreferencesDialog::OnCloseButtonClicked )    
  EVT_LISTBOX( XRCID( lstConnections_ID ), PreferencesDialog::OnListBox )
  EVT_TEXT( XRCID( edtTitle_ID ), PreferencesDialog::OnTitleTextChange )
  EVT_TEXT( XRCID( edtHost_ID ), PreferencesDialog::OnHostTextChange )
END_EVENT_TABLE()

//---------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(SftpServerAccountInfo *AllServers, int &count, 
                                     int imported_sessions): mCurrentServer(0), mPos(-1)
{
  dlgres = wxXmlResource::Get()->LoadDialog(this, NULL, wxT("CONNECTIONS"));

  if (dlgres) {
    lstConnections = XRCCTRL(*this, lstConnections_ID, wxListBox);
    edtTitle = XRCCTRL(*this, edtTitle_ID, wxTextCtrl);
    edtHost = XRCCTRL(*this, edtHost_ID, wxTextCtrl);
    edtPort = XRCCTRL(*this, edtPort_ID, wxTextCtrl);
    edtUsername = XRCCTRL(*this, edtUsername_ID, wxTextCtrl);
    edtPassword = XRCCTRL(*this, edtPassword_ID, wxTextCtrl);
    edtHomedir = XRCCTRL(*this, edtHomedir_ID, wxTextCtrl);
    edtChmodPut = XRCCTRL(*this, edtChmodPut_ID, wxTextCtrl);
    edtChmodDir = XRCCTRL(*this, edtChmodDir_ID, wxTextCtrl);
    edtKeyFilename = XRCCTRL(*this, edtKeyFilename_ID, wxTextCtrl);
    edtProxyHost = XRCCTRL(*this, edtProxyHost_ID, wxTextCtrl);
    edtProxyPort = XRCCTRL(*this, edtProxyPort_ID, wxTextCtrl);
    edtProxyUsername = XRCCTRL(*this, edtProxyUsername_ID, wxTextCtrl);
    edtProxyPassword = XRCCTRL(*this, edtProxyPassword_ID, wxTextCtrl);
    edtProxyTelnetCommand = XRCCTRL(*this, edtProxyTelnetCommand_ID, wxTextCtrl);
    //
    chkAsk4Username = XRCCTRL(*this, chkAsk4Username_ID, wxCheckBox);
    chkAsk4Password = XRCCTRL(*this, chkAsk4Password_ID, wxCheckBox);
    chkCompression = XRCCTRL(*this, chkCompression_ID, wxCheckBox);
    chkSetMTime = XRCCTRL(*this, chkSetMTime_ID, wxCheckBox);
    chkChmodPut = XRCCTRL(*this, chkChmodPut_ID, wxCheckBox);
    chkChmodDir = XRCCTRL(*this, chkChmodDir_ID, wxCheckBox);
    chkUseKeyAuth = XRCCTRL(*this, chkUseKeyAuth_ID, wxCheckBox);
    chkAsk4Passphrase = XRCCTRL(*this, chkAsk4Passphrase_ID, wxCheckBox);
    //
    cboProxyType = XRCCTRL(*this, cboProxyType_ID, wxChoice);

    dlgres = lstConnections && edtTitle && edtHost && edtPort && edtUsername && 
      edtPassword && edtHomedir && edtChmodPut && edtChmodDir && edtKeyFilename &&
      edtProxyHost && edtProxyPort && edtProxyUsername && edtProxyPassword && 
      edtProxyTelnetCommand && chkAsk4Username && chkAsk4Password && chkCompression && 
      chkSetMTime && chkChmodPut && chkChmodDir && chkUseKeyAuth && chkAsk4Passphrase &&
      cboProxyType;

    if (dlgres) {
      lstConnections->Clear();
      cboProxyType->Clear();
      cboProxyType->Append(CHOICE_PROXY_NONE);
      cboProxyType->Append(CHOICE_PROXY_SOCKS4);
      cboProxyType->Append(CHOICE_PROXY_SOCKS5);
      cboProxyType->Append(CHOICE_PROXY_HTTP);
      cboProxyType->Append(CHOICE_PROXY_TELNET);
      cboProxyType->Append(CHOICE_PROXY_CMD);
      for(int i=0; i<count-1-imported_sessions; i++) {
        lstConnections->Append(AllServers[i].title, &AllServers[i]);
      }
      if (lstConnections->GetCount()>0) {
        wxCommandEvent dummy_event;

        lstConnections->SetSelection(0);
        OnListBox(dummy_event);
      }
    }
  }
}

//---------------------------------------------------------------------

// Destructor. (Empty, as I don't need anything special done when destructing).
PreferencesDialog::~PreferencesDialog()
{
}

//---------------------------------------------------------------------

void PreferencesDialog::OnCloseButtonClicked( wxCommandEvent &event )
{
  this->Close();
}

//---------------------------------------------------------------------

void PreferencesDialog::OnListBox( wxCommandEvent &event )
{
  int pos = lstConnections->GetSelection();
  if (pos!=mPos) {
    char buf[MAX_PATH];
    mPos = pos;
    mCurrentServer = (SftpServerAccountInfo *)lstConnections->GetClientData(mPos);
    //TextBoxes
    edtTitle->SetValue(mCurrentServer->title);
    edtHost->SetValue(mCurrentServer->host);
    sprintf(buf, "%d", mCurrentServer->port);
    edtPort->SetValue(buf);
    edtUsername->SetValue(mCurrentServer->username);
    edtPassword->SetValue(mCurrentServer->password);
    edtHomedir->SetValue(mCurrentServer->home_dir);
    sprintf(buf, "%d", mCurrentServer->chmod_value_put);
    edtChmodPut->SetValue(buf);
    sprintf(buf, "%d", mCurrentServer->chmod_value_mkdir);
    edtChmodDir->SetValue(buf);
    edtKeyFilename->SetValue(mCurrentServer->keyfilename);
    edtProxyHost->SetValue(mCurrentServer->proxy_host);
    sprintf(buf, "%d", mCurrentServer->proxy_port);
    edtProxyPort->SetValue(buf);
    edtProxyUsername->SetValue(mCurrentServer->proxy_username);
    edtProxyPassword->SetValue(mCurrentServer->proxy_password);
    edtProxyTelnetCommand->SetValue(mCurrentServer->proxy_telnet_command);
    //CheckBoxes
    chkAsk4Username->SetValue(!mCurrentServer->dont_ask4_username);
    chkAsk4Password->SetValue(!mCurrentServer->dont_ask4_password);
    chkCompression->SetValue(mCurrentServer->compression);
    chkSetMTime->SetValue(mCurrentServer->set_mtime_after_put);
    chkChmodPut->SetValue(mCurrentServer->set_chmod_after_put);
    chkChmodDir->SetValue(mCurrentServer->set_chmod_after_mkdir);
    chkUseKeyAuth->SetValue(mCurrentServer->use_key_auth);
    chkAsk4Passphrase->SetValue(!mCurrentServer->dont_ask4_passphrase);
    //Choices
    cboProxyType->SetSelection(mCurrentServer->proxy_type);
    if (cboProxyType->GetSelection())
      cboProxyType->SetSelection(PROXY_NONE);
  }
}

//---------------------------------------------------------------------

void PreferencesDialog::OnTitleTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->title, edtTitle->GetValue().c_str(), MAX_PATH);
  lstConnections->SetString(mPos, mCurrentServer->title);
}

//---------------------------------------------------------------------

void PreferencesDialog::OnHostTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->host, edtHost->GetValue().c_str(), MAX_PATH);
}

//---------------------------------------------------------------------

wxApp *app;
wxLog *logStdErr;
char xrcFileName[MAX_PATH];

void __stdcall InitializeCfgDLL(HMODULE hModule)
{
  GetModuleFileName(hModule, xrcFileName, sizeof(xrcFileName) - 1);
  char *p = strrchr(xrcFileName, '\\');
  if (p)
    p++;
  else
    p = xrcFileName;
  strcpy(p, DefaultXrcFileName);
  for(size_t i=0; xrcFileName[i]!=0; i++) if (xrcFileName[i]=='\\') xrcFileName[i]='/';

  app = new wxMyApp();
  app->Initialize();
  logStdErr = new wxLogStderr();
  wxLog::SetActiveTarget(logStdErr);
}

//---------------------------------------------------------------------

void __stdcall FreeCfgDLL()
{
  if (app != NULL) {
    app->CleanUp();
    app = NULL;
  }
}

//---------------------------------------------------------------------

bool __stdcall Properties(int Mode, SftpServerAccountInfo *AllServers, int &count, 
                          int imported_sessions)
{
  bool res = false;
  try
  {
    wxXmlResource *xmlres = wxXmlResource::Get();
    xmlres->InitAllHandlers();
    bool loadres = xmlres->Load(wxT(xrcFileName));

    PreferencesDialog *dlg = new PreferencesDialog(AllServers, count, imported_sessions);
    
    // "non_derived_dialog" is the name of the wxDialog XRC node that should
    // be loaded.
    try
    {
      if (dlg->loaded()) {
        dlg->ShowModal();
        res = true;
      }
    } catch(...) {
    }

    delete dlg;
  } catch(...) {
  }

  return res;
}

//---------------------------------------------------------------------

IMPLEMENT_APP_NO_MAIN(wxMyApp);

//---------------------------------------------------------------------

bool wxMyApp::OnInit()
{
  return FALSE;
}
