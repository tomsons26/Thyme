/**
 * @file
 *
 * @author tomsons26
 *
 * @brief WND UI system manager.
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
#include "subsysteminterface.h"
#include <list>

#include "wininstancedata.h"

class GameFont;
class Image;
class GameWindow;
class ModalWindow;


class GameWindowManager : public SubsystemInterface
{

public:
    GameWindowManager();

    void Init() override;
    void Reset() override;
    void Update() override;

    virtual GameWindow *Alloc_New_Window() = 0;

    virtual window_draw_func Get_Push_Button_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Push_Button_Draw_Func() = 0;

    virtual window_draw_func Get_Check_Box_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Check_Box_Draw_Func() = 0;

    virtual window_draw_func Get_Radio_Button_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Radio_Button_Draw_Func() = 0;

    virtual window_draw_func Get_Tab_Control_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Tab_Control_Draw_Func() = 0;

    virtual window_draw_func Get_List_Box_Image_Draw_Func() = 0;
    virtual window_draw_func Get_List_Box_Draw_Func() = 0;

    virtual window_draw_func Get_Combo_Box_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Combo_Box_Draw_Func() = 0;

    virtual window_draw_func Get_Horizontal_Slider_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Horizontal_Slider_Draw_Func() = 0;

    virtual window_draw_func Get_Vertical_Slider_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Vertical_Slider_Draw_Func() = 0;

    virtual window_draw_func Get_Progress_Bar_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Progress_Bar_Draw_Func() = 0;

    virtual window_draw_func Get_Static_Text_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Static_Text_Draw_Func() = 0;

    virtual window_draw_func Get_Text_Entry_Image_Draw_Func() = 0;
    virtual window_draw_func Get_Text_Entry_Draw_Func() = 0;
    //
    //virtual window_draw_func getDefaultDraw();
    //getDefaultSystem
    //getDefaultInput
    //getDefaultTooltip
    //gogoMessageBox
    //gogoMessageBox
    //gogoGadgetPushButton
    //gogoGadgetCheckbox
    //gogoGadgetRadioButton
    //gogoGadgetTabControl
    //gogoGadgetListBox
    //gogoGadgetSlider
    //gogoGadgetProgressBar
    //gogoGadgetStaticText
    //gogoGadgetTextEntry
    //gogoGadgetComboBox
    //assignDefaultGadgetLook
    //winCreateFromScript
    //winCreateLayout
    //freeStaticStrings
    //winCreate
    //winDestroy
    //winDestroyAll
    virtual GameWindow *Win_Get_Window_List();
    //hideWindowsInRange
    //enableWindowsInRange
    //windowHiding
    //winRepaint
    //winNextTab
    //winPrevTab
    virtual void Register_Tab_List(std::list<GameWindow *> list);
    virtual void Clear_Tab_List();
    //winProcessMouseEvent
    //winProcessKey
    virtual GameWindow *Win_Get_Focus();
    //winSetFocus
    virtual void Win_Set_Grab_Window(GameWindow *window);
    virtual GameWindow *Win_Get_Grab_Window();
    //winSetLoneWindow
    //isEnabled
    //isHidden
    //addWindowToParent
    //addWindowToParentAtEnd
    //winSendSystemMsg
    //winSendInputMsg
    //virtual GameWindow *Win_Get_Window_From_Id(GameWindow *window, int id);
    virtual int Win_Capture(GameWindow *);
    virtual int Win_Release(GameWindow *);
    virtual GameWindow *Get_Win_Capture();
    //virtual int Win_Set_Modal(GameWindow *);
    //virtual int Win_Unset_Modal(GameWindow *);
    virtual void Win_Draw_Image(Image *image, int start_x, int start_y, int end_x, int end_y, int color);
    virtual void Win_Fill_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y);
    virtual void Win_Open_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y);
    virtual void Win_Draw_Line(int color, float width, int start_x, int start_y, int end_x, int end_y);
    virtual int Win_Make_Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
    //virtual const Image *Win_Find_Image(const char *name);
    virtual int Win_Font_Height(GameFont *font);
    virtual int Win_Is_Digit(int c);
    virtual int Win_Is_Ascii(int c);
    virtual int Win_Is_Alpha_Numeric(int c);
    //winFormatText
    //winGetTextSize
    //winTextLabelToText
    virtual GameFont *Win_Find_Font(Utf8String font_name, int point_size, int bold);
    virtual GameWindow *Get_Win_Under_Cursor(int x, int y, int ignore_enabled);

    void Process_Destroy_List();

#ifdef GAME_DLL
    GameWindowManager *Hook_Ctor() { return new (this) GameWindowManager; }

#endif

private:
    GameWindow *m_windowList;
    GameWindow *m_windowTail;
    GameWindow *m_destroyList;

    GameWindow *m_currMouseRgn;
    GameWindow *m_mouseCaptor;

    GameWindow *m_keyboardFocus;

    ModalWindow *m_modalHead;

    GameWindow *m_grabWindow;
    GameWindow *m_loneWindow;

    std::list<GameWindow *> m_tabList;

    Image *m_cursorBitmap; // needs confirming
    unsigned int m_captureFlags; // needs confirming
};