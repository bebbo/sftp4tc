#include "PreferencesDialog.h"
#include "putty_proxy.h"
#include "ServerInfo.h"
#include "ConfigProperties.h"

#define lstConnections_ID "lstConnections"
//TextBoxes
#define edtTitle_ID "edtTitle"
#define edtHost_ID "edtHost"
#define edtPort_ID "edtPort"
#define edtUsername_ID "edtUsername"
#define edtPassword_ID "edtPassword"
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
//
#define cboImportPutty_ID "cboImportPutty"
#define edtImportSSHcom_ID "edtImportSSHcom"

//Proxy choices
#define CHOICE_PROXY_NONE "none"
#define CHOICE_PROXY_SOCKS4 "socks4"
#define CHOICE_PROXY_SOCKS5 "socks5"
#define CHOICE_PROXY_HTTP "http"
#define CHOICE_PROXY_TELNET "telnet"
#define CHOICE_PROXY_CMD "cmd"

#define CHOICE_PUTTY_IMPORT_NONE "no"
#define CHOICE_PUTTY_IMPORT_PASSWORD "use password authentication"
#define CHOICE_PUTTY_IMPORT_AUTH_KEY "use key authentication"

//---------------------------------------------------------------------

BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
  EVT_BUTTON( XRCID( "btnNew" ), PreferencesDialog::OnNewButtonClicked )    
  EVT_BUTTON( XRCID( "btnDuplicate" ), PreferencesDialog::OnDuplicateButtonClicked )    
  EVT_BUTTON( XRCID( "btnDelete" ), PreferencesDialog::OnDeleteButtonClicked )    
  EVT_BUTTON( XRCID( "btnClose" ), PreferencesDialog::OnCloseButtonClicked )    
  EVT_LISTBOX( XRCID( lstConnections_ID ), PreferencesDialog::OnListBox )
  EVT_TEXT( XRCID( edtTitle_ID ), PreferencesDialog::OnTitleTextChange )
  EVT_TEXT( XRCID( edtHost_ID ), PreferencesDialog::OnHostTextChange )
  EVT_TEXT( XRCID( edtPort_ID ), PreferencesDialog::OnPortTextChange )
  EVT_TEXT( XRCID( edtUsername_ID ), PreferencesDialog::OnUsernameTextChange )
  EVT_TEXT( XRCID( edtPassword_ID ), PreferencesDialog::OnPasswordTextChange )
  EVT_TEXT( XRCID( edtHomedir_ID ), PreferencesDialog::OnHomedirTextChange )
  EVT_TEXT( XRCID( edtChmodPut_ID ), PreferencesDialog::OnChmodPutTextChange )
  EVT_TEXT( XRCID( edtChmodDir_ID ), PreferencesDialog::OnChmodDirTextChange )
  EVT_TEXT( XRCID( edtKeyFilename_ID ), PreferencesDialog::OnKeyFilenameTextChange )
  EVT_TEXT( XRCID( edtProxyHost_ID ), PreferencesDialog::OnProxyHostTextChange )
  EVT_TEXT( XRCID( edtProxyPort_ID ), PreferencesDialog::OnProxyPortTextChange )
  EVT_TEXT( XRCID( edtProxyUsername_ID ), PreferencesDialog::OnProxyUsernameTextChange )
  EVT_TEXT( XRCID( edtProxyPassword_ID ), PreferencesDialog::OnProxyPasswordTextChange )
  EVT_TEXT( XRCID( edtProxyTelnetCommand_ID ), PreferencesDialog::OnProxyTelnetCommandTextChange )
  EVT_CHECKBOX( XRCID( chkAsk4Username_ID ), PreferencesDialog::chkAsk4UsernameChecked )
  EVT_CHECKBOX( XRCID( chkAsk4Password_ID ), PreferencesDialog::chkAsk4PasswordChecked )
  EVT_CHECKBOX( XRCID( chkCompression_ID ), PreferencesDialog::chkCompressionChecked )
  EVT_CHECKBOX( XRCID( chkSetMTime_ID ), PreferencesDialog::chkSetMTimeChecked )
  EVT_CHECKBOX( XRCID( chkChmodPut_ID ), PreferencesDialog::chkChmodPutChecked )
  EVT_CHECKBOX( XRCID( chkChmodDir_ID ), PreferencesDialog::chkChmodDirChecked )
  EVT_CHECKBOX( XRCID( chkUseKeyAuth_ID ), PreferencesDialog::chkUseKeyAuthChecked )
  EVT_CHECKBOX( XRCID( chkAsk4Passphrase_ID ), PreferencesDialog::chkAsk4PassphraseChecked )
  EVT_CHOICE( XRCID( cboProxyType_ID ), PreferencesDialog::cboProxyTypeChoiceSelected )
  EVT_CHECKBOX( XRCID( cboImportPutty_ID ), PreferencesDialog::cboImportPuttyChecked )
  EVT_TEXT( XRCID( edtImportSSHcom_ID ), PreferencesDialog::edtImportSSHcomTextChange )
