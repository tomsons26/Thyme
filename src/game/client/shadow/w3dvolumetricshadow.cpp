/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dvolumetricshadow.h"
#include "hlod.h"
#include "mesh.h"
#include "meshmdl.h"
#include "missing.h"
#include "view.h"
#include "w3dbuffermanager.h"
#include <cmath>
#include <new>

#include "baseheightmap.h"
#include "globaldata.h"
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

class MeshModelClass;

W3DShadowGeometryMesh::W3DShadowGeometryMesh() :
    m_polyNeighbors(nullptr),
    m_numPolyNeighbors(0),
    m_parentVerts(nullptr),
    m_polygonNormals(nullptr),
    // BUGFIX Init all members
    m_mesh(nullptr),
    m_modelIndex(0),
    m_verts(nullptr),
    m_parentGeometry(nullptr),
    m_numVerts(0),
    m_numPolygons(0),
    m_polygons(nullptr)
{
}

W3DShadowGeometryMesh::~W3DShadowGeometryMesh()
{
    Delete_Neighbors();

    if (m_parentVerts != nullptr) {
        delete[] m_parentVerts;
    }

    if (m_polygonNormals != nullptr) {
        delete[] m_polygonNormals;
    }
}

PolyNeighbor *W3DShadowGeometryMesh::Get_Poly_Neighbor(int index)
{
    if (m_polyNeighbors == nullptr) {
        Build_Polygon_Neighbors();
    }

    if (index < 0 || index >= m_numPolyNeighbors) {
        captainslog_assert(0);
        return nullptr;
    }

    return &m_polyNeighbors[index];
}

