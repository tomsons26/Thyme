#include "lightenv.h"
#include "colorspace.h"
#include "matrix3d.h"
#include "light.h"

#include <algorithm>

const float DIFFUSE_TO_AMBIENT_FRACTION = 1.0f;

static float _LightingLODCutoff = 0.5f;
static float _LightingLODCutoff2 = 0.5f * 0.5f;

// matches zh/bfme2
LightEnvironmentClass::LightEnvironmentClass() :
    m_lightCount(0), m_objectCenter(0, 0, 0), m_outputAmbient(0, 0, 0), m_intensity(0)
{
}

// matches zh/bfme2
LightEnvironmentClass::~LightEnvironmentClass() {}

// matches zh/bfme2
void LightEnvironmentClass::Reset(const Vector3 &object_center, const Vector3 &ambient)
{
    m_lightCount = 0;
    m_objectCenter = object_center;
    m_outputAmbient = ambient;
}

// matches zh/bfme2?
void LightEnvironmentClass::Add_Light(const LightClass &light)
{
    Vector3 diffuse;
    light.Get_Diffuse(&diffuse);

    if (diffuse[0] >= 0.05f || diffuse[1] >= 0.05f || diffuse[2] >= 0.05f) {
        InputLightStruct new_light;
        new_light.Init(light, m_objectCenter);

        if (m_intensity != 0.0) {
            new_light.m_diffuse *= light.Get_Intensity();
        }

        m_outputAmbient += new_light.m_ambient;

        if (!new_light.m_diffuseRejected || new_light.m_isPoint) {
            for (int i = 0; i < m_lightCount; i++) {
                if (m_inputLights[i].Contribution() < new_light.Contribution()) {
                    for (int j = m_lightCount; j > i; --j) {
                        if (j < 4) {
                            m_inputLights[j] = m_inputLights[j - 1];
                        }
                    }

                    m_inputLights[i] = new_light;
                    m_lightCount = std::min<int>(m_lightCount + 1, MAX_LIGHTS);
                    return;
                }
            }

            if (m_lightCount < MAX_LIGHTS) {
                m_inputLights[m_lightCount] = new_light;
                ++m_lightCount;
            }
        }
    }
}

// matches zh/bfme2
void LightEnvironmentClass::Pre_Render_Update(const Matrix3D &camera_tm)
{
    Calculate_Fill_Light();

    for (int i = 0; i < m_lightCount; i++) {
        m_outputLights[i].Init(m_inputLights[i], camera_tm);
    }

    m_outputAmbient.X = std::clamp(m_outputAmbient.X, 0.0f, 1.0f);
    m_outputAmbient.Y = std::clamp(m_outputAmbient.Y, 0.0f, 1.0f);
    m_outputAmbient.Z = std::clamp(m_outputAmbient.Z, 0.0f, 1.0f);
}

// matches zh/bfme2
void LightEnvironmentClass::Add_Fill_Light()
{
    if (m_fillLight.m_diffuse[0] >= 0.05f || m_fillLight.m_diffuse[1] >= 0.05f || m_fillLight.m_diffuse[2] >= 0.05f) {
        int i = m_lightCount;
        if (i == MAX_LIGHTS) {
            i = 3;
        } else {
            i = m_lightCount++;
        }
        m_inputLights[i] = m_fillLight;
    }
    m_outputAmbient += m_fillLight.m_ambient;
}

// matches zh/bfme2?
void LightEnvironmentClass::Calculate_Fill_Light()
{
    if (m_lightCount > 0 && m_intensity != 0.0) {
        float c0 = m_inputLights[0].Contribution();

        InputLightStruct input = m_inputLights[0];

        for (int i = 1; i < std::min(m_lightCount, 3); ++i) {
            float cc = m_inputLights[i].Contribution() / c0;

            input.m_direction += m_inputLights[i].m_direction * cc;
            input.m_ambient += m_inputLights[i].m_ambient * cc;
            input.m_diffuse += m_inputLights[i].m_diffuse * cc;
        }

        input.m_direction.Normalize();

        Vector3 hsv;

        RGB_To_HSV(hsv, input.m_diffuse);

        //?
        hsv.X = hsv.X + 180.0;
        if (hsv.X > 360.0) {
            hsv.X = hsv.X - 360.0;
        }

        hsv.Z = hsv.Z * m_intensity;
        HSV_To_RGB(m_fillLight.m_diffuse, hsv);

        // A fill light has no ambient.
        m_fillLight.m_ambient.Set(0, 0, 0);

        m_fillLight.m_direction = input.m_direction * -1.0;

        m_fillLight.m_diffuseRejected = false;

        Add_Fill_Light();
    }
}

