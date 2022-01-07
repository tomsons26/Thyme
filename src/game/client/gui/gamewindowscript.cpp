/**
 * @file
 *
 * @author tomsons26
 *
 * @brief WND UI system manager parser.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "colorspace.h"
#include "display.h"
#include "file.h"
#include "functionlexicon.h"
#include "gamefont.h"
#include "gametext.h"
#include "gamewindowmanager.h"
#include "image.h"
#include "windowlayout.h"

#ifndef GAME_DLL
Utf8String s_theSystemString;
Utf8String s_theInputString;
Utf8String s_theTooltipString;
Utf8String s_theDrawString;
#else
extern Utf8String &s_theSystemString;
extern Utf8String &s_theInputString;
extern Utf8String &s_theTooltipString;
extern Utf8String &s_theDrawString;
#endif

const char *s_WindowStatusNames[] = {
    "ACTIVE",
    "TOGGLE",
    "DRAGABLE",
    "ENABLED",
    "HIDDEN",
    "ABOVE",
    "BELOW",
    "IMAGE",
    "TABSTOP",
    "NOINPUT",
    "NOFOCUS",
    "DESTROYED",
    "BORDER",
    "SMOOTH_TEXT",
    "ONE_LINE",
    "NO_FLUSH",
    "SEE_THRU",
    "RIGHT_CLICK",
    "WRAP_CENTERED",
    "CHECK_LIKE",
    "HOTKEY_TEXT",
    "USE_OVERLAY_STATES",
    "NOT_READY",
    "FLASHING",
    "ALWAYS_COLOR",
    "ON_MOUSE_DOWN",
    nullptr,
};

const char *s_WindowStyleNames[] = {
    "PUSHBUTTON",
    "RADIOBUTTON",
    "CHECKBOX",
    "VERTSLIDER",
    "HORZSLIDER",
    "SCROLLLISTBOX",
    "ENTRYFIELD",
    "STATICTEXT",
    "PROGRESSBAR",
    "USER",
    "MOUSETRACK",
    "ANIMATED",
    "TABSTOP",
    "TABCONTROL",
    "TABPANE",
    "COMBOBOX",
    nullptr,
};

bool Parse_Bit_Flag(char *flag_string, unsigned int *bits, const char **flag_list);
void Parse_Bit_String(char *in_buffer, unsigned int *bits, const char **flag_list);
bool Parse_Color(int *color, char *buffer);
GameWindow *Pop_Window();
GameWindow *Push_Window(GameWindow *window);
void Set_Window_Text(GameWindow *window, Utf8String label);
void Read_Until_Semicolon(File *file, char *buffer, int max_buf_len);

int Scan_Int(const char *buffer, int &value);
int Scan_Unsigned_Int(const char *buffer, unsigned int &value);
int Scan_Bool(const char *buffer, bool &value);
int Scan_Short(const char *buffer, short &value);

bool Parse_Tooltip(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    // TODO wtf, why is this even a thing..

    Utf16String text;

    text.Set(L"Need tooltip translation");
    inst_data->Set_Tooltip_Text(text);

    return true;
}

bool Parse_Image_Offset(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    const char *delim = " \t\n\r";

    inst_data->m_imageOffset.x = atoi(strtok(buffer, delim));
    inst_data->m_imageOffset.y = atoi(strtok(nullptr, delim));

    return true;
}

bool Parse_Font(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    int point_size;
    int bold;

    char font_name[256];
    char *i;

    char *delim = " ,\n\r\t";

    strtok(buffer, delim);

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    strcpy(font_name, strtok(++i, ":,\n\r\t\""));

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), point_size);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), bold);

    if (g_theFontLibrary) {
        GameFont *font = g_theFontLibrary->Get_Font(font_name, point_size, bold != 0);

        if (font != nullptr) {
            inst_data->m_font = font;
        }
    }

    return true;
}

int Scan_Int(const char *buffer, int &value)
{
    return sscanf(buffer, "%d", value);
}

bool Parse_Name(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;
    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    inst_data->m_decoratedNameString = strtok(i + 1, "\"");

    if (g_theNameKeyGenerator != nullptr) {
        inst_data->m_id = g_theNameKeyGenerator->Name_To_Key(inst_data->m_decoratedNameString);
    }

    return true;
}

bool Parse_Status(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    inst_data->m_status = 0;

    Parse_Bit_String(buffer, &inst_data->m_status, s_WindowStatusNames);

    return true;
}

void Parse_Bit_String(char *in_buffer, unsigned int *bits, const char **flag_list)
{
    char buf[256];

    strcpy(buf, in_buffer);

    if (strncmp(buf, "NULL", (sizeof("NULL") - 1))) {

        for (char *flag_string = strtok(buf, "+"); flag_string != nullptr; flag_string = strtok(nullptr, "+")) {

            if (!Parse_Bit_Flag(flag_string, bits, flag_list)) {
                captainslog_error("ParseBitString: Invalid flag '%s'.\n", flag_string);
            }
        }
    }
}

bool Parse_Bit_Flag(char *flag_string, unsigned int *bits, const char **flag_list)
{
    unsigned char j = 0;

    for (const char **i = flag_list; *i != nullptr; ++i) {

        if (!_stricmp(*i, flag_string)) {
            *bits |= 1 << j;
            return true;
        }

        ++j;
    }

    return false;
}

bool Parse_Style(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    inst_data->m_style = 0;

    Parse_Bit_String(buffer, &inst_data->m_style, s_WindowStyleNames);

    return true;
}

WindowCallbackFunc s_systemFunc;
WindowCallbackFunc s_inputFunc;
WindowTooltipFunc s_tooltipFunc;
WindowDrawFunc s_drawFunc;

bool Parse_System_Callback(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    char *string = strtok(i + 1, "\"");

#if 0
    if ((!TheNameKeyGenerator || !TheFunctionLexicon) && !byte_4CAD190) {
        TheCurrentAllowCrashPtr = &byte_4CAD190;
        DebugCrash("Invalid singletons");
        TheCurrentAllowCrashPtr = 0;
    }
#endif

    s_theSystemString = string;
    s_systemFunc = (WindowCallbackFunc)g_theFunctionLexicon->Find_Function(
        g_theNameKeyGenerator->Name_To_Key(s_theSystemString), FunctionLexicon::TABLE_GAME_WIN_SYSTEM);

    return true;
}

bool Parse_Input_Callback(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    char *string = strtok(i + 1, "\"");

#if 0
    if ((!TheNameKeyGenerator || !TheFunctionLexicon) && !byte_4CAD190) {
        TheCurrentAllowCrashPtr = &byte_4CAD190;
        DebugCrash("Invalid singletons");
        TheCurrentAllowCrashPtr = 0;
    }
#endif

    s_theInputString = string;
    s_inputFunc = (WindowCallbackFunc)g_theFunctionLexicon->Find_Function(
        g_theNameKeyGenerator->Name_To_Key(s_theInputString), FunctionLexicon::TABLE_GAME_WIN_INPUT);

    return true;
}

bool Parse_Tooltip_Callback(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    char *string = strtok(i + 1, "\"");

#if 0
    if ((!TheNameKeyGenerator || !TheFunctionLexicon) && !byte_4CAD190) {
        TheCurrentAllowCrashPtr = &byte_4CAD190;
        DebugCrash("Invalid singletons");
        TheCurrentAllowCrashPtr = 0;
    }
#endif

    s_theTooltipString = string;
    s_tooltipFunc = (WindowTooltipFunc)g_theFunctionLexicon->Find_Function(
        g_theNameKeyGenerator->Name_To_Key(s_theTooltipString), FunctionLexicon::TABLE_GAME_WIN_TOOLTIP);

    return true;
}

bool Parse_Draw_Callback(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    char *string = strtok(i + 1, "\"");

#if 0
    if ((!TheNameKeyGenerator || !TheFunctionLexicon) && !byte_4CAD190) {
        TheCurrentAllowCrashPtr = &byte_4CAD190;
        DebugCrash("Invalid singletons");
        TheCurrentAllowCrashPtr = 0;
    }
#endif

    s_theDrawString = string;
    s_drawFunc = g_theFunctionLexicon->Game_Win_Draw_Func(
        g_theNameKeyGenerator->Name_To_Key(s_theDrawString), FunctionLexicon::TABLE_ANY);

    return true;
}

bool Parse_Header_Template(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    char *string = strtok(i + 1, "\"");

#if 0
    if ((!TheNameKeyGenerator || !TheFunctionLexicon) && !byte_4CAD190) {
        TheCurrentAllowCrashPtr = &byte_4CAD190;
        DebugCrash("Invalid singletons");
        TheCurrentAllowCrashPtr = 0;
    }
#endif

    inst_data->m_headerTemplateString = string;

    return true;
}

bool Parse_Listbox_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *delim = " :,\n\r\t";

    _ListboxData *d = (_ListboxData *)data;

    strtok(buffer, delim);

    Scan_Short(strtok(nullptr, delim), &d->listLength);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->autoScroll);

    const char *s = strtok(nullptr, delim);

    if (!_stricmp(s, "ScrollIfAtEnd")) {

        Scan_Bool(strtok(nullptr, delim), &d->ScrollIfAtEnd);

        strtok(nullptr, delim);

    } else {
        d->ScrollIfAtEnd = 0;
    }

    Scan_Bool(strtok(nullptr, delim), &d->autoPurge);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->scrollBar);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->multiSelect);

    strtok(nullptr, delim);

    Scan_Short(strtok(nullptr, delim), &d->columns);

    if (d->columns > 1) {
        d->columnWidthPercentage = (int *)operator new(4 * d->columns,
            "C:\\projects\\ZeroHour\\code\\GameEngine\\Source\\GameClient\\GUI\\GameWindowManagerScript.cpp",
            0x35F);

        for (int i = 0; i < d->columns; ++i) {
            strtok(nullptr, delim);

            Scan_Int(strtok(nullptr, delim), &d->columnWidthPercentage[i]);
        }
    } else {
        d->columnWidthPercentage = 0;
    }

    d->columnWidth = 0;

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), d->forceSelect);

    return true;
}

int Scan_Bool(const char *buffer, bool &value)
{

    int v = 0;

    int res = sscanf(buffer, "%d", &v);
    value = v != 0;
    return res;
}

int Scan_Short(const char *buffer, short &value)
{

    int v = 0;

    int res = sscanf(buffer, "%d", &v);
    value = v;
    return res;
}

bool Parse_Combo_Box_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *delim = " :,\n\r\t";

    _ComboBoxData *d = (_ComboBoxData *)data;

    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *Src; // ST10_4

    strtok(buffer, delim);

    Scan_Bool(strtok(nullptr, delim), d->isEditable);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->maxChars);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->maxDisplay);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), d->asciiOnly);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), d->lettersAndNumbersOnly);

    return true;
}

bool Parse_Slider_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *delim = " :,\n\r\t";

    _SliderData *d = (_SliderData *)data;

    strtok(buffer, delim);

    Scan_Int(strtok(nullptr, delim), d->minVal);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->maxVal);

    return true;
}

bool Parse_Radio_Button_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *delim = " :,\n\r\t";

    _RadioButtonData *d = (_RadioButtonData *)data;

    strtok(buffer, delim);

    Scan_Int(strtok(nullptr, delim), d->group);

    return true;
}

bool Parse_Tooltip_Text(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i = buffer;

    char *delim = "\n\r\t\"";

    while (*i != '"') {
        ++i;
    }

    if (strlen(++i) == 1) {
        return true;
    }

    char *label = strtok(i, delim);

    if (strlen(label) >= 128) {
        captainslog_error("TextTooltip label '%s' is too long, max is '%d'", label, 128);
        return false;
    }

    inst_data->m_tooltipString.Set(label);

    inst_data->Set_Tooltip_Text(g_theGameText->Fetch(label, nullptr));

    return true;
}

bool Parse_Tooltip_Delay(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    Scan_Int(strtok(buffer, " :,\n\r\t"), inst_data->m_tooltipDelay);

    return true;
}

bool Parse_Text(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    char *i;

    for (i = buffer; *i != '"'; ++i) {
        ;
    }

    char *label = strtok(i + 1, "\n\r\t\"");
    if (strlen(label) >= 128) {
        captainslog_error("Text label '%s' is too long, max is '%d'", label, 128);
        return false;
    }
    inst_data->m_textLabelString = label;

    return true;
}

bool Parse_Text_Color(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    TextDrawData *text_data;
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;

    char *delim = " :,\n\r\t";

    bool b = true;

    for (int i = 0; i < 3; ++i) {
        switch (i) {
            case 0:
                text_data = &inst_data->m_enabledText;
                break;
            case 1:
                text_data = &inst_data->m_disabledText;
                break;
            case 2:
                text_data = &inst_data->m_hiliteText;
                break;
            default:
                captainslog_error("Undefined state for text color\n");
                return false;
        }

        if (b == 1) {
            strtok(buffer, delim);
        } else {
            strtok(nullptr, delim);
        }

        b = false;

        Scan_Unsigned_Int(strtok(nullptr, delim), red);
        Scan_Unsigned_Int(strtok(nullptr, delim), green);
        Scan_Unsigned_Int(strtok(nullptr, delim), blue);
        Scan_Unsigned_Int(strtok(nullptr, delim), alpha);

        text_data->color = Make_Color(red, green, blue, alpha);

        strtok(nullptr, delim);

        Scan_Unsigned_Int(strtok(nullptr, delim), red);
        Scan_Unsigned_Int(strtok(nullptr, delim), green);
        Scan_Unsigned_Int(strtok(nullptr, delim), blue);
        Scan_Unsigned_Int(strtok(nullptr, delim), alpha);

        text_data->borderColor = Make_Color(red, green, blue, alpha);
    }

    return true;
}

int Scan_Unsigned_Int(const char *buffer, unsigned int &value)
{
    return sscanf(buffer, "%d", value);
}

bool Parse_Static_Text_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    const char *delim = " :,\n\r\t";

    _TextData *d = (_TextData *)data;

    strtok(buffer, delim);

    Scan_Bool(strtok(nullptr, delim), &d->centered);

    d->field_5 = 1;
    d->dwfield_8 = 7;
    d->dwfield_C = 7;

    return true;
}

bool Parse_Text_Entry_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    const char *delim = " :,\n\r\t";

    _EntryData *d = (_EntryData *)data;

    strtok(buffer, delim);

    Scan_Short(strtok(nullptr, delim), &d->maxTextLen);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->secretText);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->numericalOnly);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->alphaNumericalOnly);

    strtok(nullptr, delim);

    Scan_Bool(strtok(nullptr, delim), &d->aSCIIOnly);

    return true;
}

bool Parse_Tab_Control_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    _TabControlData *d = (_TabControlData *)data;

    const char *delim = " :,\n\r\t";

    strtok(buffer, delim);

    Scan_Int(strtok(nullptr, delim), d->tabOrientation);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->tabEdge);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->tabWidth);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->tabHeight);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->tabCount);

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), d->paneBorder);

    int v11 = 0;

    strtok(nullptr, delim);

    Scan_Int(strtok(nullptr, delim), v11);

    for (int i = 0; i < v11; ++i) {
        Scan_Bool(strtok(nullptr, delim), d->subPaneDisabled[i]);
    }

    return true;
}

WinDrawData s_hiliteSliderThumbDrawData[9];
WinDrawData s_disabledSliderThumbDrawData[9];
WinDrawData s_enabledSliderThumbDrawData[9];
WinDrawData s_hiliteSliderDrawData[9];
WinDrawData s_disabledSliderDrawData[9];
WinDrawData s_enabledSliderDrawData[9];
WinDrawData s_hiliteDownButtonDrawData[9];
WinDrawData s_disabledDownButtonDrawData[9];
WinDrawData s_enabledDownButtonDrawData[9];
WinDrawData s_hiliteUpButtonDrawData[9];
WinDrawData s_disabledUpButtonDrawData[9];
WinDrawData s_enabledUpButtonDrawData[9];
WinDrawData s_hiliteListBoxDrawData[9];
WinDrawData s_disabledListBoxDrawData[9];
WinDrawData s_enabledListBoxDrawData[9];
WinDrawData s_hiliteEditBoxDrawData[9];
WinDrawData s_disabledEditBoxDrawData[9];
WinDrawData s_enabledEditBoxDrawData[9];
WinDrawData s_hiliteDropDownButtonDrawData[9];
WinDrawData s_disabledDropDownButtonDrawData[9];
WinDrawData s_enabledDropDownButtonDrawData[9];

bool Parse_Draw_Data(char *token, WinInstanceData *inst_data, char *buffer, void *data)
{
    WinDrawData *draw_data;

    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;

    bool b = true;

    const char *delim = " :,\n\r\t";

    for (int i = 0; i < 9; ++i) {

        if (!strcmp(token, "ENABLEDDRAWDATA")) {
            draw_data = &inst_data->m_enabledDrawData[i];

        } else if (!strcmp(token, "DISABLEDDRAWDATA")) {
            draw_data = &inst_data->m_disabledDrawData[i];

        } else if (!strcmp(token, "HILITEDRAWDATA")) {
            draw_data = &inst_data->m_hiliteDrawData[i];

        } else if (!strcmp(token, "LISTBOXENABLEDUPBUTTONDRAWDATA")) {
            draw_data = &s_enabledUpButtonDrawData[i];

        } else if (!strcmp(token, "LISTBOXDISABLEDUPBUTTONDRAWDATA")) {
            draw_data = &s_disabledUpButtonDrawData[i];

        } else if (!strcmp(token, "LISTBOXHILITEUPBUTTONDRAWDATA")) {
            draw_data = &s_hiliteUpButtonDrawData[i];

        } else if (!strcmp(token, "LISTBOXENABLEDDOWNBUTTONDRAWDATA")) {
            draw_data = &s_enabledDownButtonDrawData[i];

        } else if (!strcmp(token, "LISTBOXDISABLEDDOWNBUTTONDRAWDATA")) {
            draw_data = &s_disabledDownButtonDrawData[i];

        } else if (!strcmp(token, "LISTBOXHILITEDOWNBUTTONDRAWDATA")) {
            draw_data = &s_hiliteDownButtonDrawData[i];

        } else if (!strcmp(token, "LISTBOXENABLEDSLIDERDRAWDATA")) {
            draw_data = &s_enabledSliderDrawData[i];

        } else if (!strcmp(token, "LISTBOXDISABLEDSLIDERDRAWDATA")) {
            draw_data = &s_disabledSliderDrawData[i];

        } else if (!strcmp(token, "LISTBOXHILITESLIDERDRAWDATA")) {
            draw_data = &s_hiliteSliderDrawData[i];

        } else if (!strcmp(token, "SLIDERTHUMBENABLEDDRAWDATA")) {
            draw_data = &s_enabledSliderThumbDrawData[i];

        } else if (!strcmp(token, "SLIDERTHUMBDISABLEDDRAWDATA")) {
            draw_data = &s_disabledSliderThumbDrawData[i];

        } else if (!strcmp(token, "SLIDERTHUMBHILITEDRAWDATA")) {
            draw_data = &s_hiliteSliderThumbDrawData[i];

        } else if (!strcmp(token, "COMBOBOXDROPDOWNBUTTONENABLEDDRAWDATA")) {
            draw_data = &s_enabledDropDownButtonDrawData[i];

        } else if (!strcmp(token, "COMBOBOXDROPDOWNBUTTONDISABLEDDRAWDATA")) {
            draw_data = &s_disabledDropDownButtonDrawData[i];

        } else if (!strcmp(token, "COMBOBOXDROPDOWNBUTTONHILITEDRAWDATA")) {
            draw_data = &s_hiliteDropDownButtonDrawData[i];

        } else if (!strcmp(token, "COMBOBOXEDITBOXENABLEDDRAWDATA")) {
            draw_data = &s_enabledEditBoxDrawData[i];

        } else if (!strcmp(token, "COMBOBOXEDITBOXDISABLEDDRAWDATA")) {
            draw_data = &s_disabledEditBoxDrawData[i];

        } else if (!strcmp(token, "COMBOBOXEDITBOXHILITEDRAWDATA")) {
            draw_data = &s_hiliteEditBoxDrawData[i];

        } else if (!strcmp(token, "COMBOBOXLISTBOXENABLEDDRAWDATA")) {
            draw_data = &s_enabledListBoxDrawData[i];

        } else if (!strcmp(token, "COMBOBOXLISTBOXDISABLEDDRAWDATA")) {
            draw_data = &s_disabledListBoxDrawData[i];

        } else if (!strcmp(token, "COMBOBOXLISTBOXHILITEDRAWDATA")) {
            draw_data = &s_hiliteListBoxDrawData[i];

        } else {
            captainslog_error("undefined token '%s'", token);
            return false;
        }

        if (b) {
            strtok(buffer, delim);
        } else {
            strtok(nullptr, delim);
        }

        b = false;

        char *s = strtok(nullptr, delim);

        if (strcmp(s, "NoImage")) {
            draw_data->image = g_theMappedImageCollection->Find_Image_By_Name(Utf8String(s));
        } else {
            draw_data->image = nullptr;
        }

        strtok(nullptr, delim);

        Scan_Unsigned_Int(strtok(nullptr, delim), red);
        Scan_Unsigned_Int(strtok(nullptr, delim), green);
        Scan_Unsigned_Int(strtok(nullptr, delim), blue);
        Scan_Unsigned_Int(strtok(nullptr, delim), alpha);

        draw_data->color = Make_Color(red, green, blue, alpha);

        strtok(nullptr, delim);

        Scan_Unsigned_Int(strtok(nullptr, delim), red);
        Scan_Unsigned_Int(strtok(nullptr, delim), green);
        Scan_Unsigned_Int(strtok(nullptr, delim), blue);
        Scan_Unsigned_Int(strtok(nullptr, delim), alpha);

        draw_data->borderColor = Make_Color(red, green, blue, alpha);
    }

    return true;
}

void *Get_Data_Template(char *type)
{
    static _SliderData slider_data;
    static _ListboxData listbox_data;
    static _TabControlData tabctrl_data;
    static _EntryData entry_field_data;
    static _TextData static_text_data;
    static _RadioButtonData radio_box_data;
    static _ComboBoxData combobox_data;

    if (!strcmp(type, "VERTSLIDER") || !strcmp(type, "HORZSLIDER")) {
        memset(&slider_data, 0, sizeof(slider_data));
        return &slider_data;
    }

    if (!strcmp(type, "SCROLLLISTBOX")) {
        memset(&listbox_data, 0, sizeof(listbox_data));
        return &listbox_data;
    }

    if (!strcmp(type, "TABCONTROL")) {
        memset(&tabctrl_data, 0, sizeof(tabctrl_data));
        return &tabctrl_data;
    }

    if (!strcmp(type, "ENTRYFIELD")) {
        memset(&entry_field_data, 0, sizeof(entry_field_data));
        return &entry_field_data;
    }

    if (!strcmp(type, "STATICTEXT")) {
        memset(&static_text_data, 0, sizeof(static_text_data));
        return &static_text_data;
    }

    if (!strcmp(type, "RADIOBUTTON")) {
        memset(&radio_box_data, 0, sizeof(radio_box_data));
        return &radio_box_data;
    }

    if (!strcmp(type, "COMBOBOX")) {
        memset(&combobox_data, 0, sizeof(combobox_data));
        return &combobox_data;
    }

    return nullptr;
}

bool Parse_Layout_Block(File *file, char *buffer, unsigned int version, WindowLayoutInfo *info)
{
    char *v5; // ST20_4
    char *v6; // eax
    char **i; // [esp+14h] [ebp-114h]
    char buf[256]; // [esp+1Ch] [ebp-10Ch]
    int trash; // [esp+124h] [ebp-4h]

    Utf8String string;

    if (!file->Scan_String(string)) {
        return false;
    }

    if (!AsciiString::compare(&string, "STARTLAYOUTBLOCK")) {
    LABEL_5:
        while (true) {
            file->vtable->scanString(file, &string);
            if (!AsciiString::compare(&string, "ENDLAYOUTBLOCK")) {
                break;
            }
            for (i = &layoutScriptTable; i && *i; i += 2) {

                if (!string.Compare(*i)) {

                    Read_Until_Semicolon(file, buffer, 2048);

                    v5 = strtok(buffer, " =");
                    strcpy(buf, string.Str());

                    if (((int(__cdecl *)(char *, char *, int, WindowLayoutInfo *))i[1])(buf, v5, version, info) & 0xFF) {
                        goto LABEL_5;
                    }

                    return false;
                }
            }
        }

        return true;
    }

    return false;
}

void Read_Until_Semicolon(File *file, char *buffer, int max_buf_len)
{
    int i = 0;
    bool b = true;

    while (i < max_buf_len) {
        file->Read(&buffer[i], 1);
        if (isspace(buffer[i])) {
            if (!b) {
                buffer[i++] = ' ';
            }
        } else {
            b = false;
            if (buffer[i] == ';') {
                buffer[i] = 0;
                return;
            }
            ++i;
        }
    }

    captainslog_error("Read buffer overflow - input truncated.\n");
    buffer[max_buf_len - 1] = 0;
}

WindowLayout *GameWindowManager::Win_Create_Layout(Utf8String filename)
{
    WindowLayout *layout = new WindowLayout;

    if (!layout->Load(filename)) {
        layout->Delete_Instance();
        return nullptr;
    }

    return layout;
}

void GameWindowManager::Free_Static_Strings()
{
    s_theSystemString.Clear();
    s_theInputString.Clear();
    s_theTooltipString.Clear();
    s_theDrawString.Clear();
}

GameWindow *GameWindowManager::Win_Create_From_Script(Utf8String filename, WindowLayoutInfo *info)
{
    // TODO
#ifdef GAME_DLL
    return Call_Method<GameWindow *, GameWindowManager, Utf8String, WindowLayoutInfo *>(
        PICK_ADDRESS(0xBAAAAAD, 0), this, filename, info);
#else
    return nullptr;
#endif
}

GameWindow **s_stackPtr;
GameWindow *s_windowStack[10];

void Reset_Window_Stack()
{
    memset(s_windowStack, 0, sizeof(s_windowStack));
    s_stackPtr = s_windowStack;
}

int s_defEnabledColor;
int s_defDisabledColor;
int s_defBackgroundColor;
int s_defHiliteColor;
int s_defSelectedColor;
int s_defTextColor;
GameFont *s_defFont;

void Reset_Window_Defaults()
{
    s_defEnabledColor = 0;
    s_defDisabledColor = 0;
    s_defBackgroundColor = 0;
    s_defHiliteColor = 0;
    s_defSelectedColor = 0;
    s_defTextColor = 0;
    s_defFont = nullptr;
}

bool Parse_Default_Color(int *color, File *file, char *buffer)
{
    Utf8String string;

    file->Scan_String(string);

    Read_Until_Semicolon(file, buffer, 2048);

    if (!strcmp(buffer, "TRANSPARENT")) {
        *color = 0xFFFFFF;
    } else {
        Parse_Color(color, buffer);
    }

    return true;
}

bool Parse_Color(int *color, char *buffer)
{
    const char *delim = " \t\n\r";

    unsigned char r = atoi(strtok(buffer, delim));
    unsigned char g = atoi(strtok(nullptr, delim));
    unsigned char b = atoi(strtok(nullptr, delim));

    *color = g_theWindowManager->Win_Make_Color(r, g, b, 255);

    return true;
}

bool Parse_Default_Font(GameFont *font, File *file, char *buffer)
{
    Utf8String string;

    file->Scan_String(string);

    Read_Until_Semicolon(file, buffer, 2048);

    return true;
}

GameWindow *Peek_Window()
{
    if (s_stackPtr == s_windowStack) {
        return nullptr;
    }

    return s_stackPtr[-1];
}

bool Parse_Screen_Rect(char *token, char *buffer, int *x, int *y, int *width, int *height)
{
    ICoord2D parent_screen_pos;
    IRegion2D screen_region;
    ICoord2D create_res;

    GameWindow *window = Peek_Window();

    const char *delim = " ,:=\n\r\t";

    strtok(nullptr, delim);

    char *s = strtok(nullptr, delim);
    Scan_Int(s, &screen_region.lo.x);

    s = strtok(nullptr, delim);
    Scan_Int(s, &screen_region.lo.y);

    s = strtok(nullptr, delim);

    s = strtok(nullptr, delim);
    Scan_Int(s, &screen_region.hi.x);

    s = strtok(nullptr, delim);
    Scan_Int(s, &screen_region.hi.y);

    s = strtok(nullptr, delim);

    s = strtok(nullptr, delim);
    Scan_Int(s, &create_res.x);

    s = strtok(nullptr, delim);
    Scan_Int(s, &create_res.y);

    float x_scale = (float)g_theDisplay->Get_Width() / (float)create_res.x;
    float y_scale = (float)g_theDisplay->Get_Width() / (float)create_res.y;

    screen_region.lo.x = (int)((float)screen_region.lo.x * x_scale);
    screen_region.lo.y = (int)((float)screen_region.lo.y * y_scale);

    screen_region.hi.x = (int)((float)screen_region.hi.x * x_scale);
    screen_region.hi.y = (int)((float)screen_region.hi.y * y_scale);

    if (window != nullptr) {
        window->Win_Get_Screen_Position(&parent_screen_pos.x, &parent_screen_pos.y);
        *x = screen_region.lo.x - parent_screen_pos.x;
        *y = screen_region.lo.y - parent_screen_pos.y;
    } else {
        *x = screen_region.lo.x;
        *y = screen_region.lo.y;
    }

    *width = screen_region.hi.x - screen_region.lo.x;
    *height = screen_region.hi.y - screen_region.lo.y;

    return true;
}

bool Parse_Data(void **data, char *type, char *buffer)
{
    const char *delim = " \t\n\r";

    static _SliderData slider_data;
    static _ListboxData listbox_data;
    static _EntryData entry_field_data;
    static _TextData static_text_data;
    static _RadioButtonData radio_box_data;

    if (!strcmp(type, "VERTSLIDER") || !strcmp(type, "HORZSLIDER")) {
        memset(&slider_data, 0, sizeof(_SliderData));
        slider_data.minVal = atoi(strtok(buffer, delim));
        slider_data.maxVal = atoi(strtok(nullptr, delim));
        *data = &slider_data;
        return true;
    }

    if (!strcmp(type, "SCROLLLISTBOX")) {
        memset(&listbox_data, 0, sizeof(_ListboxData));
        listbox_data.listLength = atoi(strtok(buffer, delim));
        listbox_data.autoScroll = atoi(strtok(nullptr, delim)) != 0;
        listbox_data.autoPurge = atoi(strtok(nullptr, delim)) != 0;
        listbox_data.scrollBar = atoi(strtok(nullptr, delim)) != 0;
        listbox_data.multiSelect = atoi(strtok(nullptr, delim)) != 0;
        listbox_data.forceSelect = atoi(strtok(nullptr, delim)) != 0;
        *data = &listbox_data;
        return true;
    }

    if (!strcmp(type, "ENTRYFIELD")) {
        memset(&entry_field_data, 0, sizeof(_EntryData));

        entry_field_data.maxTextLen = atoi(strtok(buffer, delim));
        strtok(nullptr, delim);

        const char *secret_txt = strtok(nullptr, delim);

        if (secret_txt) {
            entry_field_data.secretText = atoi(secret_txt) != 0;
            entry_field_data.secretText = entry_field_data.secretText != 0;
        } else {
            entry_field_data.secretText = 0;
        }

        const char *flags = strtok(nullptr, delim);

        if (flags != nullptr) {
            entry_field_data.numericalOnly = atoi(flags) == 1;
            entry_field_data.alphaNumericalOnly = atoi(flags) == 2;
            entry_field_data.aSCIIOnly = atoi(flags) == 3;
        } else {
            entry_field_data.numericalOnly = 0;
            entry_field_data.alphaNumericalOnly = 0;
            entry_field_data.aSCIIOnly = 0;
        }

        *data = &entry_field_data;
        return true;
    }
    if (!strcmp(type, "STATICTEXT")) {
        static_text_data.centered = atoi(strtok(buffer, delim)) != 0;
        static_text_data.centered = static_text_data.centered != 0;
        strtok(nullptr, delim);
        *data = &static_text_data;
        return true;
    }

    if (!strcmp(type, "RADIOBUTTON")) {
        radio_box_data.group = atoi(strtok(buffer, delim));
        *data = &radio_box_data;
        return true;
    }

    *data = nullptr;

    return true;
}

GameWindow *Create_Window(char *type,
    int id,
    int status,
    int x,
    int y,
    int width,
    int height,
    WinInstanceData *inst_data,
    void *data,
    WindowCallbackFunc system,
    WindowCallbackFunc input,
    WindowTooltipFunc tooltip,
    WindowDrawFunc draw)
{
    GameWindow *window = nullptr;

    GameWindow *parent = Peek_Window();

    if (!strcmp(type, "USER")) {
        window = g_theWindowManager->Win_Create(parent, status, x, y, width, height, system, nullptr);

        if (window != nullptr) {
            inst_data->m_style |= GWS_USER;
            window->Win_Set_Instance_Data(inst_data);
            window->Win_Set_Window_Id(id);
        }

    } else if (!strcmp(type, "TABPANE")) {
        window = g_theWindowManager->Win_Create(parent, status, x, y, width, height, system, nullptr);

        if (window != nullptr) {
            inst_data->m_style |= GWS_TAB_PLANE;
            window->Win_Set_Instance_Data(inst_data);
            window->Win_Set_Window_Id(id);
        }

    } else {
        window = Create_Gadget(type, parent, status, x, y, width, height, inst_data, data);

        if (window != nullptr) {
            window->Win_Set_Window_Id(id);
        }
    }

    if (window != nullptr) {
        if (system != nullptr) {
            window->Win_Set_System_Func(system);
        }
        if (input != nullptr) {
            window->Win_Set_Input_Func(input);
        }
        if (tooltip != nullptr) {
            window->Win_Set_Tooltip_Func(tooltip);
        }
        if (draw != nullptr) {
            window->Win_Set_Draw_Func(draw);
        }

        GameWindowEditData *edit_data = window->Win_Get_Edit_Data();

        if (edit_data != nullptr) {
            edit_data->system_callback_string = s_theSystemString;
            edit_data->input_callback_string = s_theInputString;
            edit_data->tooltip_callback_string = s_theTooltipString;
            edit_data->draw_callback_string = s_theDrawString;
        }
    }

    if (window != nullptr) {
        Set_Window_Text(window, inst_data->m_textLabelString);
    }

    if (window != nullptr && parent != nullptr) {
        g_theWindowManager->Win_Send_Input_Msg(parent, 0x16, id, 0);
    }

    return window;
}

void Set_Window_Text(GameWindow *window, Utf8String label)
{
    if (label.Is_Empty()) {
        return;
    }

    Utf16String text;
    Utf16String translated;

    text = g_theGameText->Fetch(label.Str(), nullptr);

    if (window->Win_Get_Style() & GWS_PUSH_BUTTON) {
        GadgetButtonSetText(window, text);
        return;
    }

    if (window->Win_Get_Style() & GWS_RADIO_BUTTON) {
        GadgetRadioSetText(window, text);
        return;
    }

    if (window->Win_Get_Style() & GWS_CHECK_BOX) {
        GadgetCheckBoxSetText(window, text);
        return;
    }

    if (window->Win_Get_Style() & GWS_STATIC_TEXT) {
        GadgetStaticTextSetText(window, text);
        return;
    }

    if (window->Win_Get_Style() & GWS_TEXT_ENTRY_FIELD) {
        translated.Translate(label);
        GadgetTextEntrySetText(window, translated);
        return;
    }

    window->Win_Set_Text(text);
}

bool Parse_Child_Windows(GameWindow *window, File *file, char *buffer)
{
    GameWindow *next;
    Utf8String string;

    // TODO is this correct? makes no sense
    if (window->Win_Get_Style() & GWS_TAB_CONTROL) {
        for (GameWindow *i = window->Win_Get_Child(); i != nullptr; i = next) {
            next = i->Win_Get_Next();
            g_theWindowManager->Win_Destroy(i);
        }
    }

    Push_Window(window);

    while (file->Scan_String(string) && string.Compare("ENDALLCHILDREN") && string.Compare("END")) {

        if (!string.Compare("ENABLEDCOLOR")) {
            if (!Parse_Default_Color(&s_defEnabledColor, file, buffer)) {
                return false;
            }

        } else if (!string.Compare("DISABLEDCOLOR")) {
            if (!Parse_Default_Color(&s_defDisabledColor, file, buffer)) {
                return false;
            }

        } else if (!string.Compare("HILITECOLOR")) {
            if (!Parse_Default_Color(&s_defHiliteColor, file, buffer)) {
                return false;
            }

        } else if (!string.Compare("SELECTEDCOLOR")) {
            if (!Parse_Default_Color(&s_defSelectedColor, file, buffer)) {
                return false;
            }

        } else if (!string.Compare("TEXTCOLOR")) {
            if (!Parse_Default_Color(&s_defTextColor, file, buffer)) {
                return false;
            }

        } else if (!string.Compare("WINDOW") && !Parse_Window(file, buffer)) {
            return false;
        }
    }

    if (Pop_Window() != window) {
        captainslog_error("Unmatched window on stack. Corrupt stack or bad source.");
        return false;
    }
    if (window->Win_Get_Style() & GWS_TAB_CONTROL) {
        GadgetTabControlFixupSubPaneList(window);
    }

    return true;
}

GameWindow *Pop_Window()
{
    if (s_stackPtr == s_windowStack) {
        return nullptr;
    }

    --s_stackPtr;

    return *s_stackPtr;
}

GameWindow *Push_Window(GameWindow *window)
{
    if (s_stackPtr == &s_windowStack[9]) {
        captainslog_warn("Warning, stack overflow\n");
        return nullptr;
    }

    *s_stackPtr = window;
    ++s_stackPtr;

    return *s_stackPtr;
}