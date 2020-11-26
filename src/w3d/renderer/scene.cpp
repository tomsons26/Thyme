/**
 * @file
 *
 * @author tomsons26
 *
 * @brief scene classes
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scene.h"
#include "dx8wrapper.h"
#include "w3d.h"

SceneClass::SceneClass() :
    m_ambientLight(0.5f, 0.5f, 0.5f),
    m_polyRenderMode(FILL),
    m_extraPassPolyRenderMode(EXTRA_PASS_DISABLE),
    m_fogEnabled(false),
    m_fogColor(0.0f, 0.0f, 0.0f),
    m_fogStart(0.0f),
    m_fogEnd(1000.0f)
{
}

void SceneClass::Save(ChunkSaveClass &csave)
{
    captainslog_error("Unimplemented function called!");
}

void SceneClass::Load(ChunkLoadClass &cload)
{
    captainslog_error("Unimplemented function called!");
}

void SceneClass::Render(RenderInfoClass &rinfo)
{
    Pre_Render_Processing(rinfo);
    DX8Wrapper::Set_Fog(m_fogEnabled, m_fogColor, m_fogStart, m_fogEnd);

    if (m_extraPassPolyRenderMode == EXTRA_PASS_DISABLE) {
        Customized_Render(rinfo);
    } else {

        bool state = W3D::Is_Texturing_Enabled();
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0); // BFME2 calls Set_DX8_ZBias(0.0f) here
        Customized_Render(rinfo);

        if (m_extraPassPolyRenderMode == EXTRA_PASS_LINE) {
            W3D::Enable_Texturing(false);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 7); // BFME2 calls Set_DX8_ZBias(7.0f) here
            Customized_Render(rinfo);
        } else if (m_extraPassPolyRenderMode == EXTRA_PASS_LINE) {
            DX8Wrapper::Clear(true, false, { 0, 0, 0 }, 0.0f);
            W3D::Enable_Texturing(false);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 7); //BFME2 calls Set_DX8_ZBias(7.0f) here
            Customized_Render(rinfo);
        }

        W3D::Enable_Texturing(state);
    }

    Post_Render_Processing(rinfo);
}

SimpleSceneClass::SimpleSceneClass() : m_visibilityChecked(false) {}
