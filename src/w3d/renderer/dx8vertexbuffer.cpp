#include "dx8vertexbuffer.h"

class DX8VertexBufferClass;

// matches zh
VertexBufferClass::VertexBufferClass(
    unsigned int type, unsigned int fvf, unsigned short vertex_count, unsigned int vertex_size) :
    m_type(type), m_VertexCount(vertex_count), m_engine_refs(0)
{
    DEBUG_ASSERT(m_VertexCount);
    DEBUG_ASSERT(m_type == BUFFER_TYPE_DX8 || m_type == BUFFER_TYPE_SORTING);
    DEBUG_ASSERT((fvf != 0 && vertex_size == 0) || (fvf == 0 && vertex_size != 0));
    m_fvf_info = new FVFInfoClass(fvf, vertex_size);
    ++VertexBufferCount;
    VertexBufferTotalVertices += m_VertexCount;
    VertexBufferTotalSize = m_VertexCount * m_fvf_info->Get_FVF_Size();
}

// matches zh
VertexBufferClass::~VertexBufferClass()
{
    --VertexBufferCount;
    VertexBufferTotalVertices -= m_VertexCount;
    VertexBufferTotalSize -= m_VertexCount * m_fvf_info->Get_FVF_Size();
    if (m_fvf_info) {
        delete m_fvf_info;
    }
}

// matches zh
unsigned int VertexBufferClass::Get_Total_Buffer_Count()
{
    return VertexBufferCount;
}

// matches zh
unsigned int VertexBufferClass::Get_Total_Allocated_Vertices()
{
    return VertexBufferTotalVertices;
}

// matches zh
unsigned int VertexBufferClass::Get_Total_Allocated_Memory()
{
    return VertexBufferTotalSize;
}

// matches zh
void VertexBufferClass::Add_Engine_Ref()
{
    ++m_engine_refs;
}

// matches zh
void VertexBufferClass::Release_Engine_Ref()
{
    --m_engine_refs;
    DEBUG_ASSERT(m_engine_refs >= 0);
}

// matches zh
VertexBufferLockClass::VertexBufferLockClass(VertexBufferClass *VertexBuffer)
{
    m_VertexBuffer = VertexBuffer;
}

// matches zh
SortingVertexBufferClass::SortingVertexBufferClass(unsigned short vertex_count) :
    VertexBufferClass(1, DX8_FVF_XYZNDUV2, vertex_count, 0)
{
    m_SortedVertexBuffer = new VertexFormatXYZNDUV2[vertex_count];
}

// matches zh
SortingVertexBufferClass::~SortingVertexBufferClass()
{
    delete m_SortedVertexBuffer;
}

// matches zh
VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass *vertex_buffer, int flags) :
    VertexBufferLockClass(vertex_buffer)
{
    // if (_DX8SingleThreaded) {
    // DEBUG_ASSERT(DX8Wrapper::Get_Main_Thread_ID() == ThreadClass::Get_Current_Thread_ID());
    //}
    DEBUG_ASSERT(m_VertexBuffer);
    DEBUG_ASSERT(!m_VertexBuffer->Engine_Refs());
    static_cast<DX8VertexBufferClass *>(m_VertexBuffer)->Add_Ref();
    switch (m_VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
            // DX8_Assert();
            // DX8_ErrorCode(
            static_cast<DX8VertexBufferClass *>(m_VertexBuffer)
                ->Get_DX8_Vertex_Buffer()
                ->Lock(0, 0, (BYTE **)&m_Vertices, flags);
            //);
            break;
        }
        case BUFFER_TYPE_SORTING: {
            m_Vertices = static_cast<SortingVertexBufferClass *>(m_VertexBuffer)->m_SortedVertexBuffer;
            break;
        }
        default:
            DEBUG_ASSERT(0);
            break;
    }
}

