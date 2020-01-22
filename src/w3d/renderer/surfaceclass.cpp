#include "surfaceclass.h"
#include "dx8wrapper.h"
#include <algorithm>

SurfaceClass::SurfaceClass(w3dsurface_t d3d_surface)
{
    m_d3dSurface = nullptr;
    Attach(d3d_surface);

    // ren might not do this
    SurfaceDescription sd;
    Get_Description(sd);
    m_surfaceFormat = sd.format;
}

SurfaceClass::SurfaceClass(const char *name)
{
    m_d3dSurface = nullptr;
    m_d3dSurface = DX8Wrapper::Create_Surface(name);

    // ren might not do this
    SurfaceDescription sd;
    Get_Description(sd);
    m_surfaceFormat = sd.format;
}

SurfaceClass::SurfaceClass(unsigned width, unsigned height, WW3DFormat format)
{
    m_d3dSurface = nullptr;
    m_surfaceFormat = format;
    // assert(width);
    // assert(height);
    m_d3dSurface = DX8Wrapper::Create_Surface(width, height, format);
}

SurfaceClass::~SurfaceClass()
{
    Detach();
}

void SurfaceClass::Get_Description(SurfaceDescription &surface_desc)
{
    D3DSURFACE_DESC desc;
    memset(&desc, 0, sizeof(D3DSURFACE_DESC));
    m_d3dSurface->GetDesc(&desc);
    // v2 = v4->D3DSurface->vtable->GetDesc(desc);
    // DX8_ErrorCode(v2);
    surface_desc.format = D3DFormat_To_WW3DFormat(desc.Format);
    surface_desc.height = desc.Height;
    surface_desc.width = desc.Width;
}

void *SurfaceClass::Lock(int *pitch)
{
    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    m_d3dSurface->LockRect(&lock_rect, NULL, 0);
    // v2 = this->D3DSurface->lpVtbl->LockRect(this->D3DSurface, &lock_rect, NULL, 0);
    // if ( v2 ) {
    // Log_DX8_ErrorCode(v2);
    //}
    *pitch = lock_rect.Pitch;
    return lock_rect.pBits;
}

void *SurfaceClass::Lock(int *pitch, bool discard)
{
    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = nullptr;
    DWORD flags = discard != 0 ? D3DLOCK_DISCARD : 0;
    // BYTE1(flags) |= 8u;
    flags |= D3DLOCK_NOSYSLOCK; // it think
    m_d3dSurface->LockRect(&lock_rect, NULL, flags);
    // v2 = this->D3DSurface->lpVtbl->LockRect(this->D3DSurface, &lock_rect, NULL, flags);
    // if ( v2 ) {
    // Log_DX8_ErrorCode(v2);
    //}
    *pitch = lock_rect.Pitch;
    return lock_rect.pBits;
}

void *SurfaceClass::Lock_Rect(int *pitch, int left, int top, int right, int bottom)
{
    RECT rect;

    // DX8_THREAD_ASSERT();
    rect.top = top;
    rect.left = left;
    rect.right = right;
    rect.bottom = bottom;
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    m_d3dSurface->LockRect(&lock_rect, &rect, D3DLOCK_NOSYSLOCK);
    // v7 = v6->D3DSurface->lpVtbl->LockRect(v6->D3DSurface, &lock_rect, &rect, D3DLOCK_NOSYSLOCK);
    // DX8_ErrorCode(v7);
    *pitch = lock_rect.Pitch;
    return lock_rect.pBits;
}

void SurfaceClass::Unlock()
{
    m_d3dSurface->UnlockRect();
    // v1 = m_d3dSurface->UnlockRect();
    // if ( v1 ) {
    //    Log_DX8_ErrorCode(v1);
    //}
}