void W3DShadowGeometryMesh::Build_Polygon_Neighbors()
{
    Build_Polygon_Normals();

    const int num = Get_Num_Polygon();

    if (num == 0) {
        if (m_numPolyNeighbors != 0) {
            Delete_Neighbors();
        }
        return;
    }

    if (num != m_numPolyNeighbors) {
        Delete_Neighbors();

        if (!Allocate_Neighbors(num)) {
            return;
        }
    }

    for (int i = 0; i < m_numPolyNeighbors; ++i) {

        m_polyNeighbors[i].myIndex = (short)i;

        for (int j = 0; j < 3; ++j) {
            m_polyNeighbors[i].neighbor[j].neighborIndex = -1;
        }
    }

    for (int i = 0; i < m_numPolyNeighbors; ++i) {

        short poly[3];
        Get_Polygon_Index(i, poly);
        Vector3 *normal = Get_Polygon_Normal(i);

        for (int j = 0; j < m_numPolyNeighbors; ++j) {

            if (i != j) {

                short other_poly[3];
                Get_Polygon_Index(j, other_poly);

                int index1_pos[2];

                int index1 = -1;
                int index2 = -1;

                for (int k = 0; k < 3; ++k) {

                    for (int l = 0; l < 3; ++l) {

                        if (poly[k] == other_poly[l]) {

                            if (index1 == -1) {
                                index1 = poly[k];
                                index1_pos[0] = k;
                                index1_pos[1] = l;

                            } else if (index2 == -1) {

                                int p0_dist = k - index1_pos[0];
                                int p1_dist = l - index1_pos[1];

                                // TODO what is this
                                unsigned int p0 = (p0_dist & 0x80000000) ^ (abs(p0_dist) & 2) << 30;
                                unsigned int p1 = (p1_dist & 0x80000000) ^ (abs(p1_dist) & 2) << 30;

                                if (p0 != p1) {

                                    Vector3 *other_normal = Get_Polygon_Normal(j);

                                    const float epsilon = 1.0f / 100.0f;

                                    if (fabsf((*other_normal * *normal) + 1.0f) > epsilon) {
                                        index2 = poly[k];
                                    }
                                }

                            } else {
                                index2 = -1;
                                index1 = -1;
                            }
                        }
                    }
                }

                if (index1 != -1 && index2 != -1) {
                    // found a shared edge
                    for (int edge = 0; edge < 3; ++edge) {
                        if (m_polyNeighbors[i].neighbor[edge].neighborIndex == -1) {
                            m_polyNeighbors[i].neighbor[edge].neighborIndex = (short)j;
                            m_polyNeighbors[i].neighbor[edge].neighborEdgeIndex[0] = (short)index1;
                            m_polyNeighbors[i].neighbor[edge].neighborEdgeIndex[1] = (short)index2;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool W3DShadowGeometryMesh::Allocate_Neighbors(int num_polys)
{
    captainslog_assert(m_numPolyNeighbors == 0);
    captainslog_assert(m_polyNeighbors == nullptr);

    m_polyNeighbors = new PolyNeighbor[num_polys];

    if (m_polyNeighbors == nullptr) {
        captainslog_assert(0);
        return false;
    }

    m_numPolyNeighbors = num_polys;
    return true;
}

void W3DShadowGeometryMesh::Delete_Neighbors()
{
    if (m_polyNeighbors != nullptr) {
        delete[] m_polyNeighbors;
        m_polyNeighbors = nullptr;
        m_numPolyNeighbors = 0;
    }
}

Vector3 *W3DShadowGeometryMesh::Get_Polygon_Normal(int index) const
{
    captainslog_assert(m_polygonNormals != nullptr);

    return &m_polygonNormals[index];
}

void W3DShadowGeometryMesh::Build_Polygon_Normals()
{
    if (m_polygonNormals == nullptr) {
        Vector3 *normals = new Vector3[m_numPolygons];

        for (int i = 0; i < m_numPolygons; ++i) {
            Get_Polygon_Normal(i, &normals[i]);
        }

        m_polygonNormals = normals;
    }
}

void W3DShadowGeometryMesh::Get_Polygon_Normal(int index, Vector3 *normal) const
{

    if (m_polygonNormals != nullptr) {
        *normal = m_polygonNormals[index];
        return;
    }

    short index_list[3];
    Get_Polygon_Index(index, index_list);

    Vector3 *v0 = Get_Vertex(index_list[0]);
    Vector3 *v1 = Get_Vertex(index_list[1]);
    Vector3 *v2 = Get_Vertex(index_list[2]);

    Vector3 b = *v1 - *v0;
    Vector3 a = *v1 - *v2;

    Vector3::Normalized_Cross_Product(a, b, normal);
}

void W3DShadowGeometryMesh::Get_Polygon_Index(int polygon_index, short *index_list) const
{
    const TriIndex *v = &m_polygons[polygon_index];
    index_list[0] = m_parentVerts[v->I];
    index_list[1] = m_parentVerts[v->J];
    index_list[2] = m_parentVerts[v->K];
}

W3DShadowGeometry::W3DShadowGeometry() : m_name{}, m_meshCount(0), m_numTotalsVerts(0) {}

void W3DShadowGeometry::Set_Name(const char *name)
{
    strlcpy(m_name, name, sizeof(m_name));
}

int W3DShadowGeometry::Init(RenderObjClass *robj)
{
    return 1;
}

int W3DShadowGeometry::Init_From_HLOD(RenderObjClass *robj)
{
    // TODO deduplicate Init_From_HLOD Init_From_Mesh code

    HLodClass *hlod = static_cast<HLodClass *>(robj);

    int lod_index = hlod->Get_LOD_Count() - 1;

    captainslog_dbgassert(m_meshCount < MAX_SHADOW_CASTER_MESHES, "Too many shadow sub-meshes");

    W3DShadowGeometryMesh *geo_mesh = &m_meshList[m_meshCount];

    m_numTotalsVerts = 0;
    for (int model_index = 0; model_index < hlod->Get_Lod_Model_Count(lod_index); ++model_index) {

        RenderObjClass *lod_model = hlod->Peek_Lod_Model(lod_index, model_index);

        if (lod_model != nullptr) {

            if (lod_model->Class_ID() == RenderObjClass::CLASSID_MESH) {

                captainslog_dbgassert(m_meshCount < MAX_SHADOW_CASTER_MESHES, "Too many shadow sub-meshes");

                MeshClass *mesh = static_cast<MeshClass *>(lod_model);

                geo_mesh->m_mesh = mesh;
                geo_mesh->m_modelIndex = model_index;

                MeshModelClass *model = geo_mesh->m_mesh->Peek_Model();

                if (geo_mesh->m_mesh->Is_Alpha() || geo_mesh->m_mesh->Is_Translucent()) {

                    if (!model->Get_Flag(MeshGeometryClass::CAST_SHADOW)) {
                        continue;
                    }
                }

                // TODO Look into. Is this why infantry can't have shadows?
                if (model->Get_Flag(MeshGeometryClass::SKIN)) {
                    continue;
                }

                geo_mesh->m_numVerts = model->Get_Vertex_Count();
                geo_mesh->m_verts = model->Get_Vertex_Array();
                geo_mesh->m_numPolygons = model->Get_Polygon_Count();
                geo_mesh->m_polygons = model->Get_Polygon_Array();

                if (geo_mesh->m_numVerts > 16384) {
                    return 0;
                }

                unsigned short verts[16384];
                memset(verts, -1, sizeof(verts));

                int num = geo_mesh->m_numVerts;

                for (int i = 0; i < geo_mesh->m_numVerts; ++i) {
                    if (verts[i] == 0xFFFF) {

                        for (int j = i + 1; j < geo_mesh->m_numVerts; ++j) {

                            Vector3 d = geo_mesh->m_verts[i] - geo_mesh->m_verts[j];

                            if (d.Length2() == 0.0f) {
                                verts[j] = i;
                                --num;
                            }
                        }
                        verts[i] = i;
                    }
                }

                geo_mesh->m_parentVerts = new unsigned short[geo_mesh->m_numVerts];
                memcpy(geo_mesh->m_parentVerts, verts, geo_mesh->m_numVerts * sizeof(unsigned short));

                geo_mesh->m_numVerts = num;
                m_numTotalsVerts += num;
                geo_mesh->m_parentGeometry = this;

                ++geo_mesh;

                ++m_meshCount;
            }
        }

        if (lod_model != nullptr) {
            if (lod_model->Class_ID() == RenderObjClass::CLASSID_SHDMESH) {
                captainslog_dbgassert(0, "Meshes using shaders are not supported! How did you end up here!?");
            }
        }
    }
    return m_meshCount != 0;
}

int W3DShadowGeometry::Init_From_Mesh(RenderObjClass *robj)
{
    // TODO deduplicate Init_From_HLOD Init_From_Mesh code

    captainslog_dbgassert(m_meshCount < MAX_SHADOW_CASTER_MESHES, "Too many shadow sub-meshes");

    W3DShadowGeometryMesh *geo_mesh = &m_meshList[m_meshCount];

    MeshClass *mesh = static_cast<MeshClass *>(robj);

    geo_mesh->m_mesh = mesh;

    geo_mesh->m_modelIndex = -1;

    MeshModelClass *model = geo_mesh->m_mesh->Peek_Model();

    if (geo_mesh->m_mesh->Is_Alpha() || geo_mesh->m_mesh->Is_Translucent()) {
        if (!model->Get_Flag(MeshGeometryClass::CAST_SHADOW)) {
            return 0;
        }
    }

    geo_mesh->m_numVerts = model->Get_Vertex_Count();
    geo_mesh->m_verts = model->Get_Vertex_Array();
    geo_mesh->m_numPolygons = model->Get_Polygon_Count();
    geo_mesh->m_polygons = model->Get_Polygon_Array();

    if (geo_mesh->m_numVerts > 16384) {
        return 0;
    }

    unsigned short verts[16384];
    memset(verts, -1, sizeof(verts));

    int num = geo_mesh->m_numVerts;

    for (int i = 0; i < geo_mesh->m_numVerts; ++i) {
        if (verts[i] == 0xFFFF) {

            for (int j = i + 1; j < geo_mesh->m_numVerts; ++j) {

                Vector3 d = geo_mesh->m_verts[i] - geo_mesh->m_verts[j];

                if (d.Length2() == 0.0f) {
                    verts[j] = i;
                    --num;
                }
            }
            verts[i] = i;
        }
    }

    geo_mesh->m_parentVerts = new unsigned short[geo_mesh->m_numVerts];
    memcpy(geo_mesh->m_parentVerts, verts, geo_mesh->m_numVerts * sizeof(unsigned short));

    geo_mesh->m_numVerts = num;
    geo_mesh->m_parentGeometry = this;

    m_meshCount = 1;
    m_numTotalsVerts = num;

    return 1;
}

W3DShadowGeometryManager::W3DShadowGeometryManager()
{
    m_geomPtrTable = new HashTableClass(2048);
    m_missingGeomTable = new HashTableClass(2048);
}

W3DShadowGeometryManager::~W3DShadowGeometryManager()
{
    Free_All_Geoms();

    if (m_geomPtrTable != nullptr) {
        delete m_geomPtrTable;
    }

    if (m_missingGeomTable != nullptr) {
        delete m_missingGeomTable;
    }
}

void W3DShadowGeometryManager::Free_All_Geoms()
{
    W3DShadowGeometryManagerIterator it(*this);

    for (it.First(); !it.Is_Done(); it.Next()) {
        W3DShadowGeometry *v1 = it.Get_Current_Geom();
        v1->Release_Ref();
    }

    m_geomPtrTable->Reset();
}

W3DShadowGeometry *W3DShadowGeometryManager::Peek_Geom(const char *name)
{
    return static_cast<W3DShadowGeometry *>(m_geomPtrTable->Find(name));
}

W3DShadowGeometry *W3DShadowGeometryManager::Get_Geom(const char *name)
{
    W3DShadowGeometry *geo = Peek_Geom(name);

    if (geo != nullptr) {
        geo->Add_Ref();
    }

    return geo;
}

bool W3DShadowGeometryManager::Add_Geom(W3DShadowGeometry *new_geom)
{

    captainslog_assert(new_geom != nullptr);

    new_geom->Add_Ref();
    m_geomPtrTable->Add(new_geom);

    return true;
}

void W3DShadowGeometryManager::Register_Missing(const char *name)
{
    m_missingGeomTable->Add(new MissingGeomClass(name));
}

bool W3DShadowGeometryManager::Is_Missing(const char *name) const
{
    return m_missingGeomTable->Find(name) != nullptr;
}

int W3DShadowGeometryManager::Load_Geom(RenderObjClass *robj, const char *name)
{
    bool state = false;

    W3DShadowGeometry *geo = new W3DShadowGeometry;

    if (geo == nullptr) {
        return 1;
    }

    geo->Set_Name(name);

    switch (robj->Class_ID()) {
        case RenderObjClass::CLASSID_MESH:
            state = geo->Init_From_Mesh(robj) != 0;
            break;

        case RenderObjClass::CLASSID_HLOD:
            state = geo->Init_From_HLOD(robj) != 0;
            break;

        default:
            break;
    }

    if (!state) {
        geo->Release_Ref();
        return 1;
    }

    if (Peek_Geom(geo->Get_Name())) {
        geo->Release_Ref();
        return 1;
    }

    Add_Geom(geo);
    geo->Release_Ref();

    return 0;
}

W3DShadowGeometry *W3DShadowGeometryManagerIterator::Get_Current_Geom()
{
    return static_cast<W3DShadowGeometry *>(Get_Current());
}

W3DVolumetricShadowManager::W3DVolumetricShadowManager() :
    m_shadowList(nullptr),
    // BUGFIX original didn't init these
    m_dynamicShadowVolumesToRender(nullptr),
    m_W3DShadowGeometryManager(nullptr)
{
    m_W3DShadowGeometryManager = new W3DShadowGeometryManager;

    g_theW3DBufferManager = new W3DBufferManager;
}

W3DVolumetricShadowManager::~W3DVolumetricShadowManager()
{
    Release_Resources();

    if (m_W3DShadowGeometryManager != nullptr) {
        delete m_W3DShadowGeometryManager;
    }
    m_W3DShadowGeometryManager = nullptr;

    if (g_theW3DBufferManager != nullptr) {
        delete g_theW3DBufferManager;
    }
    g_theW3DBufferManager = nullptr;
}

int W3DVolumetricShadowManager::Init()
{
    return 1;
}

void W3DVolumetricShadowManager::Reset()
{
    m_W3DShadowGeometryManager->Free_All_Geoms();
    g_theW3DBufferManager->Free_All_Buffers();
}

void W3DVolumetricShadowManager::Release_Resources()
{
    if (g_shadowIndexBufferD3D != nullptr) {
        g_shadowIndexBufferD3D->Release();
    }
    if (g_shadowVertexBufferD3D != nullptr) {
        g_shadowVertexBufferD3D->Release();
    }
    g_shadowIndexBufferD3D = nullptr;
    g_shadowVertexBufferD3D = nullptr;

    if (g_theW3DBufferManager != nullptr) {
        g_theW3DBufferManager->Release_Resources();

        Invalidate_Cached_Light_Positions();
    }
}

int W3DVolumetricShadowManager::Re_Acquire_Resources()
{
    Release_Resources();
#ifdef BUILD_WITH_D3D8
    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    captainslog_dbgassert(device, "Trying to ReAquireResources on W3DVolumetricShadowManager without device");

    if (device->CreateIndexBuffer(sizeof(unsigned short) * SHADOW_INDEX_SIZE,
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            D3DFMT_INDEX16,
            D3DPOOL_DEFAULT,
            &g_shadowIndexBufferD3D)
        < 0) {
        return 0;
    }
    if (!g_shadowVertexBufferD3D
        && device->CreateVertexBuffer(sizeof(Vector3) * SHADOW_VERTEX_SIZE,
               D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
               0,
               D3DPOOL_DEFAULT,
               &g_shadowVertexBufferD3D)
            < 0) {
        return 0;
    }
    if (g_theW3DBufferManager == nullptr || g_theW3DBufferManager->ReAcquire_Resources()) {
        return 1;
    }
#endif
    return 0;
}

void W3DVolumetricShadowManager::Remove_Shadow(W3DVolumetricShadow *shadow)
{
    W3DVolumetricShadow *s = nullptr;

    for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = i->m_next) {
        if (i == shadow) {
            if (s) {
                s->m_next = shadow->m_next;
            } else {
                m_shadowList = shadow->m_next;
            }
            delete shadow;
            return;
        }
        s = i;
    }
}

void W3DVolumetricShadowManager::Remove_All_Shadows()
{
    W3DVolumetricShadow *next;

    for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = next) {
        next = i->m_next;
        i->m_next = nullptr;
        delete i;
    }

    m_shadowList = nullptr;
}

void W3DVolumetricShadowManager::Add_Dynamic_Shadow_Task(W3DVolumetricShadowRenderTask *task)
{
    W3DVolumetricShadowRenderTask *cur = m_dynamicShadowVolumesToRender;
    m_dynamicShadowVolumesToRender = task;
    m_dynamicShadowVolumesToRender->m_nextTask = cur;
}

void W3DVolumetricShadowManager::Render_Stencil_Shadows()
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_VERTEX
    {
        D3DXVECTOR4 p;
        unsigned long color;
    };

    IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();
    if (dev != nullptr) {
        _TRANS_LIT_VERTEX vertex[4];

        int32_t x;
        int32_t y;

        g_theTacticalView->Get_Origin(&x, &y);

        int32_t width = g_theTacticalView->Get_Width();
        int32_t height = g_theTacticalView->Get_Height();

        float fy = float(height + y);
        float fx = float(width + x);

        vertex[0].p = D3DXVECTOR4(fx, fy, 0.0f, 1.0f);
        vertex[1].p = D3DXVECTOR4(fx, 0.0f, 0.0f, 1.0f);
        vertex[2].p = D3DXVECTOR4(float(x), fy, 0.0f, 1.0f);
        vertex[3].p = D3DXVECTOR4(float(x), 0.0f, 0.0f, 1.0f);

        vertex[0].color = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[1].color = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[2].color = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[3].color = g_theW3DShadowManager->Get_Shadow_Color();

        dev->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        dev->SetRenderState(D3DRS_ZENABLE, TRUE);
        dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
        dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        dev->SetRenderState(D3DRS_STENCILMASK, ~g_theW3DShadowManager->Get_Stencil_Mask());
        dev->SetRenderState(D3DRS_STENCILREF, 1);
        dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

        if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
            dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_VERTEX));
        }

        dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        dev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }
#endif
}

void W3DVolumetricShadowManager::Render_Shadows(bool force_stencil_fill)
{
#if 0
    W3DVolumetricShadowRenderTask *v8; // [esp+Ch] [ebp-4Ch]
    int color_write; // [esp+10h] [ebp-48h]
    W3DVolumetricShadowRenderTask *j; // [esp+14h] [ebp-44h]
    W3DVolumetricShadowRenderTask *i; // [esp+18h] [ebp-40h]
    W3DBufferManager::W3DVertexBuffer *vb; // [esp+24h] [ebp-34h]
    W3DVolumetricShadow *k; // [esp+2Ch] [ebp-2Ch]

    int count = 0;
    AABoxClass aabox;
    SphereClass sphere;

    g_theTerrainRenderObject->Get_Maximum_Visible_Box(shadowCameraFrustum, &aabox, 1);

    bcX = aabox.m_center.X;
    bcY = aabox.m_center.Y;
    bcZ = aabox.m_center.Z;
    beX = aabox.m_extent.X;
    beY = aabox.m_extent.Y;
    beZ = aabox.m_extent.Z;

    if (m_shadowList && g_theWriteableGlobalData->m_shadowVolumes) {

        IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();

        if (dev != nullptr) {

            nShadowIndicesInBuf = 0xFFFF;
            nShadowVertsInBuf = 0xFFFF;

            VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
            DX8Wrapper::Set_Material(material);

            Ref_Ptr_Release(material);

            DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);

            DX8Wrapper::Set_Texture(0, 0);
            DX8Wrapper::Set_Texture(1u, 0);

            DX8Wrapper::Apply_Render_State_Changes();

            dev->SetRenderState(D3DRS_ZFUNC, 4);
            dev->SetRenderState(D3DRS_ZENABLE, 1);
            dev->SetRenderState(D3DRS_ZWRITEENABLE, 0);
            dev->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
            dev->SetRenderState(D3DRS_FOGENABLE, 0);
            dev->SetRenderState(D3DRS_SHADEMODE, 1);
            dev->SetRenderState(D3DRS_LIGHTING, 0);

            dev->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
            dev->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
            dev->SetTextureStageState(0, D3DTSS_COLOROP, 3);
            dev->SetTextureStageState(0, D3DTSS_ALPHAOP, 1);
            dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
            dev->SetTextureStageState(1, D3DTSS_COLOROP, 1);
            dev->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
            dev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

            dev->SetTexture(0, 0);
            dev->SetTexture(1, 0);

            color_write = 0x12345678;

            const DX8Caps *v2 = DX8Wrapper::Get_Current_Caps();

            if (DX8Caps::Get_DX8_Caps(v2)->PrimitiveMiscCaps & 0x80) {
                dev->GetRenderState(D3DRS_COLORWRITEENABLE, (unsigned int *)&color_write);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0);
            } else {
                dev->SetRenderState(D3DRS_SRCBLEND, 1);
                dev->SetRenderState(D3DRS_DESTBLEND, 2);
                dev->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
            }

            dev->SetRenderState(D3DRS_STENCILENABLE, 1);

            if (g_theW3DShadowManager->Get_Stencil_Mask() == 0x80808080) {
                dev->SetRenderState(D3DRS_STENCILFUNC, 6);
            } else {
                dev->SetRenderState(D3DRS_STENCILFUNC, 7);
            }

            dev->SetRenderState(D3DRS_STENCILREF, 0x80808080);
            dev->SetRenderState(D3DRS_STENCILMASK, g_theW3DShadowManager->Get_Stencil_Mask());
            dev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
            dev->SetRenderState(D3DRS_STENCILZFAIL, 1);
            dev->SetRenderState(D3DRS_STENCILFAIL, 1);
            dev->SetRenderState(D3DRS_STENCILPASS, 7);
            dev->SetVertexShader(2);
            dev->SetRenderState(D3DRS_CULLMODE, 2);

            lastActiveVertexBuffer = 0;

            m_dynamicShadowVolumesToRender = 0;

            for (k = m_shadowList; k; k = k->m_next) {
                if (k->m_isEnabled) {
                    if (!k->m_isInvisibleEnabled) {
                        v8 = m_dynamicShadowVolumesToRender;
                        W3DVolumetricShadow::Update(k);
                        j = m_dynamicShadowVolumesToRender;
                        while (j != v8) {
                            W3DVolumetricShadow::RenderVolume(k, j->m_meshIndex, j->m_lightIndex);
                            j = (W3DVolumetricShadowRenderTask *)j->base.m_nextTask;
                            ++count;
                        }
                    }
                }
            }
            int format = W3DBufferManager::Get_DX8_Format(W3DBufferManager::VBM_FVF_XYZ);
            dev->SetVertexShader(format);

            for (vb = W3DBufferManager::getNextVertexBuffer(TheW3DBufferManager, 0, 0); vb;
                 vb = W3DBufferManager::getNextVertexBuffer(TheW3DBufferManager, vb, 0)) {
                i = (W3DVolumetricShadowRenderTask *)vb->m_renderTaskList;
                while (i) {
                    W3DVolumetricShadow::RenderVolume(i->m_parentShadow, i->m_meshIndex, i->m_lightIndex);
                    i = (W3DVolumetricShadowRenderTask *)i->base.m_nextTask;
                    ++count;
                }
            }

            dev->SetRenderState(D3DRS_STENCILPASS, 5);
            dev->SetRenderState(D3DRS_CULLMODE, 3);

            for (vb = W3DBufferManager::getNextVertexBuffer(TheW3DBufferManager, 0, 0); vb;
                 vb = W3DBufferManager::getNextVertexBuffer(TheW3DBufferManager, vb, 0)) {
                for (i = (W3DVolumetricShadowRenderTask *)vb->m_renderTaskList; i;
                     i = (W3DVolumetricShadowRenderTask *)i->base.m_nextTask) {
                    W3DVolumetricShadow::RenderVolume(i->m_parentShadow, i->m_meshIndex, i->m_lightIndex);
                }
            }

            dev->SetVertexShader(2);

            for (j = m_dynamicShadowVolumesToRender; j; j = (W3DVolumetricShadowRenderTask *)j->base.m_nextTask) {
                W3DVolumetricShadow::RenderVolume(j->m_parentShadow, j->m_meshIndex, j->m_lightIndex);
            }

            for (vb = W3DBufferManager::getNextVertexBuffer(TheW3DBufferManager, 0, 0); vb;
                 vb = W3DBufferManager::getNextVertexBuffer(TheW3DBufferManager, vb, 0)) {
                vb->m_renderTaskList = 0;
            }

            dev->SetRenderState(D3DRS_CULLMODE, 2);

            if (color_write != 0x12345678) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, color_write);
            }

            Render_Stencil_Shadows();

            dev->SetRenderState(D3DRS_SHADEMODE, 2);
            dev->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            dev->SetRenderState(D3DRS_LIGHTING, 0);

            DX8Wrapper::Invalidate_Cached_Render_States();
        }
    } else if (force_stencil_fill) {
        VertexMaterialClass *vetmat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        DX8Wrapper::Set_Material(vetmat);

        Ref_Ptr_Release(vetmat);

        DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Apply_Render_State_Changes();

        Render_Stencil_Shadows();
        DX8Wrapper::Invalidate_Cached_Render_States();
    }
