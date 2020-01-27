#include "renderobj.h"
#include "castres.h"
#include "coltest.h"
#include "intersect.h"
#include "persistfactory.h"
#include "sphere.h"
#include "vector2.h"
#include "wwstring.h"

const float RenderObjClass::AT_MIN_LOD = FLT_MAX;
const float RenderObjClass::AT_MAX_LOD = -1.0f;

#include "Windows.h" //for lstrcpy, lstrlen, needs to be made portable
#include <stdio.h>
#include <string.h>

/**
 *
 *
 */
StringClass Filename_From_Asset_Name(const char *asset_name)
{
    StringClass filename;
    if (asset_name != nullptr) {
        // copy the model name to a new file name buffer
        ::lstrcpy(filename.Get_Buffer(::lstrlen(asset_name) + 5), asset_name);

        // strip off the model's suffix if there is one
        char *suffix = ::strchr((char *)filename, '.');
        if (suffix != nullptr) {
            suffix[0] = 0;
        }

        // add the w3d file extension
        filename += ".w3d";
    }

    return filename;
}

/**
 *
 *
 */
static inline bool Check_Is_Transform_Identity(const Matrix3D &m)
{
    const float zero = 0.0f;
    const float one = 1.0f;

    unsigned d = ((unsigned &)m[0][0] ^ (unsigned &)one) | ((unsigned &)m[0][1] ^ (unsigned &)zero)
        | ((unsigned &)m[0][2] ^ (unsigned &)zero) | ((unsigned &)m[0][3] ^ (unsigned &)zero)
        | ((unsigned &)m[1][0] ^ (unsigned &)zero) | ((unsigned &)m[1][1] ^ (unsigned &)one)
        | ((unsigned &)m[1][2] ^ (unsigned &)zero) | ((unsigned &)m[1][3] ^ (unsigned &)zero)
        | ((unsigned &)m[2][0] ^ (unsigned &)zero) | ((unsigned &)m[2][1] ^ (unsigned &)zero)
        | ((unsigned &)m[2][2] ^ (unsigned &)one) | ((unsigned &)m[2][3] ^ (unsigned &)zero);
    return !d;
}

class RenderObjPersistFactoryClass : public PersistFactoryClass
{
    virtual unsigned int Chunk_ID() const;
    virtual PersistClass *Load(ChunkLoadClass &cload) const;
    virtual void Save(ChunkSaveClass &csave, PersistClass *obj) const;
};

static RenderObjPersistFactoryClass _RenderObjPersistFactory;

/**
 *
 *
 */
unsigned int RenderObjPersistFactoryClass::Chunk_ID() const
{
    return WW3D_PERSIST_CHUNKID_RENDEROBJ;
}

PersistClass *RenderObjPersistFactoryClass::Load(ChunkLoadClass &cload) const
{
    // requires WW3DAssetManager
    return nullptr;
}

void RenderObjPersistFactoryClass::Save(ChunkSaveClass &csave, PersistClass *obj) const
{
    // requires WW3DAssetManager
}

/**
 *
 *
 */
RenderObjClass::RenderObjClass() :
    m_bits(DEFAULT_BITS),
    m_transform(1.0f),
    m_objectScale(1.0f),
    m_unknown(0),
    m_cachedBoundingSphere(Vector3(0, 0, 0), 1.0f),
    m_cachedBoundingBox(Vector3(0, 0, 0), Vector3(1.0f, 1.0f, 1.0f)),
    m_nativeScreenSize(1.0f), // should be (W3D::Get_Default_Native_Screen_Size()),
    m_isTransformIdentity(false),
    m_scene(nullptr),
    m_container(nullptr),
    m_userData(nullptr),
    m_unknown2(0)
{
}

/**
 *
 *
 */
RenderObjClass::RenderObjClass(const RenderObjClass &src) :
    m_bits(src.m_bits),
    m_transform(src.m_transform),
    m_objectScale(1.0f),
    m_unknown(0),
    m_cachedBoundingSphere(src.m_cachedBoundingSphere),
    m_cachedBoundingBox(src.m_cachedBoundingBox),
    m_nativeScreenSize(src.m_nativeScreenSize),
    m_isTransformIdentity(src.m_isTransformIdentity),
    m_scene(nullptr),
    m_container(nullptr),
    m_userData(nullptr),
    m_unknown2(0)
{
}