void SurfaceClass::Clear()
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    m_d3dSurface->LockRect(&lock_rect, NULL, 0);
    // v1 = this->D3DSurface->vtable->LockRect(this->D3DSurface, &lock_rect, NULL, 0);
    // DX8_ErrorCode(v1);
    uint8_t *bytes = static_cast<uint8_t *>(lock_rect.pBits);
    for (unsigned int i = 0; i < sd.height; ++i) {
        memset(bytes, 0, sd.width * size);
        bytes += lock_rect.Pitch;
    }
    m_d3dSurface->UnlockRect();
    // v2 = m_d3dSurface->UnlockRect();
    // DX8_ErrorCode(v2);
}

void SurfaceClass::Copy(Vector2i &min, Vector2i &max, unsigned char *other)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    RECT rect;
    rect.left = min.I;
    rect.right = max.I;
    rect.top = min.J;
    rect.bottom = max.J;
    m_d3dSurface->LockRect(&lock_rect, &rect, 0);
    // v4 = this->D3DSurface->vtable->LockRect(this->D3DSurface, &lock_rect, &rect, 0);
    // DX8_ErrorCode(v4);
    uint8_t *bytes = static_cast<uint8_t *>(lock_rect.pBits);
    int diff = max.I - min.I;
    for (unsigned int i = min.J; i < max.J; ++i) {
        memcpy(bytes, &other[size * (min.I + sd.width * i)], diff * size);
        bytes += lock_rect.Pitch;
    }
    m_d3dSurface->UnlockRect();
    // v5 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
    // DX8_ErrorCode(v5);
}

void SurfaceClass::Copy(unsigned char *other)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(lock_rect));
    m_d3dSurface->LockRect(&lock_rect, NULL, 0);
    // v2 = v9->D3DSurface->vtable->LockRect(v9->D3DSurface, &lock_rect, NULL, 0);
    // DX8_ErrorCode(v2);
    uint8_t *bytes = static_cast<uint8_t *>(lock_rect.pBits);
    for (unsigned int i = 0; i < sd.height; ++i) {
        memcpy(bytes, &other[size * sd.width * i], sd.width * size);
        bytes += lock_rect.Pitch;
    }
    m_d3dSurface->UnlockRect();
    // v3 = v9->D3DSurface->vtable->UnlockRect(v9->D3DSurface);
    // DX8_ErrorCode(v3);
}

// dunno if real name exists
int Copy_Rects(IDirect3DSurface8 *pSourceSurface, RECT *pSourceRectsArray, int cRects,
    IDirect3DSurface8 *pDestinationSurface, POINT *pDestPointsArray)
{
    int result; // eax@1

    // DX8_Assert();
    DX8Wrapper::Get_D3D_Device8()->CopyRects(
        pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray);
    // v6 = DX8Wrapper::Get_D3D_Device8()->CopyRects(v5, pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface,
    // pDestPointsArray); DX8_ErrorCode(v6);
    //++number_of_DX8_calls;
    return result;
}

void SurfaceClass::Copy(
    unsigned dst_x, unsigned dst_y, unsigned src_x, unsigned src_y, unsigned width, unsigned height, SurfaceClass *other)
{
    // assert(other);
    // assert(width);
    // assert(height);
    RECT dest;

    SurfaceDescription sd;
    SurfaceDescription osd;
    Get_Description(sd);
    other->Get_Description(osd);
    RECT src;
    src.left = src_x;
    src.right = width + src_x;
    src.top = src_y;
    src.bottom = height + src_y;
    if ((width + src_x) > osd.width) {
        src.right = osd.width;
    }
    if (src.bottom > osd.height) {
        src.bottom = osd.height;
    }
    if (sd.format == osd.format && sd.width == osd.width && sd.height == osd.height) {
        POINT destpt;
        destpt.x = dst_x;
        destpt.y = dst_y;
        // dunno what this is, some binaries expose it, might be a surfclass static
        Copy_Rects(other->m_d3dSurface, &src, 1, m_d3dSurface, &destpt);
    } else {
        dest.left = dst_x;
        dest.right = width + dst_x;
        dest.top = dst_y;
        dest.bottom = height + dst_y;
        if ((width + dst_x) > sd.width) {
            dest.right = sd.width;
        }
        if (dest.bottom > sd.height) {
            dest.bottom = sd.height;
        }
        // D3DXLoadSurfaceFromSurface(m_d3dSurface, NULL, &dest, other->m_d3dSurface, NULL, &src, 1u, 0);
        // v8 = D3DXLoadSurfaceFromSurface(m_d3dSurface, NULL, &dest, other->m_d3dSurface, NULL, &src, 1u, 0);
        // DX8_ErrorCode(v8);
    }
}