#endif
}
void W3DVolumetricShadowManager::Invalidate_Cached_Light_Positions()
{
    if (m_shadowList != nullptr) {
        Vector3 v(0.0, 0.0, 0.0);

        for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = i->m_next) {
            for (int j = 0; j < 1; ++j) {
                for (int k = 0; k < MAX_SHADOW_CASTER_MESHES; ++k) {
                    i->Set_Light_Pos_History(j, k, &v);
                }
            }
        }
    }
}

Geometry::Geometry() :
    m_verts(nullptr),
    m_indices(nullptr),
    m_numPolygon(0),
    m_numVertex(0),
    m_flags(0),
    // BUGFIX original didn't init these
    m_visibleState(STATE_0),
    m_numActivePolygon(0),
    m_numActiveVertex(0)
{
}

Geometry::~Geometry()
{
    Release();
}

bool Geometry::Create(int num_vert, int num_poly)
{
    if (num_vert) {
        m_verts = new Vector3[num_vert];
        if (m_verts == nullptr) {
            return false;
        }
    }

    if (num_poly) {
        m_indices = new short[3 * num_poly];

        if (m_indices == nullptr) {
            return false;
        }
    }

    m_numPolygon = num_poly;
    m_numVertex = num_vert;
    m_numActivePolygon = 0;
    m_numActiveVertex = 0;

    return true;
}

