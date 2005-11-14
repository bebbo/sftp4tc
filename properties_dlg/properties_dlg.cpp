#include "PreferencesDialog.h"
#include "SessionConfigDialog.h"
#include "wxmyapp.h"
#include "wx/xrc/xmlres.h"          // XRC XML resouces
#include "properties_dlg.h"
#include "wx/module.h"

//---------------------------------------------------------------------
#define DefaultXrcFileName "rc\\wfx_sftp_cfg.xrc"

//---------------------------------------------------------------------

wxMyApp *app;
wxLog *logStdErr;
char xrcFileName[MAX_PATH];

extern "C" {

void __stdcall InitializeCfgDLL(HMODULE hModule)
{
  GetModuleFileName(hModule, xrcFileName, sizeof(xrcFileName) - 1);
  char *p = strrchr(xrcFileName, '\\');
#ifdef _SFTP_DEBUG
  OutputDebugStringA("InitializeCfgDLL");
  OutputDebugStringA(p);
#endif
  if (p)
    p++;
  else
    p = xrcFileName;
  strcpy(p, DefaultXrcFileName);

#ifdef _SFTP_DEBUG
  OutputDebugStringA(xrcFileName);
#endif

  app = new wxMyApp();
#ifdef WXWIN_COMPATIBILITY_2_4
  {
    int argc=0;
    wxChar *argv=NULL;
    app->Initialize(argc, &argv);
  }
#else
  app->Initialize();
#endif

  //wxFileSystem::AddHandler(new wxLocalFSHandler);
  wxModule::RegisterModules();
  wxModule::InitializeModules();

#ifdef _SFTP_DEBUG
  wxLog::SetActiveTarget(new wxLogGui());
#else
  logStdErr = new wxLogStderr();
  wxLog::SetActiveTarget(logStdErr);
#endif
}

//---------------------------------------------------------------------

void __stdcall FreeCfgDLL()
{
  if (app != NULL) {
    app->CleanUp();
    app = NULL;
  }
  wxModule::CleanUpModules();
}

//---------------------------------------------------------------------

bool __stdcall Properties(int Mode, ConfigPropertiesType *aProperties)
{
  bool res = false;
  try
  {
    wxXmlResource *xmlres = wxXmlResource::Get();
#ifdef _SFTP_DEBUG
    if (xmlres==NULL)
  	  OutputDebugStringA("wxXmlResource::Get() failed");
#endif
    xmlres->InitAllHandlers();
    bool loadres = xmlres->Load(wxT(xrcFileName));
#ifdef _SFTP_DEBUG
    if (loadres)
  	  OutputDebugStringA("Loaded");
    else
  	  OutputDebugStringA("Not Loaded");
    wxLog::FlushActive();
#endif

    app->SetHWND(aProperties->MainWindow);
    CommonDialog *dlg;
    if (Mode==2)
      dlg = new SessionConfigDialog(aProperties, app->GetHostWindow());
    else
      dlg = new PreferencesDialog(aProperties, app->GetHostWindow());
    
    try
    {
      if (dlg->loaded()) 
      {
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

}

//---------------------------------------------------------------------

IMPLEMENT_APP(wxMyApp); //_NO_MAIN

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
    mHostWindow = 0;
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
