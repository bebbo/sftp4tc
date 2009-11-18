#include "SessionConfigDialog.h"
#include "wx/msw/winundef.h"

//CheckBoxes
#define chkShowHiddenFiles_ID "chkShowHiddenFiles"

BEGIN_EVENT_TABLE(SessionConfigDialog, wxDialog)
  EVT_BUTTON( XRCID( "btnCancel" ), SessionConfigDialog::OnCancelButtonClicked )    
  EVT_BUTTON( XRCID( "btnOK" ), SessionConfigDialog::OnOKButtonClicked )    
  EVT_CHECKBOX( XRCID( chkShowHiddenFiles_ID ), SessionConfigDialog::chkShowHiddenFilesChecked )
END_EVENT_TABLE()

SessionConfigDialog::SessionConfigDialog(ConfigPropertiesType *aProperties, wxWindow *aParent):
  mProperties(aProperties)
{
  dlgres = wxXmlResource::Get()->LoadDialog(this, aParent, wxT("SESSION"));
  if (dlgres) {
    chkShowHiddenFiles = XRCCTRL(*this, chkShowHiddenFiles_ID, wxCheckBox);

    dlgres = (chkShowHiddenFiles!=0);

    if (dlgres) {
      chkShowHiddenFiles->SetValue(mProperties->ServerInfos[mProperties->SelectedSession].show_hidden_files=='1');
    }
  }
}

SessionConfigDialog::~SessionConfigDialog()
{
}

void SessionConfigDialog::chkShowHiddenFilesChecked( wxCommandEvent &event )
{
}

void SessionConfigDialog::OnCancelButtonClicked( wxCommandEvent &event )
{
  this->Close();
}

void SessionConfigDialog::OnOKButtonClicked( wxCommandEvent &event )
{
  mProperties->ServerInfos[mProperties->SelectedSession].show_hidden_files = chkShowHiddenFiles->GetValue() ? '1': '0';
  this->Close();
}