void Geometry::Release()
{
    if (m_verts != nullptr) {
        delete[] m_verts;
        m_verts = nullptr;
    }
    if (m_indices != nullptr) {
        delete[] m_indices;
        m_indices = nullptr;
    }

    m_numPolygon = 0;
    m_numActivePolygon = 0;
    m_numVertex = 0;
    m_numActiveVertex = 0;
}

W3DVolumetricShadow::W3DVolumetricShadow()
{
    m_next = nullptr;
    m_geometry = nullptr;

    m_shadowLengthScale = 0.0;
    m_optimalExtrusionPadding = 0.0;
    m_robj = nullptr;

    m_isEnabled = true;
    m_isInvisibleEnabled = false;

    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; ++i) {

        m_numSilhouetteIndices[i] = 0;
        m_maxSilhouetteEntries[i] = 0;

        m_silhouetteIndex[i] = nullptr;
        m_shadowVolumeCount[i] = 0;
    }

    for (int i = 0; i < 1; ++i) {

        for (int j = 0; j < MAX_SHADOW_CASTER_MESHES; ++j) {

            m_shadowVolume[i][j] = nullptr;
            m_shadowVolumeVB[i][j] = nullptr;
            m_shadowVolumeIB[i][j] = nullptr;

            m_shadowVolumeRenderTask[i][j].m_parentShadow = this;
            m_shadowVolumeRenderTask[i][j].m_meshIndex = j;
            m_shadowVolumeRenderTask[i][j].m_lightIndex = i;

            m_objectXformHistory[i][j].Make_Identity();

            m_lightPosHistory[i][j] = Vector3(0.0f, 0.0f, 0.0f);
        }
    }
}