// matches zh
VertexBufferClass::WriteLockClass::~WriteLockClass()
{
    // if (_DX8SingleThreaded) {
    // DEBUG_ASSERT(DX8Wrapper::Get_Main_Thread_ID() == ThreadClass::Get_Current_Thread_ID());
    //}
    switch (m_VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
            // DX8_Assert();
            // DX8_ErrorCode(
            static_cast<DX8VertexBufferClass *>(m_VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock();
            //);
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            DEBUG_ASSERT(0);
            break;
    }
    m_VertexBuffer->Release_Ref();
}

// matches zh
VertexBufferClass::AppendLockClass::AppendLockClass(
    VertexBufferClass *vertex_buffer, unsigned int start_index, unsigned int index_range) :
    VertexBufferLockClass(vertex_buffer)
{
    // if (_DX8SingleThreaded) {
    // DEBUG_ASSERT(DX8Wrapper::Get_Main_Thread_ID() == ThreadClass::Get_Current_Thread_ID());
    //}
    DEBUG_ASSERT(m_VertexBuffer);
    DEBUG_ASSERT(m_VertexBuffer->Engine_Refs());
    DEBUG_ASSERT(start_index + index_range <= m_VertexBuffer->Get_Vertex_Count());
    m_VertexBuffer->Add_Ref();
    switch (vertex_buffer->Type()) {
        case BUFFER_TYPE_DX8: {
            UINT offset = m_VertexBuffer->FVF_Info().Get_FVF_Size() * start_index;
            UINT size = m_VertexBuffer->FVF_Info().Get_FVF_Size() * index_range;
            // DX8_Assert();
            // DX8_ErrorCode(
            static_cast<DX8VertexBufferClass *>(m_VertexBuffer)
                ->Get_DX8_Vertex_Buffer()
                ->Lock(offset, size, (BYTE **)&m_Vertices, 0);
            //);
            break;
        }
        case BUFFER_TYPE_SORTING: {
            m_Vertices = &static_cast<SortingVertexBufferClass *>(vertex_buffer)->m_SortedVertexBuffer[start_index];
            break;
        }
        default:
            DEBUG_ASSERT(0);
            break;
    }
}

// matches zh
VertexBufferClass::AppendLockClass::~AppendLockClass()
{
    // if (_DX8SingleThreaded) {
    // DEBUG_ASSERT(DX8Wrapper::Get_Main_Thread_ID() == ThreadClass::Get_Current_Thread_ID());
    //}
    switch (m_VertexBuffer->Type()) {
        case BUFFER_TYPE_DX8: {
            // DX8_Assert();
            // DX8_ErrorCode(
            static_cast<DX8VertexBufferClass *>(m_VertexBuffer)->Get_DX8_Vertex_Buffer()->Unlock();
            //);
            break;
        }
        case BUFFER_TYPE_SORTING:
            break;
        default:
            DEBUG_ASSERT(0);
            break;
    }
    m_VertexBuffer->Release_Ref();
}

//matches zh
DX8VertexBufferClass::DX8VertexBufferClass(unsigned int fvf, unsigned short vertex_count, UsageType usage, int vertex_size) :
    VertexBufferClass(0, fvf, vertex_count, vertex_size), m_DX8VertexBuffer(nullptr)
{
    Create_Vertex_Buffer(usage);
}

//matches zh
DX8VertexBufferClass::DX8VertexBufferClass(
    Vector3 *vertices, Vector3 *normals, Vector2 *tex_coords, unsigned short vertex_count, UsageType usage) :
    VertexBufferClass(0, DX8_FVF_XYZNUV1, vertex_count, 0), m_DX8VertexBuffer(nullptr)
{
    DEBUG_ASSERT(vertices);
    DEBUG_ASSERT(normals);
    DEBUG_ASSERT(tex_coords);
    Create_Vertex_Buffer(usage);
    Copy(vertices, normals, tex_coords, 0, vertex_count);
}

//matches zh
DX8VertexBufferClass::DX8VertexBufferClass(Vector3 *vertices, Vector3 *normals, Vector4 *diffuse, Vector2 *tex_coords,
    unsigned short vertex_count, UsageType usage) :
    VertexBufferClass(0, DX8_FVF_XYZNDUV1, vertex_count, 0), m_DX8VertexBuffer(nullptr)
{
    DEBUG_ASSERT(vertices);
    DEBUG_ASSERT(normals);
    DEBUG_ASSERT(tex_coords);
    DEBUG_ASSERT(diffuse);
    Create_Vertex_Buffer(usage);
    Copy(vertices, normals, tex_coords, diffuse, 0, vertex_count);
}

//matches zh
DX8VertexBufferClass::DX8VertexBufferClass(
    Vector3 *vertices, Vector4 *diffuse, Vector2 *tex_coords, unsigned short vertex_count, UsageType usage) :
    VertexBufferClass(0, DX8_FVF_XYZDUV1, vertex_count, 0), m_DX8VertexBuffer(nullptr)
{
    DEBUG_ASSERT(vertices);
    DEBUG_ASSERT(tex_coords);
    DEBUG_ASSERT(diffuse);
    Create_Vertex_Buffer(usage);
    Copy(vertices, tex_coords, diffuse, 0, vertex_count);
}

//matches zh
DX8VertexBufferClass::DX8VertexBufferClass(
    Vector3 *vertices, Vector2 *tex_coords, unsigned short vertex_count, UsageType usage) :
    VertexBufferClass(0, DX8_FVF_XYZUV1, vertex_count, 0), m_DX8VertexBuffer(nullptr)
{
    DEBUG_ASSERT(vertices);
    DEBUG_ASSERT(tex_coords);
    Create_Vertex_Buffer(usage);
    Copy(vertices, tex_coords, 0, vertex_count);
}

//matches zh
DX8VertexBufferClass::~DX8VertexBufferClass()
{
    m_DX8VertexBuffer->Release();
}

void DX8VertexBufferClass::Create_Vertex_Buffer(UsageType usage)
{
    // need help with this one
}

//matches zh
void DX8VertexBufferClass::Copy(Vector3 *loc, unsigned int first_vertex, unsigned int count)
{
    DEBUG_ASSERT(loc);
    DEBUG_ASSERT(count <= m_VertexCount);
    DEBUG_ASSERT(FVF_Info().Get_FVF() == DX8_FVF_XYZNUV1);
    if (first_vertex != 0) {
        VertexBufferClass::AppendLockClass lock(this, first_vertex, count);
        VertexFormatXYZ *vertices = (VertexFormatXYZ *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
        }
    } else {
        VertexBufferClass::WriteLockClass lock(this, 0);
        VertexFormatXYZ *vertices = (VertexFormatXYZ *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
        }
    }
}

//matches zh
void DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, unsigned int first_vertex, unsigned int count)
{
    DEBUG_ASSERT(loc);
    DEBUG_ASSERT(norm);
    DEBUG_ASSERT(count <= m_VertexCount);
    DEBUG_ASSERT(FVF_Info().Get_FVF() == DX8_FVF_XYZNUV1);
    if (first_vertex != 0) {
        VertexBufferClass::AppendLockClass lock(this, first_vertex, count);
        VertexFormatXYZN *vertices = (VertexFormatXYZN *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].nx = norm->X;
            vertices[i].ny = norm->Y;
            vertices[i].nz = norm->Z;
            ++norm;
        }
    } else {
        VertexBufferClass::WriteLockClass lock(this, 0);
        VertexFormatXYZN *vertices = (VertexFormatXYZN *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].nx = norm->X;
            vertices[i].ny = norm->Y;
            vertices[i].nz = norm->Z;
            ++norm;
        }
    }
}