// matches zh/bfme2
void LightEnvironmentClass::Set_Lighting_LOD_Cutoff(float inten)
{
    _LightingLODCutoff = inten;
    _LightingLODCutoff2 = _LightingLODCutoff * _LightingLODCutoff;
}

// matches zh/bfme2
float LightEnvironmentClass::Get_Lighting_LOD_Cutoff()
{
    return _LightingLODCutoff;
}

// matches zh/bfme2
void LightEnvironmentClass::InputLightStruct::Init(const LightClass &light, const Vector3 &object_center)
{
    m_isPoint = false;

    switch (light.Get_Type()) {
        case LightClass::POINT:
        case LightClass::SPOT:
            Init_From_Point_Or_Spot_Light(light, object_center);
            break;
        case LightClass::DIRECTIONAL:
            Init_From_Directional_Light(light, object_center);
            break;
        default:
            captainslog_debug("Invalid light type.  Light will not be initialized properly.");
            break;
    };
}

// matches zh/bfme2?
void LightEnvironmentClass::InputLightStruct::Init_From_Point_Or_Spot_Light(
    const LightClass &light, const Vector3 &object_center)
{
    m_direction = light.Get_Position() - object_center;
    float dist = m_direction.Length();

    if (dist > 0.0f) {
        m_direction /= dist;
    }

    double atten_start, atten_end;
    float atten = 1.0f;

    light.Get_Far_Attenuation_Range(atten_start, atten_end);

    if (light.Get_Flag(LightClass::FAR_ATTENUATION)) {
        if (GameMath::Fabs(atten_end - atten_start) >= 0.000099999997) {
            atten = 1.0f - (dist - atten_start) / (atten_end - atten_start);
            atten = std::clamp(atten, 0.0f, 1.0f);
        } else if (dist > atten_start) {
            atten = 0.0;
        }
    }

    if (light.Get_Type() == LightClass::SPOT) {
        Vector3 spot_dir;
        light.Get_Spot_Direction(spot_dir);
        Matrix3D::Rotate_Vector(light.Get_Transform(), spot_dir, &spot_dir);

        float spot_angle_cos = light.Get_Spot_Angle_Cos();
        atten *= (Vector3::Dot_Product(-spot_dir, m_direction) - spot_angle_cos) / (1.0f - spot_angle_cos);
        atten = std::clamp(atten, 0.0f, 1.0f);
    }

    light.Get_Ambient(&m_ambient);
    light.Get_Diffuse(&m_diffuse);

    m_ambient *= light.Get_Intensity();
    m_diffuse *= light.Get_Intensity();

    m_isPoint = light.Get_Type() == LightClass::POINT;

    m_pointCenter = light.Get_Position();

    m_pointInnerRadius = atten_start;
    m_pointOuterRadius = atten_end;

    m_pointAmbient = m_ambient;
    m_pointDiffuse = m_diffuse;

    if (m_diffuse.Length2() > _LightingLODCutoff2) {
        m_diffuseRejected = false;
        m_ambient *= atten;
        m_diffuse *= atten;

    } else {
        m_diffuseRejected = true;
        m_ambient *= atten;
        m_ambient += atten * DIFFUSE_TO_AMBIENT_FRACTION * m_diffuse;
        m_diffuse.Set(0, 0, 0);
    }
}

// matches zh/bfme2
void LightEnvironmentClass::InputLightStruct::Init_From_Directional_Light(
    const LightClass &light, const Vector3 &object_center)
{
    m_direction = -light.Get_Transform().Get_Z_Vector();

    m_diffuseRejected = false;
    light.Get_Ambient(&m_ambient);
    light.Get_Diffuse(&m_diffuse);
}

// matches zh/bfme2
float LightEnvironmentClass::InputLightStruct::Contribution()
{
    return m_diffuse.Length2();
}

// matches zh/bfme2
void LightEnvironmentClass::OutputLightStruct::Init(const InputLightStruct &input, const Matrix3D &camera_tm)
{
    m_diffuse = input.m_diffuse;
    Matrix3D::Inverse_Rotate_Vector(camera_tm, input.m_direction, &m_direction);

    if (m_direction.Length2() == 0.0) {
        m_direction.X = 1.0;
    }
}