void SurfaceClass::Stretch_Copy(unsigned int dstx, unsigned int dsty, unsigned int dstwidth, unsigned int dstheight,
    unsigned int srcx, unsigned int srcy, unsigned int srcwidth, unsigned int srcheight, SurfaceClass *other)
{
    // assert(other);

    SurfaceDescription sd;
    SurfaceDescription osd;
    Get_Description(sd);
    other->Get_Description(osd);

    RECT dest;
    RECT src;
    src.left = srcx;
    src.right = srcwidth + srcx;
    src.top = srcy;
    src.bottom = srcheight + srcy;
    dest.left = dstx;
    dest.right = dstwidth + dstx;
    dest.top = dsty;
    dest.bottom = dstheight + dsty;
    // D3DXLoadSurfaceFromSurface(m_d3dSurface, NULL, &dest, other->m_d3dSurface, NULL, &src, 4, 0);
    // v10 = D3DXLoadSurfaceFromSurface(m_d3dSurface, NULL, &dest, other->m_d3dSurface, NULL, &src, 4, 0);
    // DX8_ErrorCode(v10);
}

void SurfaceClass::FindBB(Vector2i *min, Vector2i *max)
{
    int v4; // eax@30
    int v5; // [sp+4Ch] [bp-6Ch]@25
    int v6; // [sp+50h] [bp-68h]@22
    int v7; // [sp+54h] [bp-64h]@19
    int v8; // [sp+58h] [bp-60h]@16

    SurfaceDescription sd;
    Get_Description(sd);

    // assert(Has_Alpha(sd.format);

    int alphabits = Alpha_Bits(sd.format);
    int mask = 0;
    switch (alphabits) {
        case 1:
            mask = 1;
            break;
        case 4:
            mask = 0xF;
            break;
        case 8:
            mask = 0xFF;
            break;
    }
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    RECT rect;
    memset(&rect, 0, sizeof(RECT));
    rect.bottom = max->J;
    rect.top = min->J;
    rect.left = min->I;
    rect.right = max->I;
    m_d3dSurface->LockRect(&lock_rect, &rect, D3DLOCK_READONLY);
    // v3 = m_d3dSurface->LockRect(&lock_rect, &rect, D3DLOCK_READONLY);
    // DX8_ErrorCode(v3);
    int size = Pixel_Size(sd);
    int realmin = max->I;
    int realmin_4 = max->J;
    int realmax = min->I;
    int realmax_4 = min->J;
    for (int i = min->J; i < max->J; ++i) {
        for (int j = min->I; j < max->I; ++j) {
            if ((unsigned char)mask
                & (unsigned char)((signed int)*((unsigned char *)lock_rect.pBits + lock_rect.Pitch * (i - min->J)
                                      + size * (j - min->I) + size - 1)
                    >> (8 - alphabits))) {
                if (realmin >= j) {
                    v8 = j;
                } else {
                    v8 = realmin;
                }
                realmin = v8;
                if (realmax <= j) {
                    v7 = j;
                } else {
                    v7 = realmax;
                }
                realmax = v7;
                if (realmin_4 >= i) {
                    v6 = i;
                } else {
                    v6 = realmin_4;
                }
                realmin_4 = v6;
                if (realmax_4 <= i) {
                    v5 = i;
                } else {
                    v5 = realmax_4;
                }
                realmax_4 = v5;
            }
        }
    }
    m_d3dSurface->UnlockRect();
    // v4 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
    // DX8_ErrorCode(v4);
    max->I = realmax;
    max->J = realmax_4;
    min->I = realmin;
    min->J = realmin_4;
}

