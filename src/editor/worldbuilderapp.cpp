/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Implements application object for the editor.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include <new>

#include "worldbuilderapp.h"
#include "resource.h"

#include "filesystem.h"
#include "mainframe.h"
#include "namekeygenerator.h"
#include "scriptlist.h"

#define MFC_TEST

BEGIN_MESSAGE_MAP(CWorldBuilderApp, CWinApp)
ON_COMMAND(IDM_APP_ABOUT, OnAppAbout)
ON_COMMAND(IDM_WINDOW_RESET_POSITIONS, OnResetWindows)
ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
// ON_COMMAND(?, OnTexturesizingMapclifftextures)
// ON_UPDATE_COMMAND_UI(?, OnUpdateTexturesizingMapclifftextures)
ON_COMMAND(IDM_FILE_NEW, OnFileNew)
ON_COMMAND(IDM_FILE_OPEN, OnFileOpen)
ON_COMMAND(IDM_FILE_PRINT_SETUP, OnFilePrintSetup)
END_MESSAGE_MAP()

CWorldBuilderApp::CWorldBuilderApp() :
    m_tools{}, m_curTool(nullptr), m_selTool(nullptr), m_lockCurTool(0), m_3dtemplate(nullptr), m_pasteMapObjList(nullptr)
{
    // TODO tool static inits
}

CWorldBuilderApp::~CWorldBuilderApp()
{
    m_curTool = nullptr;
    m_selTool = nullptr;

    for (int i = 0; i < NUM_VIEW_TOOLS; ++i) {
        if (m_tools[i]) {
            m_tools[i] = nullptr;
        }
    }

    // TODO ugly? original has this
    _exit(0);
}

BOOL CWorldBuilderApp::InitInstance()
{
#ifdef MFC_TEST
    m_pMainWnd = new CMainFrame();
    m_pMainWnd->ShowWindow(m_nCmdShow);
    return TRUE;
#endif
}

int CWorldBuilderApp::ExitInstance()
{
    // THIS HAS TODO s

    WriteProfileString("WorldbuilderApp", "OpenDirectory", m_currentDirectory);
    m_currentDirectory.Clear();

    // ScriptList::Reset();

    g_theSubsystemList->Shutdown_All();

    // WorldHeightMapEdit::Shutdown();

    delete g_theFileSystem;
    // delete g_theW3DFileSystem;
    delete g_theNameKeyGenerator;

    Shutdown_Memory_Manager();
    // Debug_Shutdown(); whats ours?

    return CWinApp::ExitInstance();
}

void CWorldBuilderApp::OnAppAbout()
{
    CAboutDlg().DoModal();
}

void CWorldBuilderApp::OnResetWindows()
{
    CMainFrame *frame = CMainFrame::GetMainFrame();
    if (frame != nullptr) {
        frame->ResetWindowPositions();
    }
}

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUT) {}

CAboutDlg::~CAboutDlg() {}

void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
    CWnd::DoDataExchange(pDX);
}