//matches zh
void DX8VertexBufferClass::Copy(Vector3 *loc, Vector2 *uv, unsigned int first_vertex, unsigned int count)
{
    DEBUG_ASSERT(loc);
    DEBUG_ASSERT(uv);
    DEBUG_ASSERT(count <= m_VertexCount);
    DEBUG_ASSERT(FVF_Info().Get_FVF() == DX8_FVF_XYZNUV1);
    if (first_vertex != 0) {
        VertexBufferClass::AppendLockClass lock(this, first_vertex, count);
        VertexFormatXYZUV1 *vertices = (VertexFormatXYZUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
        }
    } else {
        VertexBufferClass::WriteLockClass lock(this, 0);
        VertexFormatXYZUV1 *vertices = (VertexFormatXYZUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
        }
    }
}

// matches zh
void DX8VertexBufferClass::Copy(Vector3 *loc, Vector3 *norm, Vector2 *uv, unsigned int first_vertex, unsigned int count)
{
    DEBUG_ASSERT(loc);
    DEBUG_ASSERT(norm);
    DEBUG_ASSERT(uv);
    DEBUG_ASSERT(count <= m_VertexCount);
    DEBUG_ASSERT(FVF_Info().Get_FVF() == DX8_FVF_XYZNUV1);
    if (first_vertex != 0) {
        VertexBufferClass::AppendLockClass lock(this, first_vertex, count);
        VertexFormatXYZNUV1 *vertices = (VertexFormatXYZNUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].nx = norm->X;
            vertices[i].ny = norm->Y;
            vertices[i].nz = norm->Z;
            ++norm;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
        }
    } else {
        VertexBufferClass::WriteLockClass lock(this, 0);
        VertexFormatXYZNUV1 *vertices = (VertexFormatXYZNUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].nx = norm->X;
            vertices[i].ny = norm->Y;
            vertices[i].nz = norm->Z;
            ++norm;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
        }
    }
}

//matches zh
void DX8VertexBufferClass::Copy(Vector3 *loc, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned int count)
{
    DEBUG_ASSERT(loc);
    DEBUG_ASSERT(uv);
    DEBUG_ASSERT(diffuse);
    DEBUG_ASSERT(count <= m_VertexCount);
    DEBUG_ASSERT(FVF_Info().Get_FVF() == DX8_FVF_XYZNUV1);
    if (first_vertex != 0) {
        VertexBufferClass::AppendLockClass lock(this, first_vertex, count);
        VertexFormatXYZDUV1 *vertices = (VertexFormatXYZDUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
            vertices[i].diffuse = DX8Wrapper_Convert_Color(diffuse[i]);
        }
    } else {
        VertexBufferClass::WriteLockClass lock(this, 0);
        VertexFormatXYZDUV1 *vertices = (VertexFormatXYZDUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
            vertices[i].diffuse = DX8Wrapper_Convert_Color(diffuse[i]);
        }
    }
}