bool SurfaceClass::Is_Transparent_Column(unsigned column)
{
    SurfaceDescription sd;

    Get_Description(sd);
    // assert(column<sd.width);
    // assert(Has_Alpha(sd.format));

    int bits = Alpha_Bits(sd.format);
    int mask = 0;
    switch (bits) {
        case 1:
            mask = 1;
            break;
        case 4:
            mask = 15;
            break;
        case 8:
            mask = 255;
            break;
    }

    int size = Pixel_Size(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(_D3DLOCKED_RECT));
    RECT rect;
    rect.bottom = sd.height;
    rect.top = 0;
    rect.left = column;
    rect.right = column + 1;
    m_d3dSurface->LockRect(&lock_rect, &rect, D3DLOCK_READONLY);
    // v2 = this->D3DSurface->vtable->LockRect(this->D3DSurface, &lock_rect, &rect, D3DLOCK_READONLY);
    // DX8_ErrorCode(v2);
    for (int i = 0; i < sd.height; ++i) {
        if ((unsigned char)mask
            & (unsigned char)((signed int)*((unsigned char *)lock_rect.pBits + lock_rect.Pitch * i + size - 1)
                >> (8 - bits))) {
            m_d3dSurface->UnlockRect();
            // v3 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
            // DX8_ErrorCode(v3);
            return 0;
        }
    }
    m_d3dSurface->UnlockRect();
    // v5 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
    // DX8_ErrorCode(v5);
    return 1;
}

unsigned char *SurfaceClass::Create_Copy(int *width, int *height, int *size, bool flip)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size_1 = Pixel_Size(sd);
    *width = sd.width;
    *height = sd.height;
    *size = size_1;
    uint8_t *buf = new uint8_t(size_1 * sd.width * sd.height);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    m_d3dSurface->LockRect(&lock_rect, NULL, D3DLOCK_READONLY);
    // v5 = this->D3DSurface->vtable->LockRect(this->D3DSurface, &lock_rect, NULL, 0x10);
    // DX8_ErrorCode(v5);
    uint8_t *bytes = static_cast<uint8_t *>(lock_rect.pBits);
    for (unsigned int i = 0; i < sd.height; ++i) {
        if (flip) {
            memcpy(&buf[size_1 * sd.width * (sd.height - i - 1)], bytes, sd.width * size_1);
        } else {
            memcpy(&buf[size_1 * sd.width * i], bytes, sd.width * size_1);
        }
        bytes += lock_rect.Pitch;
    }
    m_d3dSurface->UnlockRect();
    // v6 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
    // DX8_ErrorCode(v6);
    return buf;
}

w3dsurface_t SurfaceClass::Peek_D3D_Surface()
{
    return w3dsurface_t();
}

void SurfaceClass::Attach(w3dsurface_t surface)
{
    Detach();
    m_d3dSurface = surface;
    if (surface != nullptr) {
        surface->AddRef();
    }
}

void SurfaceClass::Detach()
{
    if (m_d3dSurface != nullptr) {
        m_d3dSurface->Release();
        m_d3dSurface = nullptr;
    }
}

void SurfaceClass::Draw_Horizonal_Line(unsigned int y, unsigned int x1, unsigned int x2, unsigned int color)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    RECT rect;
    rect.bottom = y + 1;
    rect.top = y;
    rect.left = x1;
    rect.right = x2 + 1;
    m_d3dSurface->LockRect(&lock_rect, &rect, 0);
    // v4 = m_d3dSurface->LockRect(this->D3DSurface, &lock, &rect, 0);
    // DX8_ErrorCode(v4);
    uint8_t *b1 = static_cast<uint8_t *>(lock_rect.pBits);
    uint16_t *b2 = static_cast<uint16_t *>(lock_rect.pBits);
    uint32_t *b4 = static_cast<uint32_t *>(lock_rect.pBits);
    for (unsigned int i = x1; i <= x2; ++i) {
        switch (size) {
            case 1:
                *b1 = color;
                ++b1;
                break;
            case 2:
                *b2 = color;
                ++b2;
                break;
            case 4:
                *b4 = color;
                ++b4;
                break;
            default:
                break;
        }
    }
    m_d3dSurface->UnlockRect();
    // v5 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
    // DX8_ErrorCode(v5);
}

