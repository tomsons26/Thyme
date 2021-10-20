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
#pragma once

#include "always.h"
#include <afxwin.h>

#include "asciistring.h"

class MapObject;
class Tool;

class CWorldBuilderApp : public CWinApp
{
public:
    enum
    {
        NUM_VIEW_TOOLS = 25,
    };

public:
    CWorldBuilderApp();
    ~CWorldBuilderApp();

public:
    virtual BOOL InitInstance() override;
    virtual int ExitInstance() override;


    void OnAppAbout();
    void OnResetWindows();

private:
    Tool *m_tools[NUM_VIEW_TOOLS];

    Tool *m_curTool;
    Tool *m_selTool;

    // Tool m_brushTool;
    // Tool m_tileTool;
    // Tool m_bigTileTool;
    // Tool m_featherTool;
    // Tool m_autoEdgeOutTool;
    // Tool m_floodFillTool;
    // Tool m_moundTool;
    // Tool m_digTool;
    // Tool m_eyedropperTool;
    // Tool m_objectTool;
    // Tool m_pointerTool;
    // Tool m_blendEdgeTool;
    // Tool m_groveTool;
    // Tool m_handScrollTool;
    // Tool m_roadTool;
    // Tool m_meshMoldTool;
    // Tool m_waypointTool;
    // Tool m_polygonTool;
    // Tool m_waterTool;
    // Tool m_buildListTool;
    // Tool m_fenceTool;
    // Tool m_rampTool;
    // Tool m_scorchTool;
    // Tool m_borderTool;
    // Tool m_rulerTool;

    int m_lockCurTool;
    Utf8String m_currentDirectory;
    CDocTemplate *m_3dtemplate;
    MapObject *m_pasteMapObjList;

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
};

//TODO needs to be wrapped
CWorldBuilderApp g_theApp;

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();
    ~CAboutDlg();

protected:
    virtual void DoDataExchange(CDataExchange *pDX) override;
};