//matches zh
void DX8VertexBufferClass::Copy(
    Vector3 *loc, Vector3 *norm, Vector2 *uv, Vector4 *diffuse, unsigned int first_vertex, unsigned int count)
{
    DEBUG_ASSERT(loc);
    DEBUG_ASSERT(norm);
    DEBUG_ASSERT(uv);
    DEBUG_ASSERT(diffuse);
    DEBUG_ASSERT(count <= m_VertexCount);
    DEBUG_ASSERT(FVF_Info().Get_FVF() == DX8_FVF_XYZNUV1);
    if (first_vertex != 0) {
        VertexBufferClass::AppendLockClass lock(this, first_vertex, count);
        VertexFormatXYZNDUV1 *vertices = (VertexFormatXYZNDUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].nx = norm->X;
            vertices[i].ny = norm->Y;
            vertices[i].nz = norm->Z;
            ++norm;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
            vertices[i].diffuse = DX8Wrapper_Convert_Color(diffuse[i]);
        }
    } else {
        VertexBufferClass::WriteLockClass lock(this, 0);
        VertexFormatXYZNDUV1 *vertices = (VertexFormatXYZNDUV1 *)lock.Get_Vertex_Array();
        for (int i = 0; i < count; ++i) {
            vertices[i].x = loc->X;
            vertices[i].y = loc->Y;
            vertices[i].z = loc->Z;
            ++loc;
            vertices[i].nx = norm->X;
            vertices[i].ny = norm->Y;
            vertices[i].nz = norm->Z;
            ++norm;
            vertices[i].u1 = uv->X;
            vertices[i].v1 = uv->Y;
            ++uv;
            vertices[i].diffuse = DX8Wrapper_Convert_Color(diffuse[i]);
        }
    }
}

// matches zh
DynamicVBAccessClass::DynamicVBAccessClass(unsigned int type, unsigned int fvf, unsigned short vertex_count) :
    m_Type(type), m_FVFInfo(DynamicFVFInfo), m_VertexCount(vertex_count), m_DynamicVertexBuffer(nullptr)
{
    DEBUG_ASSERT(fvf==dynamic_fvf_type);
    DEBUG_ASSERT(m_Type == BUFFER_TYPE_DYNAMIC_DX8 || m_Type == BUFFER_TYPE_DYNAMIC_SORTING);

    if (m_Type == BUFFER_TYPE_DYNAMIC_DX8) {
        Allocate_DX8_Dynamic_Buffer();
    } else {
        Allocate_Sorting_Dynamic_Buffer();
    }
}

// matches zh
DynamicVBAccessClass::~DynamicVBAccessClass()
{
    if (m_Type == 2) {
        DynamicDX8VertexBufferInUse = 0;
        DynamicDX8VertexBufferOffset += m_VertexCount;
    } else {
        DynamicSortingVertexArrayInUse = 0;
        DynamicSortingVertexArrayOffset += m_VertexCount;
    }
    if (m_DynamicVertexBuffer) {
        m_DynamicVertexBuffer->Release_Ref();
    }
    m_DynamicVertexBuffer = nullptr;
}

// matches zh
void DynamicVBAccessClass::Deinit()
{
    DEBUG_ASSERT((DynamicDX8VertexBuffer == NULL) || (DynamicDX8VertexBuffer->Num_Refs() == 1));
    if (DynamicDX8VertexBuffer) {
        DynamicDX8VertexBuffer->Release_Ref();
    }
    DynamicDX8VertexBuffer = nullptr;
    DynamicDX8VertexBufferInUse = 0;
    DynamicDX8VertexBufferSize = 5000;
    DynamicDX8VertexBufferOffset = 0;
    DEBUG_ASSERT((DynamicSortingVertexArray == NULL) || (DynamicSortingVertexArray->Num_Refs() == 1));
    if (DynamicSortingVertexArray) {
        DynamicSortingVertexArray->Release_Ref();
    }
    DynamicSortingVertexArray = nullptr;
    DEBUG_ASSERT(!DynamicSortingVertexArrayInUse);
    DynamicSortingVertexArrayInUse = 0;
    DynamicSortingVertexArraySize = 0;
    DynamicSortingVertexArrayOffset = 0;
}

void  DynamicVBAccessClass::Allocate_DX8_Dynamic_Buffer()
{
    //need help with this
}

void  DynamicVBAccessClass::Allocate_Sorting_Dynamic_Buffer()
{
    //need help with this
}