#include "properties_dlg.h"
#include "wxmyapp.h"
#include "wx/xrc/xmlres.h"          // XRC XML resouces
#include "PreferencesDialog.h"

//---------------------------------------------------------------------
#define DefaultXrcFileName "rc/wfx_sftp_cfg.xrc"

//---------------------------------------------------------------------

wxMyApp *app;
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

bool __stdcall Properties(int Mode, struct config_properties *aProperties)
{
  bool res = false;
  try
  {
    wxXmlResource *xmlres = wxXmlResource::Get();
    xmlres->InitAllHandlers();
    bool loadres = xmlres->Load(wxT(xrcFileName));

    app->SetHWND(aProperties->MainWindow);
    PreferencesDialog *dlg = new PreferencesDialog(aProperties, app->GetHostWindow());
    
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
  app->ReleaseHostWindow();

  return res;
}

//---------------------------------------------------------------------

IMPLEMENT_APP_NO_MAIN(wxMyApp);

//---------------------------------------------------------------------

bool wxMyApp::OnInit()
{
  return FALSE;
}

//---------------------------------------------------------------------

void wxMyApp::SetHWND(HWND aMainWindow)
{
  mMainWindow = aMainWindow;
}

//---------------------------------------------------------------------

wxWindow *wxMyApp::GetHostWindow()
{
  if (mMainWindow) {
    mHostWindow = new wxWindow();
    WXHWND hwnd = (WXHWND) mMainWindow;
    mHostWindow->SetHWND(hwnd);
    mHostWindow->Disable();
    return mHostWindow;
  } else {
    return 0;
  }
}

//---------------------------------------------------------------------

void wxMyApp::ReleaseHostWindow()
{
  if (mHostWindow) {
    mHostWindow->Enable();
    mHostWindow->SetHWND(NULL);
    delete mHostWindow;
    mHostWindow = NULL;
  }
}
