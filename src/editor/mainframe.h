/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Implements Main frame for the editor.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"

#include <afxwin.h>
#include <afxext.h>

class LayersList;
class ScriptDialog;

class CMainFrame : public CFrameWnd
{
public:
    CMainFrame();

protected:
    DECLARE_DYNAMIC(CMainFrame)

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs);

    virtual ~CMainFrame();

    void ResetWindowPositions();
    void AdjustWindowSize();

    static CMainFrame *GetMainFrame() { return TheMainFrame; }

protected:
    CStatusBar m_wndStatusBar;
    CToolBar m_wndToolBar;
    CToolBar m_floatingToolBar;

    //m_brushOptions
    //m_terrainMaterial
    //m_blendMaterial
    //m_objectOptions
    //m_fenceOptions
    //m_mapObjectProps
    //m_moundOptions
    //m_roadOptions
    //m_featherOptions
    //m_meshMoldOptions
    //m_waypointOptions
    //m_waterOptions
    //m_lightOptions
    //m_buildListOptions
    //m_groveOptions
    //m_rampOptions
    //m_scorchOptions
    //m_noOptions
    //m_globalLightOptions
    //m_cameraOptions

    LayersList *m_layersList;
    ScriptDialog *m_scripts;

    //m_rulerOptions

    CWnd *m_curOptions;

    int int1;
    int int2;

    int m_optionsPanelWidth;
    int m_optionsPanelHeight;
    int m_globalLightOptionsWidth;
    int m_globalLightOptionsHeight;
    int m_3dViewWidth;

    bool m_autoSaving;
    unsigned int m_hAutoSaveTimer;
    bool m_autoSave;
    int m_autoSaveInterval;

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);

    // TODO
    static CMainFrame *TheMainFrame;

    DECLARE_MESSAGE_MAP()
};