void SurfaceClass::Draw_Pixel(unsigned x, unsigned y, unsigned color)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    RECT rect;
    rect.bottom = y + 1;
    rect.top = y;
    rect.left = x;
    rect.right = x + 1;
    m_d3dSurface->LockRect(&lock_rect, &rect, 0);
    // v4 = m_d3dSurface->LockRect(this->D3DSurface, &lock, &rect, 0);
    // DX8_ErrorCode(v4);
    switch (size) {
        case 1:
            *static_cast<uint8_t *>(lock_rect.pBits) = color;
            break;
        case 2:
            *static_cast<uint16_t *>(lock_rect.pBits) = color;
            break;
        case 4:
            *static_cast<uint32_t *>(lock_rect.pBits) = color;
            break;
        default:
            break;
    }
    m_d3dSurface->UnlockRect();
    // v5 = this->D3DSurface->vtable->UnlockRect(this->D3DSurface);
    // DX8_ErrorCode(v5);
}

void SurfaceClass::Get_Pixel(Vector3 &rgb, int x, int y)
{
    SurfaceDescription sd;
    Get_Description(sd);

    int xpos = x;
    if (x >= sd.width - 1) {
        xpos = sd.width - 1;
    }
    int ypos = y;
    if (y >= sd.height - 1) {
        ypos = sd.height - 1;
    }

    D3DLOCKED_RECT lock_rect;
    lock_rect.Pitch = 0;
    lock_rect.pBits = 0;

    RECT rect;
    rect.right = 0;
    rect.bottom = ypos + 1;
    rect.left = xpos;
    rect.top = ypos;
    rect.right = xpos + 1;
    m_d3dSurface->LockRect(&lock_rect, &rect, D3DLOCK_READONLY);
    // v8 = v7->lpVtbl->LockRect(v7, &lock_rect, &rect, D3DLOCK_READONLY);
    // if ( v8 )
    //{
    //    Log_DX8_ErrorCode(v8);
    //}

    // Convert_Pixel(rgb, &sd, lock_rect.pBits); we don't have this?

    m_d3dSurface->UnlockRect();
    // v9 = this->D3DSurface->lpVtbl->UnlockRect(this->D3DSurface);
    // if ( v9 )
    //{
    //    Log_DX8_ErrorCode(v9);
    //}
}
void SurfaceClass::Hue_Shift(Vector3 &hsv_shift)
{
    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    int pitch;
    uint8_t *bytes = static_cast<uint8_t *>(Lock(&pitch));
    for (unsigned int i = 0; i < sd.height; ++i) {
        for (unsigned int j = 0; j < sd.width; ++j) {
            Vector3 rgb;
            // Convert_Pixel(&rgb, &sd.format, &bytes[size * j]);
            // Recolor(&rgb, hsv_shift);

            float v5;
            float v7;
            float v8;
            if (rgb.X < 0.0) {
                v5 = 0.0;
            } else if (rgb.X > 1.0) {
                v5 = 1.0;
            } else {
                v5 = rgb.X;
            }
            rgb.X = v5;
            if (rgb.Y < 0.0) {
                v7 = 0.0;
            } else if (rgb.Y > 1.0) {
                v7 = 1.0;
            } else {
                v7 = rgb.Y;
            }
            rgb.Y = v7;
            if (rgb.Z < 0.0) {
                v8 = 0.0;
            } else if (rgb.Z > 1.0) {
                v8 = 1.0;
            } else {
                v8 = rgb.Z;
            }
            rgb.Z = v8;
            // Convert_Pixel(&bytes[size * j], &sd, &rgb);
        }
        bytes += pitch;
    }
    Unlock();
}