END_EVENT_TABLE()

//---------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(config_properties *aProperties, wxWindow *aParent): 
  mProperties(aProperties), mCurrentServer(0), mPos(-1)
{
  dlgres = wxXmlResource::Get()->LoadDialog(this, aParent, wxT("CONNECTIONS"));

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
    //
    cboImportPutty = XRCCTRL(*this, cboImportPutty_ID, wxChoice);
    edtImportSSHcom = XRCCTRL(*this, edtImportSSHcom_ID, wxTextCtrl);

    dlgres = lstConnections && edtTitle && edtHost && edtPort && edtUsername && 
      edtPassword && edtHomedir && edtChmodPut && edtChmodDir && edtKeyFilename &&
      edtProxyHost && edtProxyPort && edtProxyUsername && edtProxyPassword && 
      edtProxyTelnetCommand && chkAsk4Username && chkAsk4Password && chkCompression && 
      chkSetMTime && chkChmodPut && chkChmodDir && chkUseKeyAuth && chkAsk4Passphrase &&
      cboProxyType && cboImportPutty && edtImportSSHcom;

    if (dlgres) {
      lstConnections->Clear();
      cboProxyType->Clear();
      cboImportPutty->Clear();
      cboProxyType->Append(CHOICE_PROXY_NONE);
      cboProxyType->Append(CHOICE_PROXY_SOCKS4);
      cboProxyType->Append(CHOICE_PROXY_SOCKS5);
      cboProxyType->Append(CHOICE_PROXY_HTTP);
      cboProxyType->Append(CHOICE_PROXY_TELNET);
      cboProxyType->Append(CHOICE_PROXY_CMD);
      //
      cboImportPutty->Append(CHOICE_PUTTY_IMPORT_NONE);
      cboImportPutty->Append(CHOICE_PUTTY_IMPORT_PASSWORD);
      cboImportPutty->Append(CHOICE_PUTTY_IMPORT_AUTH_KEY);
      //
      cboImportPutty->SetSelection(mProperties->DoImportPuttySessions-'0');
      if (cboImportPutty->GetSelection()==-1)
        cboImportPutty->SetSelection(0);
      edtImportSSHcom->SetValue(mProperties->DoImportSSHcomSessions);
      //
      for(int i=0; i<mProperties->ServerCount-1-mProperties->ImportedSessions; i++) {
        lstConnections->Append(mProperties->ServerInfos[i].title, &mProperties->ServerInfos[i]);
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
  SaveProperties(mProperties);
}

//------------------------ BUTTON EVENTS ---------------------------------------------

void PreferencesDialog::OnNewButtonClicked( wxCommandEvent &event )
{
  if (mProperties->ServerCount<MAX_Server_Count) {
    wxCommandEvent dummy_event;
    int pos = mProperties->ServerCount++;
    mProperties->ServerInfos[pos].id = pos;
    SetDefaultsToServerInfo(&mProperties->ServerInfos[pos]);
    strncpy(mProperties->ServerInfos[pos].title, "<new>", MAX_Server_INFO);
    mPos = -1;
    lstConnections->Append(mProperties->ServerInfos[pos].title, &mProperties->ServerInfos[pos]);
    pos = lstConnections->FindString(mProperties->ServerInfos[pos].title);
    lstConnections->SetSelection(pos);
    OnListBox(dummy_event);
  }
}

//---------------------------------------------------------------------

void PreferencesDialog::OnDuplicateButtonClicked( wxCommandEvent &event )
{
  if (mProperties->ServerCount<MAX_Server_Count) {
    wxCommandEvent dummy_event;
    int pos = mProperties->ServerCount++;
    CopyServerInfo(mProperties->ServerInfos, mCurrentServer->id, pos);
    mPos = -1;
    lstConnections->Append(mProperties->ServerInfos[pos].title, &mProperties->ServerInfos[pos]);
    pos = lstConnections->FindString(mProperties->ServerInfos[pos].title);
    lstConnections->SetSelection(pos);
    OnListBox(dummy_event);
  }
}

//---------------------------------------------------------------------

void PreferencesDialog::OnDeleteButtonClicked( wxCommandEvent &event )
{
  mCurrentServer->id = -1; //mark deleted
  lstConnections->Delete(mPos);
  int count = lstConnections->GetCount();
  if (count>0)
  {
    wxCommandEvent dummy_event;
    int pos = mPos>=count? count-1: mPos;
    mPos = -1;
    lstConnections->SetSelection(pos);
    OnListBox(dummy_event);
  } else {
    mPos = -1;
    ClearAndDisable();
  }
}

//---------------------------------------------------------------------

void PreferencesDialog::OnCloseButtonClicked( wxCommandEvent &event )
{
  this->Close();
}

//-------------------------- LISTBOX EVENTS -------------------------------------------

void PreferencesDialog::OnListBox( wxCommandEvent &event )
{
  int pos = lstConnections->GetSelection();
  if (pos!=mPos) {
    char buf[MAX_Server_INFO];
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
    chkCompression->SetValue(mCurrentServer->compression==1);
    chkSetMTime->SetValue(mCurrentServer->set_mtime_after_put==1);
    chkChmodPut->SetValue(mCurrentServer->set_chmod_after_put==1);
    chkChmodDir->SetValue(mCurrentServer->set_chmod_after_mkdir==1);
    chkUseKeyAuth->SetValue(mCurrentServer->use_key_auth==1);
    chkAsk4Passphrase->SetValue(!mCurrentServer->dont_ask4_passphrase);
    //Choices
    cboProxyType->SetSelection(mCurrentServer->proxy_type);
    if (cboProxyType->GetSelection())
      cboProxyType->SetSelection(PROXY_NONE);
  }
}

//-------------------------- EVENT MACROS -------------------------------------------

#define GetNumValue(edt, field) if (edt->GetValue().IsNumber()) { \
    unsigned long tmp; \
    edt->GetValue().ToULong( &tmp ); \
    mCurrentServer->field = tmp; \
  }

#define GetTextValue(edt, field) \
  strncpy(mCurrentServer->field, edt->GetValue().c_str(), MAX_Server_INFO);

//-------------------------- TEXT CHANGE EVENTS -------------------------------------------

void PreferencesDialog::OnTitleTextChange( wxCommandEvent &event )
{
  if (mPos!=-1) {
    strncpy(mCurrentServer->title, edtTitle->GetValue().c_str(), MAX_Server_INFO);
    lstConnections->Delete(mPos);
    lstConnections->Append(mCurrentServer->title, mCurrentServer);
    mPos = lstConnections->FindString(mCurrentServer->title);
    lstConnections->SetSelection(mPos);
  }
}

//---------------------------------------------------------------------

void PreferencesDialog::OnHostTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->host, edtHost->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnPortTextChange( wxCommandEvent &event )
{
  GetNumValue( edtPort, port )
}

void PreferencesDialog::OnUsernameTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->username, edtUsername->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnPasswordTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->password, edtPassword->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnHomedirTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->home_dir, edtHomedir->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnChmodPutTextChange( wxCommandEvent &event )
{
  GetNumValue( edtChmodPut, chmod_value_put )
}

void PreferencesDialog::OnChmodDirTextChange( wxCommandEvent &event )
{
  GetNumValue( edtChmodDir, chmod_value_mkdir )
}

void PreferencesDialog::OnKeyFilenameTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->keyfilename, edtKeyFilename->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnProxyHostTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->proxy_host, edtProxyHost->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnProxyPortTextChange( wxCommandEvent &event )
{
  GetNumValue( edtProxyPort, proxy_port )
}

void PreferencesDialog::OnProxyUsernameTextChange( wxCommandEvent &event )
{
  GetTextValue( edtProxyUsername, proxy_username )
}

void PreferencesDialog::OnProxyPasswordTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->proxy_password, edtProxyPassword->GetValue().c_str(), MAX_Server_INFO);
}

