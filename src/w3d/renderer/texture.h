/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Class implementing the texture interface for surface textures.
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
#include "surfaceclass.h"
#include "texturebase.h"
#include "w3dformat.h"
#include <new>

class TextureFilterClass
{
public:
    enum FilterType
    {
        FILTER_TYPE_NONE,
        FILTER_TYPE_FAST,
        FILTER_TYPE_BEST,
        FILTER_TYPE_DEFAULT,
        FILTER_TYPE_COUNT,
    };

    enum TextureFilterMode
    {
        FILTER_MODE_BILINEAR,
        FILTER_MODE_TRILINEAR,
        FILTER_MODE_ANISOTROPIC2X,
        FILTER_MODE_ANISOTROPIC4X, // These appear to have been added by TTScripts?
        FILTER_MODE_ANISOTROPIC8X,
        FILTER_MODE_ANISOTROPIC16X,
    };

    enum TxtAddrMode
    {
        TEXTURE_ADDRESS_REPEAT,
        TEXTURE_ADDRESS_CLAMP,
    };

public:
    TextureFilterClass(MipCountType mip_level_count);
    void Apply(unsigned stage);
    void Set_Mip_Mapping(FilterType type) { m_mipMapFilter = type; }
    void Set_Min_Filter(FilterType type) { m_minTextureFilter = type; }
    void Set_Mag_Filter(FilterType type) { m_magTextureFilter = type; }
    void Set_Default_Min_Filter(FilterType type);
    void Set_Default_Mag_Filter(FilterType type);
    void Set_Default_Mip_Filter(FilterType type);

    static void Init_Filters(TextureFilterMode mode);

#ifdef GAME_DLL
    TextureFilterClass *Hook_Ctor(MipCountType mip_level_count) { return new (this) TextureFilterClass(mip_level_count); }
#endif

private:
    FilterType m_minTextureFilter;
    FilterType m_magTextureFilter;
    FilterType m_mipMapFilter;
    TxtAddrMode m_uAddressMode;
    TxtAddrMode m_vAddressMode;
};

class TextureClass : public TextureBaseClass
{
public:
    TextureClass(unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, PoolType pool,
        bool render_target, bool allow_reduction);
    TextureClass(char const *name, char const *full_path, MipCountType mip_level_count, WW3DFormat format,
        bool allow_compression, bool allow_reduction);
    TextureClass(SurfaceClass *surface, MipCountType mip_level_count);
    TextureClass(IDirect3DBaseTexture8 *d3d_texture);

    virtual int Get_Asset_Type() override { return 0; }
    SurfaceClass *Get_Surface_Level(unsigned int level);
    IDirect3DSurface8 *Get_D3D_Surface_Level(unsigned int level);
    virtual unsigned int Get_Texture_Memory_Usage() override;
    virtual void Init() override;
    virtual void Apply_New_Surface(w3dbasetexture_t d3d_texture, bool initialized, bool reset) override;
    virtual void Apply(unsigned stage) override;
    virtual TextureClass *As_Texture() override { return this; }

    WW3DFormat Texture_Format() const { return m_textureFormat; }

private:
    WW3DFormat m_textureFormat;
    TextureFilterClass m_textureFilter;
};
