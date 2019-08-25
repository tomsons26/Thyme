#pragma once

#ifndef DX8VERTEXBUFFER_H
#define DX8VERTEXBUFFER_H

#include "d3d8.h"
#include "dx8fvf.h"
#include "refcount.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

unsigned int DX8Wrapper_Convert_Color(const Vector4 &color)
{
    DEBUG_ASSERT(color.X <= 1.0f);
    DEBUG_ASSERT(color.Y <= 1.0f);
    DEBUG_ASSERT(color.Z <= 1.0f);
    DEBUG_ASSERT(color.W <= 1.0f);
    DEBUG_ASSERT(color.X >= 0.0f);
    DEBUG_ASSERT(color.Y >= 0.0f);
    DEBUG_ASSERT(color.Z >= 0.0f);
    DEBUG_ASSERT(color.W >= 0.0f);

    return D3DCOLOR_COLORVALUE(color.X, color.Y, color.Z, color.W);
}

class VertexBufferClass;

// dunno where these were
enum
{
    BUFFER_TYPE_DX8 = 0x0,
    BUFFER_TYPE_SORTING = 0x1,
    BUFFER_TYPE_DYNAMIC_DX8 = 0x2,
    BUFFER_TYPE_DYNAMIC_SORTING = 0x3,
    BUFFER_TYPE_INVALID = 0x4,
};

struct VertexFormatXYZ
{
    int x;
    int y;
    int z;
};

struct VertexFormatXYZN
{
    int x;
    int y;
    int z;
    int nx;
    int ny;
    int nz;
};

struct VertexFormatXYZNUV1
{
    int x;
    int y;
    int z;
    int nx;
    int ny;
    int nz;
    int u1;
    int v1;
};

struct VertexFormatXYZNDUV1
{
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
    unsigned int diffuse;
    float u1;
    float v1;
};

struct VertexFormatXYZNDUV2
{
    float x;
    float y;
    float z;
    float nx;
    float ny;
    float nz;
    unsigned int diffuse;
    float u1;
    float v1;
    float u2;
    float v2;
};

struct VertexFormatXYZDUV1
{
    float x;
    float y;
    float z;
    unsigned int diffuse;
    float u1;
    float v1;
};

struct VertexFormatXYZDUV2
{
    float x;
    float y;
    float z;
    unsigned int diffuse;
    float u1;
    float v1;
    float u2;
    float v2;
};

struct VertexFormatXYZUV1
{
    int x;
    int y;
    int z;
    int u1;
    int v1;
};

class VertexBufferLockClass
{
public:
    VertexBufferLockClass(VertexBufferClass *vertex_buffer);
    void *Get_Vertex_Array() { return m_Vertices; }

    VertexBufferClass *m_VertexBuffer;
    void *m_Vertices;
};

class VertexBufferClass : public W3DMPO, public RefCountClass
{
public:
    class WriteLockClass : public VertexBufferLockClass
    {
    public:
        WriteLockClass(VertexBufferClass *vertex_buffer, int flags);
        ~WriteLockClass();
    };
    class AppendLockClass : public VertexBufferLockClass
    {
    public:
        AppendLockClass(VertexBufferClass *vertex_buffer, unsigned int start_index, unsigned int index_range);
        ~AppendLockClass();
    };
    VertexBufferClass(unsigned int type, unsigned int fvf, unsigned short vertex_count, unsigned int vertex_size);
    ~VertexBufferClass();
    FVFInfoClass &FVF_Info() { return *m_fvf_info; }
    unsigned short Get_Vertex_Count() { return m_VertexCount; }
    unsigned int Type() { return m_type; }

    static unsigned int Get_Total_Buffer_Count();
    static unsigned int Get_Total_Allocated_Vertices();
    static unsigned int Get_Total_Allocated_Memory();

    void Add_Engine_Ref();
    void Release_Engine_Ref();
    unsigned int Engine_Refs() { return m_engine_refs; }

protected:
    unsigned int m_type;
    unsigned short m_VertexCount;
    int m_engine_refs;
    FVFInfoClass *m_fvf_info;

    // todo fix types
    static char VertexBufferCount;
    static char VertexBufferTotalVertices;
    static char VertexBufferTotalSize;
};

class SortingVertexBufferClass : public VertexBufferClass
{
public:
    SortingVertexBufferClass(unsigned short vertex_count);
    ~SortingVertexBufferClass();

public:
    VertexFormatXYZNDUV2 *m_SortedVertexBuffer;
};

class DX8VertexBufferClass : public VertexBufferClass
{
public:
    enum UsageType
    {
        USAGE_DEFAULT = 0x0,
        USAGE_DYNAMIC = 0x1,
        USAGE_SOFTWAREPROCESSING = 0x2,
        USAGE_NPATCHES = 0x4,
    };
    DX8VertexBufferClass(unsigned int FVF, unsigned short vertex_count, UsageType usage, int vertex_size);
    DX8VertexBufferClass(
        Vector3 *vertices, Vector3 *normals, Vector2 *tex_coords, unsigned short vertex_count, UsageType usage);
    DX8VertexBufferClass(Vector3 *vertices, Vector3 *normals, Vector4 *diffuse, Vector2 *tex_coords,
        unsigned short VertexCount, UsageType usage);
    DX8VertexBufferClass(
        Vector3 *vertices, Vector4 *diffuse, Vector2 *tex_coords, unsigned short vertex_count, UsageType usage);
    DX8VertexBufferClass(Vector3 *vertices, Vector2 *tex_coords, unsigned short vertex_count, UsageType usage);

    ~DX8VertexBufferClass();

    IDirect3DVertexBuffer8 *Get_DX8_Vertex_Buffer() { return m_DX8VertexBuffer; }
    void Create_Vertex_Buffer(UsageType usage);

    void Copy(Vector3 *loc, unsigned int first_vertex, unsigned int count);
    void Copy(Vector3 *loc, Vector3 *norm, unsigned int first_vertex, unsigned int count);
    void Copy(Vector3 *loc, Vector2 *uv, unsigned int first_vertex, unsigned int count);
    void Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, unsigned int first_vertex, unsigned int count);
    void Copy(Vector3 *loc, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned int count);
    void Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned int count);

public:
    IDirect3DVertexBuffer8 *m_DX8VertexBuffer;
};

class DynamicVBAccessClass
{
public:
    DynamicVBAccessClass(unsigned int type, unsigned int fvf, unsigned short vertex_count);
    ~DynamicVBAccessClass();

    void Allocate_Sorting_Dynamic_Buffer();
    void Allocate_DX8_Dynamic_Buffer();

    static void DynamicVBAccessClass::Deinit();
    static DX8VertexBufferClass *DynamicDX8VertexBuffer;
    static SortingVertexBufferClass *DynamicSortingVertexArray;

public:
    FVFInfoClass &m_FVFInfo;
    unsigned int m_Type;
    unsigned short m_VertexCount;
    unsigned short m_VertexBufferOffset;
    VertexBufferClass *m_DynamicVertexBuffer;

    // todo fix types
    static char DynamicDX8VertexBufferInUse;
    static char DynamicDX8VertexBufferOffset;
    static char DynamicDX8VertexBufferSize;
    static char DynamicSortingVertexArrayInUse;
    static char DynamicSortingVertexArraySize;
    static char DynamicSortingVertexArrayOffset;
};

#endif // DX8VERTEXBUFFER_H