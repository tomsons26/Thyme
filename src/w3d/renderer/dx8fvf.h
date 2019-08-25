#pragma once

#ifndef DX8FVF_H
#define DX8FVF_H

#include "w3dmpo.h"
#include "wwstring.h"
#include "d3d8types.h"
enum
{
    DX8_FVF_XYZ = 0x2,
    DX8_FVF_XYZN = 0x12,
    DX8_FVF_XYZNUV1 = 0x112,
    DX8_FVF_XYZNUV2 = 0x212,
    DX8_FVF_XYZNDUV1 = 0x152,
    DX8_FVF_XYZNDUV2 = 0x252,
    DX8_FVF_XYZDUV1 = 0x142,
    DX8_FVF_XYZDUV2 = 0x242,
    DX8_FVF_XYZUV1 = 0x102,
    DX8_FVF_XYZUV2 = 0x202,
};

inline unsigned int Get_FVF_Vertex_Size(unsigned int fvf)
{
    return 0;//D3DXGetFVFVertexSize(fvf);
}

class FVFInfoClass : public W3DMPO
{
public:
    FVFInfoClass(unsigned int fvf, unsigned int size);
    unsigned int Get_FVF() { return m_FVF; }
    unsigned int Get_FVF_Size() { return m_fvf_size; }
    unsigned int Get_Location_Offset() { return m_location_offset; }
    unsigned int Get_Normal_Offset() { return m_normal_offset; }
    unsigned int Get_Blend_Offset() { return m_blend_offset; }
    unsigned int Get_Tex_Offset(unsigned int tex) { return m_texcoord_offset[tex]; }
    unsigned int Get_Diffuse_Offset() { return m_diffuse_offset; }
    unsigned int Get_Specular_Offset() { return m_specular_offset; }
    void Get_FVF_Name(StringClass &fvfname);

private:
    unsigned int m_FVF;
    unsigned int m_fvf_size;
    unsigned int m_location_offset;
    unsigned int m_normal_offset;
    unsigned int m_blend_offset;
    unsigned int m_texcoord_offset[8];
    unsigned int m_diffuse_offset;
    unsigned int m_specular_offset;
};

int dynamic_fvf_type = DX8_FVF_XYZNDUV2;
FVFInfoClass DynamicFVFInfo(dynamic_fvf_type, 0);

//+='s are likely sizeof's of structs in directxmath.h
FVFInfoClass::FVFInfoClass(unsigned int fvf, unsigned int size)
{
    m_FVF = fvf;
    m_fvf_size = fvf != 0 ? Get_FVF_Vertex_Size(fvf) : size;
    m_location_offset = 0;
    m_blend_offset = m_location_offset;
    if ((m_FVF & D3DFVF_XYZ) == D3DFVF_XYZ) {
        m_blend_offset += 0xC;
    }
    m_normal_offset = m_blend_offset;
    if ((m_FVF & D3DFVF_XYZB4) == D3DFVF_XYZB4 && (m_FVF & D3DFVF_LASTBETA_UBYTE4) == D3DFVF_LASTBETA_UBYTE4) {
        m_normal_offset += 0x10;
    }
    m_diffuse_offset = m_normal_offset;
    if ((m_FVF & D3DFVF_NORMAL) == D3DFVF_NORMAL) {
        m_diffuse_offset += 0xC;
    }
    m_specular_offset = m_diffuse_offset;
    if ((m_FVF & D3DFVF_DIFFUSE) == D3DFVF_DIFFUSE) {
        m_specular_offset += 4;
    }
    m_texcoord_offset[0] = m_specular_offset;
    if ((m_FVF & D3DFVF_SPECULAR) == D3DFVF_SPECULAR) {
        m_texcoord_offset[0] += 4;
    }
    for (int i = 1; i < 8; ++i) {
        m_texcoord_offset[i] = m_texcoord_offset[i - 1];
        // not sure what its doing here
        if (((3 << (i + 14)) & m_FVF) == 3 << (i + 14)) {
            m_texcoord_offset[i] += 4;
        } else {
            m_texcoord_offset[i] += 8;
        }
    }
}

// not used and quite a mess in ZH
void FVFInfoClass::Get_FVF_Name(StringClass &fvfname)
{
    fvfname = "Unknown!";
}

#endif // DX8FVF_H