W3DVolumetricShadow::~W3DVolumetricShadow()
{
    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; ++i) {
        Delete_Silhouette(i);
    }
    for (int volume_index = 0; volume_index < 1; ++volume_index) {
        for (int mesh_index = 0; mesh_index < MAX_SHADOW_CASTER_MESHES; ++mesh_index) {

            if (m_shadowVolume[volume_index][mesh_index] != nullptr) {
                Geometry *geo = m_shadowVolume[volume_index][mesh_index];
                delete geo;
            }

            if (m_shadowVolumeVB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeVB[volume_index][mesh_index]);
            }

            if (m_shadowVolumeIB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeIB[volume_index][mesh_index]);
            }
        }
    }

    Ref_Ptr_Release(m_geometry);

    m_robj = nullptr;
}

void W3DVolumetricShadow::Set_Geometry(W3DShadowGeometry *geo)
{
    unsigned short num_verticies = 0;
    unsigned short new_num_verticies = 0;

    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; ++i) {
        if (m_geometry != nullptr) {
            W3DShadowGeometryMesh *mesh = m_geometry->Get_Mesh(i);
            num_verticies = mesh->Get_Num_Vertex();
        }

        if (geo != nullptr) {
            W3DShadowGeometryMesh *mesh = geo->Get_Mesh(i);
            new_num_verticies = mesh->Get_Num_Vertex();
        }

        if (new_num_verticies > num_verticies) {
            Delete_Silhouette(i);
            if (!Allocate_Silhouette(i, new_num_verticies)) {
                return;
            }
        }
    }

    m_geometry = geo;
}

