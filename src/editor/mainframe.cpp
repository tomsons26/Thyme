#include "mainframe.h"
//#include "resource.h"

class CWorldBuilderDoc;

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
ON_WM_CREATE()
ON_WM_MOVE()
// OnViewBrushfeedback
// OnUpdateViewBrushfeedback
ON_WM_DESTROY()
ON_WM_TIMER()
ON_WM_CANCELMODE()
// OnEditCameraoptions
END_MESSAGE_MAP()

static UINT indicators[] = {
    ID_SEPARATOR, // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame() :
    m_layersList(nullptr),
    m_scripts(nullptr),
    m_curOptions(nullptr),
    int1(0),
    int2(0),
    m_optionsPanelWidth(0),
    m_optionsPanelHeight(0),
    m_globalLightOptionsWidth(0),
    m_globalLightOptionsHeight(0),
    m_3dViewWidth(0),
    m_autoSaving(false),
    m_hAutoSaveTimer(0),
    m_autoSave(false),
    m_autoSaveInterval(0)
{
    TheMainFrame = this;
}

CMainFrame::~CMainFrame() {}

void CMainFrame::ResetWindowPositions()
{
    if (m_curOptions == nullptr) {
        // m_curOptions = &m_brushOptions;
    }

    SetWindowPos(nullptr, 20, 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(SW_SHOW);

    m_curOptions->SetWindowPos(nullptr, 40, 40, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_curOptions->ShowWindow(SW_SHOW);

    CWnd *view = 0; //= CWorldBuilderDoc::GetActive2DView();

    if (view != nullptr) {
        CFrameWnd *parent = view->GetParentFrame();
        if (parent) {
            parent->SetWindowPos(nullptr, 60, 60, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    CPoint pt(20, 200);
    FloatControlBar(&m_floatingToolBar, pt, CBRS_ALIGN_LEFT);
    m_floatingToolBar.SetWindowPos(nullptr, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_floatingToolBar.ShowWindow(SW_SHOW);
}

void CMainFrame::AdjustWindowSize()
{
    RECT rect;
    ::GetWindowRect(::GetDesktopWindow(), &rect);

    rect.right -= 2 * GetSystemMetrics(SM_CYCAPTION);
    rect.bottom -= 3 * GetSystemMetrics(SM_CYCAPTION);

    CRect client_rect;

    int border_width = GetSystemMetrics(SM_CXEDGE);

    int width = AfxGetApp()->GetProfileIntA("MainFrame", "Width", 800);
    int height = AfxGetApp()->GetProfileIntA("MainFrame", "Height", 600);

    CWnd *view = 0;//CWorldBuilderDoc::GetActive3DView();
    if (view != nullptr) {
        view->GetClientRect(&client_rect);
    } else {
        GetClientRect(&client_rect);
        client_rect.right -= 2 * border_width;
    }

    int w = client_rect.Width() - width;
    int h = client_rect.Height() - height;

    CRect win_rect;

    GetWindowRect(&win_rect);

    int cx = win_rect.Width() - w;
    int cy = win_rect.Height() - h;

    SetWindowPos(nullptr, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);

    if (view != nullptr) {
        //view->Reset_3D_Engine_Display_Size(width, height);
    }

    m_3dViewWidth = width;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT))) {
        TRACE0("Failed to create status bar\n");
        return -1; // fail to create
    }

	if (!m_wndToolBar.CreateEx(this) || !m_wndToolBar.LoadToolBar(128)) {
        TRACE0("Failed to create toolbar\n");
        return -1; // fail to create
    }

    m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP);

    m_autoSave = AfxGetApp()->GetProfileInt("MainFrame", "AutoSave", 1) != false;
    m_autoSaveInterval = AfxGetApp()->GetProfileInt("MainFrame", "AutoSaveIntervalSeconds", 120);
    m_hAutoSaveTimer = SetTimer(1, 1000 * m_autoSaveInterval, nullptr);
    return 0;
}

void CMainFrame::OnMove(int x, int y)
{
    CWnd::OnMove(x, y);

    if (IsWindowVisible()) {
        if (!IsIconic()) {
            RECT rect;
            GetWindowRect(&rect);
            AfxGetApp()->WriteProfileInt("MainFrame", "Top", rect.top);
            AfxGetApp()->WriteProfileInt("MainFrame", "Left", rect.left);
        }
    }
}

void CMainFrame::OnDestroy()
{
    if (m_hAutoSaveTimer) {
        KillTimer(m_hAutoSaveTimer);
    }
    m_hAutoSaveTimer = 0;

    CFrameWnd::OnDestroy();
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
    CWorldBuilderDoc *doc = 0;//CWorldBuilderDoc::GetActiveDoc();
    if (doc) {
        //if (doc->needAutoSave()) {
            m_autoSaving = true;

            HCURSOR cursor = SetCursor(LoadCursorA(nullptr, IDC_WAIT));

            SetMessageText("Auto Saving map...");
            //doc->autoSave();

            if (cursor) {
                SetCursor(cursor);
            }

            SetMessageText("Auto Save Complete.");

            m_autoSaving = false;
        //}
    }
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT &cs)
{
    return CFrameWnd::PreCreateWindow(cs);
}