unsigned char SurfaceClass::Is_Monochrome()
{
    SurfaceClass *pool; // eax@4
    SurfaceClass *surf; // esi@5
    char v5; // bl@7
    int refs; // eax@8
    char *buff; // ecx@11
    char v10; // bl@11
    int width; // eax@12
    unsigned int v12; // esi@13
    const char *pixel; // edi@14
    bool v14; // cf@18
    char *buff1; // [sp+10h] [bp-34h]@11
    int v16; // [sp+14h] [bp-30h]@11
    int pitch; // [sp+1Ch] [bp-28h]@11
    int size;

    unsigned int v21; // [sp+40h] [bp-4h]@4

    SurfaceDescription sd;
    Get_Description(sd);
    switch (sd.format) {
        case WW3D_FORMAT_DXT1:
        case WW3D_FORMAT_DXT2:
        case WW3D_FORMAT_DXT3:
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
        /*
            if (!(byte_4CB4FD1 & 1)) {
                byte_4CB4FD1 |= 1u;
                a1 = createW3DMemPool("SurfaceClass", sizeof(SurfaceClass));
            }
            pool = allocateFromW3DMemPool(a1,
                sizeof(SurfaceClass),
                "W3D_C:\\projects\\ZeroHour\\code\\Libraries\\Source\\WWVegas\\WW3D2\\surfaceclass.cpp",
                0);
            if (pool) {
                surf = SurfaceClass::SurfaceClass(pool, sd.width, sd.height, Get_Valid_Texture_Format(sd.format, false));
            } else {
                surf = 0;
            }
            surf->Copy(0, 0, 0, 0, sd.width, sd.height, this);
            v5 = surf->Is_Monochrome();
            if (!surf) {
                return v5;
            }
            refs = surf->ref.NumRefs - 1;
            surf->ref.NumRefs = refs;
            if (!refs) {
                (surf->ref.vtable->Delete_This)();
            }
            return v5;
        */
        default: {
            size = Pixel_Size(sd);
            // buff = Lock(&pitch);
            buff1 = buff;
            v10 = 1;
            v16 = 0;
            if (!sd.height) {
                goto LABEL_19;
            }
            width = sd.width;
            break;
            case WW3D_FORMAT_A8:
            case WW3D_FORMAT_L8:
            case WW3D_FORMAT_A8L8:
            case WW3D_FORMAT_A4L4:
                return 1;
            case WW3D_FORMAT_UNKNOWN:
            case WW3D_FORMAT_A8P8:
            case WW3D_FORMAT_P8:
            case WW3D_FORMAT_U8V8:
            case WW3D_FORMAT_L6V5U5:
            case WW3D_FORMAT_X8L8V8U8:
                return 0;
        }
    }
    while (1) {
        v12 = 0;
        if (width) {
            break;
        }
    LABEL_18:
        buff += pitch;
        v14 = (v16 + 1) < sd.height;
        buff1 = buff;
        ++v16;
        if (!v14) {
        LABEL_19:
            Unlock();
            return 1;
        }
    }
    pixel = buff;

    Vector3 rgb;
    while (1) {
        // Convert_Pixel(&rgb, &sd, pixel);
        v10 &= rgb.Z == rgb.Y && rgb.X == rgb.Z && rgb.X == rgb.Y;
        if (!v10) {
            break;
        }
        width = sd.width;
        ++v12;
        pixel += size;
        if (v12 >= sd.width) {
            buff = buff1;
            goto LABEL_18;
        }
    }
    Unlock();
    return 0;
}

int SurfaceClass::Pixel_Size(const SurfaceDescription &sd)
{
    switch (sd.format) {
        case WW3D_FORMAT_R8G8B8:
            return 3;
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
            return 4;
        case WW3D_FORMAT_R5G6B5:
        case WW3D_FORMAT_X1R5G5B5:
        case WW3D_FORMAT_A1R5G5B5:
        case WW3D_FORMAT_A4R4G4B4:
        case WW3D_FORMAT_A8R3G3B2:
        case WW3D_FORMAT_X4R4G4B4:
        case WW3D_FORMAT_A8P8:
        case WW3D_FORMAT_A8L8:
            return 2;
        case WW3D_FORMAT_R3G3B2:
        case WW3D_FORMAT_A8:
        case WW3D_FORMAT_P8:
        case WW3D_FORMAT_L8:
        case WW3D_FORMAT_A4L4:
            return 1;
        default:
            break;
    }

    return 0;
}