/**
 *
 *
 */
RenderObjClass &RenderObjClass::operator=(const RenderObjClass &that)
{
    if (this != &that) {
        // maybe TODO, in original this is missing ZH additions.....
        Set_Hidden(that.Is_Hidden());
        Set_Animation_Hidden(that.Is_Animation_Hidden());
        Set_Force_Visible(that.Is_Force_Visible());
        Set_Collision_Type(that.Get_Collision_Type());
        Set_Native_Screen_Size(that.Get_Native_Screen_Size());
        m_isTransformIdentity = false;
    }
    return *this;
}

void RenderObjClass::Add(SceneClass *scene) {}

void RenderObjClass::Remove() {}

/**
 *
 *
 */
SceneClass *RenderObjClass::Get_Scene()
{
    if (m_scene != nullptr) {
        m_scene->Add_Ref();
    }
    return m_scene;
}

/**
 *
 *
 */
void RenderObjClass::Set_Container(RenderObjClass *con)
{
    // ASSERT((con == nullptr) || (Container == nullptr));
    m_container = con;
}

/**
 *
 *
 */
void RenderObjClass::Validate_Transform() const
{
    RenderObjClass *con = Get_Container();
    bool dirty = false;

    if (con != nullptr) {
        dirty = con->Are_Sub_Object_Transforms_Dirty();

        while (con->Get_Container() != nullptr) {
            dirty |= con->Are_Sub_Object_Transforms_Dirty();
            con = con->Get_Container();
        }

        if (dirty) {
            con->Update_Sub_Object_Transforms();
            m_isTransformIdentity = Check_Is_Transform_Identity(m_transform);
        }
    }
}

/**
 *
 *
 */
void RenderObjClass::Set_Transform(const Matrix3D &m)
{
    m_transform = m;
    m_isTransformIdentity = Check_Is_Transform_Identity(m);
    Invalidate_Cached_Bounding_Volumes();
}

/**
 *
 *
 */
void RenderObjClass::Set_Position(const Vector3 &v)
{
    m_transform.Set_Translation(v);
    m_isTransformIdentity = Check_Is_Transform_Identity(m_transform);
    Invalidate_Cached_Bounding_Volumes();
}

/**
 *
 *
 */
void RenderObjClass::Notify_Added(SceneClass *scene)
{
    m_scene = scene;
}

/**
 *
 *
 */
void RenderObjClass::Notify_Removed(SceneClass *scene)
{
    m_scene = nullptr;
}

/**
 *
 *
 */
RenderObjClass *RenderObjClass::Get_Sub_Object_By_Name(const char *name, int *index) const
{
    // first try the unchanged name
    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);
        if (robj) {
            if (stricmp(robj->Get_Name(), name) == 0) {
                *index = i;
                return robj;
            } else {
                robj->Release_Ref();
            }
        }
    }

    // check the given name against the "suffix" names of each sub-object
    for (int i = 0; i < Get_Num_Sub_Objects(); i++) {
        RenderObjClass *robj = Get_Sub_Object(i);

        if (robj) {
            const char *sub_obj_name = strchr(robj->Get_Name(), '.');

            if (sub_obj_name == nullptr) {
                sub_obj_name = robj->Get_Name();
            } else {
                // skip past the period.
                sub_obj_name = sub_obj_name + 1;
            }

            if (stricmp(sub_obj_name, name) == 0) {
                *index = i;
                return robj;
            } else {
                robj->Release_Ref();
            }
        }
    }

    return nullptr;
}

/**
 *
 *
 */
int RenderObjClass::Add_Sub_Object_To_Bone(RenderObjClass *subobj, const char *bone_name)
{
    return Add_Sub_Object_To_Bone(subobj, Get_Bone_Index(bone_name));
}

/**
 *
 *
 */
int RenderObjClass::Remove_Sub_Objects_From_Bone(int bone_index)
{
    int count = Get_Num_Sub_Objects_On_Bone(bone_index);
    int remove_count = 0;

    for (int i = count - 1; i >= 0; i--) {
        RenderObjClass *robj = Get_Sub_Object_On_Bone(i, bone_index);

        if (robj) {
            remove_count += Remove_Sub_Object(robj);
            robj->Release_Ref();
        }
    }

    return remove_count;
}