void W3DVolumetricShadow::Add_Silhouette_Edge(int mesh_index, PolyNeighbor *poly_neighbor, PolyNeighbor *hidden)
{
    int edge = 0;

    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    for (int i = 0; i < 3; ++i) {
        if (poly_neighbor->neighbor[i].neighborIndex == hidden->myIndex) {
            edge = i;
            break;
        }
    }

    // BUGFIX Original didn't clear these
    short start = 0;
    short end = 0;

    short index_list[4];

    geo_mesh->Get_Polygon_Index(poly_neighbor->myIndex, index_list);

    if (index_list[0] != poly_neighbor->neighbor[edge].neighborEdgeIndex[0]
        && index_list[0] != poly_neighbor->neighbor[edge].neighborEdgeIndex[1]) {

        start = index_list[1];
        end = index_list[2];

    } else if (index_list[1] != poly_neighbor->neighbor[edge].neighborEdgeIndex[0]
        && index_list[1] != poly_neighbor->neighbor[edge].neighborEdgeIndex[1]) {

        start = index_list[2];
        end = index_list[0];

    } else {

        start = index_list[0];
        end = index_list[1];
    }

    Add_Silhouette_Indices(mesh_index, start, end);
}

void W3DVolumetricShadow::Add_Neighborless_Edges(int mesh_index, PolyNeighbor *poly_neighbor)
{
    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    // BUGFIX Original didn't clear these
    short start = 0;
    short end = 0;

    short index_list[4];

    geo_mesh->Get_Polygon_Index(poly_neighbor->myIndex, index_list);

    for (int i = 0; i < 3; ++i) {
        start = index_list[i];

        if (i == 2) {
            end = index_list[0];
        } else {
            end = index_list[i + 1];
        }

        bool add = true;

        for (int j = 0; j < 3; ++j) {
            if (poly_neighbor->neighbor[j].neighborIndex != -1
                && (poly_neighbor->neighbor[j].neighborEdgeIndex[0] == start
                        && poly_neighbor->neighbor[j].neighborEdgeIndex[1] == end
                    || poly_neighbor->neighbor[j].neighborEdgeIndex[1] == start
                        && poly_neighbor->neighbor[j].neighborEdgeIndex[0] == end)) {
                add = false;
                break;
            }
        }

        if (add) {
            Add_Silhouette_Indices(mesh_index, start, end);
        }
    }
}