//dunno where this was in
void RGB_To_HSV(Vector3 &hsv, Vector3 &rgb)
{
    double v3; // st7@1
    double v4; // st6@1
    double v5; // st6@5
    double v6; // st5@10
    double v7; // st7@15
    double v8; // st7@21
    char v10; // c0@21
    float v11; // [sp+8h] [bp+8h]@1
    float rgbb; // [sp+8h] [bp+8h]@3
    float rgbc; // [sp+8h] [bp+8h]@7
    float v1; // [sp+8h] [bp+8h]@14

    v3 = rgb.Y;
    v4 = rgb.X;
    v11 = rgb.X;
    if ( v4 > v3 )
    {
        v3 = v11;
    }
    rgbb = rgb.Z;
    if ( v3 <= rgbb )
    {
        v3 = rgbb;
    }
    v5 = rgb.Y;
    if ( rgb.X < v5 )
    {
        v5 = rgb.X;
    }
    rgbc = rgb.Z;
    if ( v5 >= rgbc )
    {
        v5 = rgbc;
    }
    hsv.Z = v3;
    if ( v3 == 0.0 )
    {
        v6 = 0.0;
    }
    else
    {
        v6 = (v3 - v5) / v3;
    }
    hsv.Y = v6;
    if ( v6 == 0.0 )
    {
        /////////////LODWORD(hsv.X) = 0xBF800000;
        return;
    }
    v1 = v3 - v5;
    if ( v3 == rgb.X )
    {
        v7 = (rgb.Y - rgb.Z) / v1;
    }
    else if ( v3 == rgb.Y )
    {
        v7 = (rgb.Z - rgb.X) / v1 + 2.0;
    }
    else
    {
        if ( v3 != rgb.Z )
        {
            goto LABEL_21;
        }
        v7 = (rgb.X - rgb.Y) / v1 + 4.0;
    }
    hsv.X = v7;
LABEL_21:
    v8 = hsv.X * 60.0;
    hsv.X = v8;
    if ( v10 )
    {
        hsv.X = v8 + 360.0;
    }
}

//dunno where this was in
void HSV_To_RGB(Vector3 &rgb, Vector3 &hsv)
{
    double v2; // st7@1
    double v3; // st6@1
    float v4; // edx@1
    double v5; // st7@5
    float v6; // ST0C_4@5
    signed __int64 v7; // rax@5
    float v8; // ST10_4@5
    double z; // st7@5
    float v10; // [sp+8h] [bp-Ch]@1
    float y; // [sp+8h] [bp-Ch]@5
    float v12; // [sp+Ch] [bp-8h]@5
    float x; // [sp+1Ch] [bp+8h]@1

    v2 = hsv.X;
    v3 = hsv.Y;
    v4 = hsv.Z;
    v10 = hsv.Y;
    x = hsv.Z;
    if ( v3 == 0.0 )
    {
        rgb.X = x;
        rgb.Y = v4;
        rgb.Z = v4;
    }
    else
    {
        if ( v2 == 360.0 )
        {
            v2 = 0.0;
        }
        v5 = v2 * 0.016666668;
        v6 = v5;
        v7 = floor(v5);
        v8 = v6 - v7;
        z = (1.0 - v10) * x;
        v12 = (1.0 - v8 * v10) * x;
        y = (1.0 - (1.0 - v8) * v10) * x;
        switch ( v7 )
        {
            case 0:
                rgb.X = x;
                rgb.Y = y;
                rgb.Z = z;
                break;
            case 1:
                rgb.X = v12;
                rgb.Y = x;
                rgb.Z = z;
                break;
            case 2:
                rgb.X = z;
                rgb.Y = x;
                rgb.Z = y;
                break;
            case 3:
                rgb.X = z;
                rgb.Y = v12;
                rgb.Z = x;
                break;
            case 4:
                rgb.X = y;
                rgb.Z = x;
                rgb.Y = z;
                break;
            case 5:
                rgb.X = x;
                rgb.Z = v12;
                rgb.Y = z;
                break;
            default:
                return;
        }
    }
}