/**
 *
 *
 */
int RenderObjClass::Remove_Sub_Objects_From_Bone(const char *bone_name)
{
    return Remove_Sub_Objects_From_Bone(Get_Bone_Index(bone_name));
}

/**
 *
 *
 */
void RenderObjClass::Update_Sub_Object_Transforms()
{
    // does nothing
}

/**
 *
 *
 */
bool RenderObjClass::Intersect(IntersectionClass *intersect, IntersectionResultClass *res)
{
    // do a quick sphere test to make sure it's worth doing the more expensive intersection test
    if (Intersect_Sphere_Quick(intersect, res)) {
        CastResultStruct castresult;
        LineSegClass lineseg;

        Vector3 end = *intersect->m_rayLocation + *intersect->m_rayDirection * intersect->m_maxDistance;
        lineseg.Set(*intersect->m_rayLocation, end);

        RayCollisionTestClass ray(lineseg, &castresult, COLLISION_TYPE_ALL, false, false);

        if (Cast_Ray(ray)) {
            lineseg.Compute_Point(ray.m_result->m_fraction, &(res->m_intersection));
            res->m_intersects = true;
            res->m_intersectionType = IntersectionResultClass::GENERIC;
            if (intersect->m_intersectionNormal)
                *intersect->m_intersectionNormal = castresult.m_normal;
            res->m_intersectedRenderObject = this;
            res->m_modelMatrix = m_transform;
            return true;
        }
    }
    res->m_intersects = false;
    return false;
}

/**
 *
 *
 */
bool RenderObjClass::Intersect_Sphere(IntersectionClass *intersect, IntersectionResultClass *res)
{
    SphereClass sphere = Get_Bounding_Sphere();
    return intersect->Intersect_Sphere(sphere, res);
}

/**
 *
 *
 */
bool RenderObjClass::Intersect_Sphere_Quick(IntersectionClass *intersect, IntersectionResultClass *res)
{
    SphereClass sphere = Get_Bounding_Sphere();
    return intersect->Intersect_Sphere_Quick(sphere, res);
}

/**
 *
 *
 */
void RenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(Vector3(0, 0, 0), 1.0f);
}

/**
 *
 *
 */
void RenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(Vector3(0, 0, 0), Vector3(0, 0, 0));
}

void RenderObjClass::Prepare_LOD(CameraClass &camera)
{
    // PredictiveLODOptimizerClass::Add_Cost(Get_Cost());
}

/**
 *
 *
 */
float RenderObjClass::Get_Cost() const
{
    int polycount = Get_Num_Polys();
    // If polycount is zero set Cost to a small number to avoid a division by zero
    float cost = (polycount != 0) ? polycount : 0.000001f;
    return cost;
}

/**
 *
 *
 */
int RenderObjClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
    values[0] = AT_MIN_LOD;
    values[1] = AT_MAX_LOD;
    costs[0] = Get_Cost();

    return 0;
}

/**
 *
 *
 */
bool RenderObjClass::Build_Dependency_List(DynamicVectorClass<StringClass> &file_list, bool recursive)
{
    if (recursive) {
        // Loop through all of this object's sub-objects
        int subobj_count = Get_Num_Sub_Objects();

        for (int index = 0; index < subobj_count; index++) {
            // request this sub-object to add all of its dependencies to the list
            RenderObjClass *sub_obj = Get_Sub_Object(index);

            if (sub_obj != nullptr) {
                sub_obj->Build_Dependency_List(file_list);
                sub_obj->Release_Ref();
            }
        }
    }

    // add all of this object's dependencies to the list
    Add_Dependencies_To_List(file_list);

    return (file_list.Count() > 0);
}

/**
 *
 *
 */