void W3DVolumetricShadow::Add_Silhouette_Indices(int index, short start, short end)
{
    m_silhouetteIndex[index][(short)m_numSilhouetteIndices[index]++] = start;
    m_silhouetteIndex[index][(short)m_numSilhouetteIndices[index]++] = end;
}

bool W3DVolumetricShadow::Allocate_Shadow_Volume(int volume_index, int mesh_index)
{
    if (volume_index < 0 || volume_index >= 1) {
        return false;
    }

    Geometry *geo = m_shadowVolume[volume_index][mesh_index];

    if (geo == nullptr) {
        geo = new Geometry;
        ++m_shadowVolumeCount[mesh_index];
    }

    if (geo == nullptr) {
        --m_shadowVolumeCount[mesh_index];
        return false;
    }

    m_shadowVolume[volume_index][mesh_index] = geo;

    int num = m_maxSilhouetteEntries[mesh_index];

    if (geo->Get_Flags() & 1 && !geo->Create(2 * num, num)) {

        delete geo;

        return false;
    }

    return true;
}

void W3DVolumetricShadow::Delete_Shadow_Volume(int volume_index)
{
    if (volume_index >= 0 && volume_index < 1) {
        for (int mesh_index = 0; mesh_index < MAX_SHADOW_CASTER_MESHES; ++mesh_index) {
            if (m_shadowVolume[volume_index][mesh_index]) {
                Geometry *geo = m_shadowVolume[volume_index][mesh_index];

                delete geo;

                m_shadowVolume[volume_index][mesh_index] = nullptr;
                --m_shadowVolumeCount[mesh_index];
            }
        }
    }
}