void PreferencesDialog::OnProxyTelnetCommandTextChange( wxCommandEvent &event )
{
  strncpy(mCurrentServer->proxy_telnet_command, 
    edtProxyTelnetCommand->GetValue().c_str(), MAX_Server_INFO);
}

//-------------------------- CHECKBOX EVENTS -------------------------------------------

void PreferencesDialog::chkAsk4UsernameChecked( wxCommandEvent &event )
{
  mCurrentServer->dont_ask4_username = !chkAsk4Username->GetValue();
}

void PreferencesDialog::chkAsk4PasswordChecked( wxCommandEvent &event )
{
  mCurrentServer->dont_ask4_password = !chkAsk4Password->GetValue();
}

void PreferencesDialog::chkCompressionChecked( wxCommandEvent &event )
{
  mCurrentServer->compression = chkCompression->GetValue();
}

void PreferencesDialog::chkSetMTimeChecked( wxCommandEvent &event )
{
  mCurrentServer->set_mtime_after_put = chkSetMTime->GetValue();
}

void PreferencesDialog::chkChmodPutChecked( wxCommandEvent &event )
{
  mCurrentServer->set_chmod_after_put = chkChmodPut->GetValue();
}

void PreferencesDialog::chkChmodDirChecked( wxCommandEvent &event )
{
  mCurrentServer->set_chmod_after_mkdir = chkChmodDir->GetValue();
}