bool RenderObjClass::Build_Texture_List(DynamicVectorClass<StringClass> &texture_file_list, bool recursive)
{
    if (recursive) {
        // Loop through all of this object's sub-objects
        int subobj_count = Get_Num_Sub_Objects();
        for (int index = 0; index < subobj_count; index++) {
            // request this sub-object to add all of its texture dependencies to the list
            RenderObjClass *sub_obj = Get_Sub_Object(index);

            if (sub_obj != nullptr) {
                sub_obj->Build_Texture_List(texture_file_list);
                sub_obj->Release_Ref();
            }
        }
    }

    // add all of this object's texture dependencies to the list
    Add_Dependencies_To_List(texture_file_list, true);

    return (texture_file_list.Count() > 0);
}

/**
 *
 *
 */
float RenderObjClass::Get_Screen_Size(CameraClass &camera)
{
    // TODO
    Validate_Transform();
    Vector3 cam; //= camera.Get_Position();

    // ViewportClass viewport = camera.Get_Viewport();
    Vector2 vpr_min, vpr_max;
    // camera.Get_View_Plane(vpr_min, vpr_max);
    float width_factor; //= viewport.Width() / (vpr_max.X - vpr_min.X);
    float height_factor; //= viewport.Height() / (vpr_max.Y - vpr_min.Y);

    const SphereClass &sphere = Get_Bounding_Sphere();
    float dist = (sphere.Center - cam).Length();
    float radius = 0.0f;
    if (dist) {
        radius = sphere.Radius / dist;
    }
    return GAMEMATH_PI * radius * radius * width_factor * height_factor;
}

/**
 *
 *
 */
const PersistFactoryClass &RenderObjClass::Get_Factory() const
{
    return _RenderObjPersistFactory;
}

/**
 *
 *
 */
bool RenderObjClass::Save(ChunkSaveClass &csave)
{
    // TODO assert, this function should never be called
    // WWASSERT(0);
    return true;
}

/**
 *
 *
 */
bool RenderObjClass::Load(ChunkLoadClass &cload)
{
    // TODO assert, this function should never be called
    // WWASSERT(0);
    return true;
}

/**
 *
 *
 */
void RenderObjClass::Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only)
{
    if (textures_only == false) {
        // main W3D file
        const char *model_name = Get_Name();
        file_list.Add(::Filename_From_Asset_Name(model_name));

        // external hierarchy file
        const HTreeClass *phtree = Get_HTree();

        if (phtree != nullptr) {
            const char *htree_name = nullptr; // phtree->Get_Name (); //TODO FIXME

            if (::lstrcmpi(htree_name, model_name) != 0) {
                // Add this file to the list
                file_list.Add(::Filename_From_Asset_Name(htree_name));
            }
        }

        // Original W3D file (if an aggregate)
        const char *base_model_name = Get_Base_Model_Name();

        if (base_model_name != nullptr) {
            // add this file to the list
            file_list.Add(::Filename_From_Asset_Name(base_model_name));
        }
    }

    return;
}

/**
 *
 *
 */
void RenderObjClass::Update_Cached_Bounding_Volumes() const
{
    Get_Obj_Space_Bounding_Box(m_cachedBoundingBox);
    Get_Obj_Space_Bounding_Sphere(m_cachedBoundingSphere);

    Validate_Transform(); // needed? bfme2 doesn't do this...

    m_cachedBoundingSphere.Center = Get_Transform() * m_cachedBoundingSphere.Center;
    m_cachedBoundingSphere.Radius *= m_objectScale;

    Validate_Transform(); // needed? bfme2 doesn't do this...

    m_cachedBoundingBox.Transform(Get_Transform());

    Validate_Cached_Bounding_Volumes();
}

/**
 *
 *
 */
void RenderObjClass::Update_Sub_Object_Bits()
{
    if (Get_Num_Sub_Objects() != 0) {
        int coltype = 0;
        int istrans = 0;
        int isalpha = 0;
        int isaddit = 0;

        for (int i = 0; i < Get_Num_Sub_Objects(); ++i) {
            RenderObjClass *robj = Get_Sub_Object(i);
            coltype |= robj->Get_Collision_Type();
            istrans |= robj->Is_Translucent();
            isalpha |= robj->Is_Alpha();
            isaddit |= robj->Is_Additive();
            robj->Release_Ref();
        }

        Set_Collision_Type(coltype);
        Set_Translucent(istrans);
        Set_Alpha(isalpha);
        Set_Additive(isaddit);

        if (m_container) {
            m_container->Update_Sub_Object_Bits();
        }
    }
}