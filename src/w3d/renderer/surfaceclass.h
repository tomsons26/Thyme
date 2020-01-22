/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 * @author tomsons26
 *
 * @brief Class for handling rendering surfaces.
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
#include "refcount.h"
#include "w3dformat.h"
#include "w3dmpo.h"
#include "vector3.h"
#include "w3dtypes.h"

class Vector2i;

class SurfaceClass : public W3DMPO, public RefCountClass
{
    IMPLEMENT_W3D_POOL(SurfaceClass)
public:
    struct SurfaceDescription
    {
        WW3DFormat format;
        unsigned width;
        unsigned height;
    };

public:
    SurfaceClass(w3dsurface_t d3d_surface);
    SurfaceClass(const char *name);
    SurfaceClass(unsigned width, unsigned height, WW3DFormat format);
    virtual ~SurfaceClass();

    void Get_Description(SurfaceDescription &surface_desc);
    void *Lock(int *pitch);
    void *Lock(int *pitch, bool discard);
    void *Lock_Rect(int *pitch, int left, int top, int right, int bottom);
    void Unlock();
    void Clear();
    void Copy(Vector2i &min, Vector2i &max, unsigned char *other);
    void Copy(unsigned char *other);
    void Copy(unsigned dst_x, unsigned dst_y, unsigned src_x, unsigned src_y, unsigned width, unsigned height,
        SurfaceClass *other);
    void Stretch_Copy(unsigned int dstx, unsigned int dsty, unsigned int dstwidth, unsigned int dstheight, unsigned int srcx,
        unsigned int srcy, unsigned int srcwidth, unsigned int srcheight, SurfaceClass *other);
    void FindBB(Vector2i *min, Vector2i *max);
    bool Is_Transparent_Column(unsigned column);
    unsigned char *Create_Copy(int *width, int *height, int *size, bool flip);
    w3dsurface_t Peek_D3D_Surface();
    void Attach(w3dsurface_t surface);
    void Detach();
    void Draw_Horizonal_Line(unsigned int y, unsigned int x1, unsigned int x2, unsigned int color);
    void Draw_Pixel(unsigned x, unsigned y, unsigned color);
    void Get_Pixel(Vector3 &rgb, int x, int y);
    void Hue_Shift(Vector3 &hsv_shift);
    unsigned char Is_Monochrome();
    WW3DFormat Get_Surface_Format() const { return m_surfaceFormat; }
    int Get_Surface_Memory_Usage();
    void Set_Alpha(int x, int y, unsigned char value);
    void Tint_Surface(float red_tint, float green_tint, float blue_tint);

    static int Pixel_Size(const SurfaceDescription &sd);

#ifdef GAME_DLL
    SurfaceClass *Hook_Ctor1(unsigned width, unsigned height, WW3DFormat format)
    {
        return new (this) SurfaceClass(width, height, format);
    }
    SurfaceClass *Hook_Ctor2(w3dsurface_t d3d_surface) { return new (this) SurfaceClass(d3d_surface); }
    SurfaceClass *Hook_Ctor3(const char *name) { return new (this) SurfaceClass(name); }
    void Hook_Dtor() { SurfaceClass::~SurfaceClass(); }
#endif

protected:
    w3dsurface_t m_d3dSurface;
    WW3DFormat m_surfaceFormat;
};
void __cdecl Convert_Pixel(/*<regrel ebp+0x4>*/ /*|0x4|*/ unsigned char* pixel, /*<regrel ebp+0x8>*/ /*|0x4|*/ struct SurfaceClass::SurfaceDescription& sd, /*<regrel ebp+0xc>*/ /*|0x4|*/ class Vector3& rgb);
void __cdecl Convert_Pixel(/*<regrel ebp+0x4>*/ /*|0x4|*/ class Vector3& rgb, /*<regrel ebp+0x8>*/ /*|0x4|*/ struct SurfaceClass::SurfaceDescription& sd, /*<regrel ebp+0xc>*/ /*|0x4|*/ unsigned char* pixel);
void Recolor(Vector3 &hsv, const Vector3 &hsv_shift);