void W3DVolumetricShadow::Reset_Shadow_Volume(int volume_index, int mesh_index)
{
    if (volume_index >= 0 && volume_index < 1) {

        Geometry *geo = m_shadowVolume[volume_index][mesh_index];

        if (geo != nullptr) {
            if (m_shadowVolumeVB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeVB[volume_index][mesh_index]);
                m_shadowVolumeVB[volume_index][mesh_index] = nullptr;
            }

            if (m_shadowVolumeIB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeIB[volume_index][mesh_index]);
                m_shadowVolumeIB[volume_index][mesh_index] = nullptr;
            }

            geo->Release();
        }
    }
}

bool W3DVolumetricShadow::Allocate_Silhouette(int index, int count)
{
    this->m_silhouetteIndex[index] = new short[5 * count];

    if (m_silhouetteIndex[index] == nullptr) {
        return false;
    }

    m_numSilhouetteIndices[index] = 0;
    m_maxSilhouetteEntries[index] = 5 * count;

    return true;
}

void W3DVolumetricShadow::Delete_Silhouette(int index)
{
    if (m_silhouetteIndex[index] != nullptr) {
        delete[] m_silhouetteIndex;
    }

    m_silhouetteIndex[index] = nullptr;
    m_numSilhouetteIndices[index] = 0;
}

void W3DVolumetricShadow::Reset_Silhouette(int index)
{
    m_numSilhouetteIndices[index] = 0;
}

void W3DVolumetricShadow::Release()
{
    g_theW3DVolumetricShadowManager->Remove_Shadow(this);
}