void PreferencesDialog::chkUseKeyAuthChecked( wxCommandEvent &event )
{
  mCurrentServer->use_key_auth = chkUseKeyAuth->GetValue();
}

void PreferencesDialog::chkAsk4PassphraseChecked( wxCommandEvent &event )
{
  mCurrentServer->dont_ask4_passphrase = !chkAsk4Passphrase->GetValue();
}

//-------------------------- CHOICE EVENTS -------------------------------------------

void PreferencesDialog::cboProxyTypeChoiceSelected( wxCommandEvent &event )
{
  int index = cboProxyType->GetSelection();

  if ((index>=PROXY_NONE) && (index<=PROXY_CMD))
    mCurrentServer->proxy_type = index;
}

//-------------------------- CHOICE EVENTS -------------------------------------------

void PreferencesDialog::cboImportPuttyChecked( wxCommandEvent &event )
{
  mProperties->DoImportPuttySessions = cboImportPutty->GetSelection();
}

//-------------------------- CHOICE EVENTS -------------------------------------------

void PreferencesDialog::edtImportSSHcomTextChange( wxCommandEvent &event )
{
  strncpy(mProperties->DoImportSSHcomSessions, 
    edtImportSSHcom->GetValue().c_str(), MAX_Server_INFO);
}

//---------------------------------------------------------------------

void PreferencesDialog::ClearAndDisable()
{
  edtTitle->Clear();
  edtHost->Clear();
  edtPort->Clear();
  edtUsername->Clear();
  edtPassword->Clear();
  edtHomedir->Clear();
  edtChmodPut->Clear();
  edtChmodDir->Clear();
  edtKeyFilename->Clear();
  edtProxyHost->Clear();
  edtProxyPort->Clear();
  edtProxyUsername->Clear();
  edtProxyPassword->Clear();
  edtProxyTelnetCommand->Clear();
  chkAsk4Username->SetValue(false);
  chkAsk4Password->SetValue(false);
  chkCompression->SetValue(false);
  chkSetMTime->SetValue(false);
  chkChmodPut->SetValue(false);
  chkChmodDir->SetValue(false);
  chkUseKeyAuth->SetValue(false);
  chkAsk4Passphrase->SetValue(false);
  cboProxyType->SetSelection(-1);
  //
  edtTitle->Disable();
  edtHost->Disable();
  edtPort->Disable();
  edtUsername->Disable();
  edtPassword->Disable();
  edtHomedir->Disable();
  edtChmodPut->Disable();
  edtChmodDir->Disable();
  edtKeyFilename->Disable();
  edtProxyHost->Disable();
  edtProxyPort->Disable();
  edtProxyUsername->Disable();
  edtProxyPassword->Disable();
  edtProxyTelnetCommand->Disable();
  chkAsk4Username->Disable();
  chkAsk4Password->Disable();
  chkCompression->Disable();
  chkSetMTime->Disable();
  chkChmodPut->Disable();
  chkChmodDir->Disable();
  chkUseKeyAuth->Disable();
  chkAsk4Passphrase->Disable();
  cboProxyType->Disable();
}