//dunno where this was in
void Recolor(Vector3 &rgb, const Vector3 &hsv_shift)
{
    float y; // [sp+Ch] [bp-30h]@9
    float z; // [sp+Ch] [bp-30h]@14

    Vector3 hsv;
    RGB_To_HSV(hsv, rgb);
    if ( hsv.X < 0.0 ) {
        hsv.X = hsv.X + 0.0;
        hsv.Y = hsv.Y + 0.0;
        hsv.Z = hsv.Z + hsv_shift.Z;
    } else {
        hsv.X = hsv.X + hsv_shift.X;
        hsv.Y = hsv.Y + hsv_shift.Y;
        hsv.Z = hsv.Z + hsv_shift.Z;
    }

    if ( hsv.X < 0.0 )
    {
        hsv.X = hsv.X + 360.0;
    }
    if ( hsv.X > 360.0 )
    {
        hsv.X = hsv.X - 360.0;
    }

    if ( hsv.Y < 0.0 )
    {
        y = 0.0;
    }
    else if ( hsv.Y > 1.0 )
    {
        y = 1.0;
    }
    else
    {
        y = hsv.Y;
    }
    hsv.Y = y;

    if ( hsv.Z < 0.0 )
    {
        z = 0.0;
    }
    else if ( hsv.Z > 1.0 )
    {
        z = 1.0;
    }
    else
    {
        z = hsv.Z;
    }
    hsv.Z = z;

    HSV_To_RGB(rgb, hsv);
}

// i suspect this was moved in ZH from surfaceclass
void Convert_Pixel(Vector3 &,SurfaceClass::SurfaceDescription const&, unsigned char const*)
{
    //see BitmapHandlerClass
}

// i suspect this was moved in ZH from surfaceclass
void Convert_Pixel(unsigned char *,SurfaceClass::SurfaceDescription const&,Vector3 const&)
{
    //see BitmapHandlerClass
}

// following are from bfme2

int SurfaceClass::Get_Surface_Memory_Usage()
{
    // DX8_THREAD_ASSERT();

    SurfaceDescription sd;
    Get_Description(sd);
    int size = Pixel_Size(sd);
    if (size) {
        return sd.height * sd.width * size;
    }

    if (sd.format == WW3D_FORMAT_DXT1 || sd.format == WW3D_FORMAT_DXT2 || sd.format == WW3D_FORMAT_DXT3
        || sd.format == WW3D_FORMAT_DXT4 || sd.format == WW3D_FORMAT_DXT5) {
        int usage = sd.height * sd.width;
        if (sd.format == WW3D_FORMAT_DXT1) {
            // usage >>= 1;
            usage /= 2; // is this right ?
        }
        return usage;
    }
    return 0;
}

void SurfaceClass::Set_Alpha(int x, int y, unsigned char value)
{
    // DX8_THREAD_ASSERT();
    SurfaceDescription sd;
    Get_Description(sd);
    D3DLOCKED_RECT lock_rect;
    memset(&lock_rect, 0, sizeof(D3DLOCKED_RECT));
    RECT rect;
    rect.bottom = y + 1;
    rect.top = y;
    rect.left = x;
    rect.right = x + 1;
    m_d3dSurface->LockRect(&lock_rect, &rect, 0);
    // v5 = v7->D3DSurface->lpVtbl->LockRect(v7->D3DSurface, &lock_rect, &rect, 0);
    // DX8_ErrorCode(v5);

    // BFME2 does this, should be Has_Alpha for to work right with ZH?
    if (sd.format == WW3D_FORMAT_A8R8G8B8) {
        //*(lock_rect.pBits + 3) = value;
    }
    m_d3dSurface->UnlockRect();
    // v6 = v7->D3DSurface->lpVtbl->UnlockRect(v7->D3DSurface);
    // DX8_ErrorCode(v6);
}

void SurfaceClass::Tint_Surface(float red_tint, float green_tint, float blue_tint)
{
    // seems to be a Hue_Shift replacement that directly handles DXT without decompressing, todo if needed
}