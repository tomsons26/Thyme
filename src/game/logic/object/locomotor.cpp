/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Templated information for locomotors.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "locomotor.h"
#include "ini.h"

// TODO needs to be moved to a common place
const float _BIGNUM = 99999.0f;

const char *g_locomotor_surface_type_names[] = { "GROUND", "WATER", "CLIFF", "AIR", "RUBBLE" };
const char *g_locomotor_behavior_z_names[] = { "NO_Z_MOTIVE_FORCE",
    "SEA_LEVEL",
    "SURFACE_RELATIVE_HEIGHT",
    "ABSOLUTE_HEIGHT",
    "FIXED_SURFACE_RELATIVE_HEIGHT",
    "FIXED_ABSOLUTE_HEIGHT",
    "FIXED_RELATIVE_TO_GROUND_AND_BUILDINGS",
    "RELATIVE_TO_HIGHEST_LAYER" };
const char *g_locomotor_appearance_names[] = {
    "TWO_LEGS", "FOUR_WHEELS", "TREADS", "HOVER", "THRUST", "WINGS", "CLIMBER", "OTHER", "MOTORCYCLE"
};
const char *g_locomotor_priority_names[] = { "MOVES_BACK", "MOVES_MIDDLE", "MOVES_FRONT" };

// clang-format off
FieldParse LocomotorTemplate::s_parseTable[] = {
    { "Surfaces", &INI::Parse_Bitstring32, g_locomotor_surface_type_names, offsetof(LocomotorTemplate, m_surfaces) },
    { "Speed", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxSpeed) },
    { "SpeedDamaged", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxSpeedDamaged) },
    { "TurnRate", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxTurnRate) },
    { "TurnRateDamaged", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_maxTurnRateDamaged) },
    { "Acceleration", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_maxAccel) },
    { "AccelerationDamaged", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_maxAccelDamaged) },
    { "Lift", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_maxLift) },
    { "LiftDamaged", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_maxLiftDamaged) },
    { "Braking", &INI::Parse_Acceleration_Real, nullptr, offsetof(LocomotorTemplate, m_braking) },
    { "MinSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_minSpeed) },
    { "MinTurnSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_minTurnSpeed) },
    { "PreferredHeight", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_preferredHeight) },
    { "PreferredHeightDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_preferredHeightDamping) },
    { "CirclingRadius", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_circlingRadius) },
    { "Extra2DFriction", INI::Parse_Friction_Per_Sec, nullptr, offsetof(LocomotorTemplate, m_extra2DFriction) },
    { "SpeedLimitZ", &INI::Parse_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_speedLimitZ) },
    { "MaxThrustAngle", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_maxThrustAngle) },
    { "ZAxisBehavior", &INI::Parse_Index_List, g_locomotor_behavior_z_names, offsetof(LocomotorTemplate, m_behaviorZ) },
    { "Appearance", &INI::Parse_Index_List, g_locomotor_appearance_names, offsetof(LocomotorTemplate, m_appearance) },
    { "GroupMovementPriority", &INI::Parse_Index_List, g_locomotor_priority_names, offsetof(LocomotorTemplate, m_groupMovementPriority) },
    { "AccelerationPitchLimit", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_accelPitchLimit) },
    { "DecelerationPitchLimit", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_deaccelPitchLimit) },
    { "BounceAmount", &INI::Parse_Angular_Velocity_Real, nullptr, offsetof(LocomotorTemplate, m_bounceKick) },
    { "PitchStiffness", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_pitchStiffness) },
    { "RollStiffness", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rollStiffness) },
    { "PitchDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_pitchDamping) },
    { "RollDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rollDamping) },
    { "ThrustRoll", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustRoll) },
    { "ThrustWobbleRate", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustWobbleRate) },
    { "ThrustMinWobble", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustMinWobble) },
    { "ThrustMaxWobble", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_thrustMaxWobble) },
    { "PitchInDirectionOfZVelFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_pitchInDirectionOfZVelFactor) },
    { "ForwardVelocityPitchFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_forwardVelCoef) },
    { "LateralVelocityRollFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_lateralVelCoef) },
    { "ForwardAccelerationPitchFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_forwardAccelCoef) },
    { "LateralAccelerationRollFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_lateralAccelCoef) },
    { "UniformAxialDamping", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_uniformAxialDamping) },
    { "TurnPivotOffset", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_turnPivotOffset) },
    { "Apply2DFrictionWhenAirborne", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_apply2DFrictionWhenAirborne) },
    { "DownhillOnly", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_downhillOnly) },
    { "AllowAirborneMotiveForce", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_allowMotiveForceWhileAirborne) },
    { "LocomotorWorksWhenDead", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_locomotorWorksWhenDead) },
    { "AirborneTargetingHeight", &INI::Parse_Int, nullptr, offsetof(LocomotorTemplate, m_airborneTargetingHeight) },
    { "StickToGround", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_stickToGround) },
    { "CanMoveBackwards", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_canMoveBackwards) },
    { "HasSuspension", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_hasSuspension) },
    { "FrontWheelTurnAngle", &INI::Parse_Angle_Real, nullptr, offsetof(LocomotorTemplate, m_wheelTurnAngle) },
    { "MaximumWheelExtension", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_maximumWheelExtension) },
    { "MaximumWheelCompression", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_maximumWheelCompression) },
    { "CloseEnoughDist", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_closeEnoughDist) },
    { "CloseEnoughDist3D", &INI::Parse_Bool, nullptr, offsetof(LocomotorTemplate, m_closeEnoughDist3D) },
    { "SlideIntoPlaceTime", &INI::Parse_Duration_Real, nullptr, offsetof(LocomotorTemplate, m_slideIntoPlaceTime) },
    { "WanderWidthFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_wanderWidthFactor) },
    { "WanderLengthFactor", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_wanderLengthFactor) },
    { "WanderAboutPointRadius", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_wanderAboutPointRadius) },
    { "RudderCorrectionDegree", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rudderCorrectionDegree) },
    { "RudderCorrectionRate", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_rudderCorrectionRate) },
    { "ElevatorCorrectionDegree", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_elevatorCorrectionDegree) },
    { "ElevatorCorrectionRate", &INI::Parse_Real, nullptr, offsetof(LocomotorTemplate, m_elevatorCorrectionRate) },
    { nullptr, nullptr, nullptr, 0 },
};
// clang-format on

LocomotorTemplate::LocomotorTemplate() :
    m_maxSpeedDamaged(-1.0f),
    m_maxTurnRateDamaged(-1.0f),
    m_maxAccelDamaged(-1.0f),
    m_maxLiftDamaged(-1.0f),
    m_surfaces(0),
    m_maxSpeed(0),
    m_maxTurnRate(0),
    m_maxAccel(0),
    m_maxLift(0),
    m_braking(_BIGNUM),
    m_minSpeed(0),
    m_minTurnSpeed(_BIGNUM),
    m_behaviorZ(Z_NO_Z_MOTIVE_FORCE),
    m_appearance(LOCO_OTHER),
    m_groupMovementPriority(1),
    m_preferredHeight(0),
    m_preferredHeightDamping(1.0f),
    m_circlingRadius(0),
    m_maxThrustAngle(0),
    m_speedLimitZ(9999999.0f),
    m_extra2DFriction(0),
    m_accelPitchLimit(0),
    m_deaccelPitchLimit(0),
    m_bounceKick(0),
    m_pitchStiffness(0.1f),
    m_rollStiffness(0.1f),
    m_pitchDamping(0.9f),
    m_rollDamping(0.9f),
    m_forwardVelCoef(0),
    m_pitchInDirectionOfZVelFactor(0),
    m_thrustRoll(0),
    m_thrustWobbleRate(0),
    m_thrustMinWobble(0),
    m_thrustMaxWobble(0),
    m_lateralVelCoef(0),
    m_forwardAccelCoef(0),
    m_lateralAccelCoef(0),
    m_uniformAxialDamping(1.0f),
    m_turnPivotOffset(0),
    m_apply2DFrictionWhenAirborne(false),
    m_downhillOnly(false),
    m_allowMotiveForceWhileAirborne(false),
    m_locomotorWorksWhenDead(false),
    m_airborneTargetingHeight(INT32_MAX),
    m_stickToGround(false),
    m_canMoveBackwards(false),
    m_hasSuspension(false),
    m_wheelTurnAngle(0),
    m_maximumWheelExtension(0),
    m_maximumWheelCompression(0),
    m_closeEnoughDist(1.0f),
    m_closeEnoughDist3D(false),
    m_slideIntoPlaceTime(0),
    m_wanderWidthFactor(0),
    m_wanderLengthFactor(1.0f),
    m_wanderAboutPointRadius(0),
    m_rudderCorrectionDegree(0),
    m_rudderCorrectionRate(0),
    m_elevatorCorrectionDegree(0),
    m_elevatorCorrectionRate(0)
{
}

void LocomotorTemplate::Validate()
{
    if (m_maxSpeedDamaged < 0.0f) {
        m_maxSpeedDamaged = m_maxSpeed;
    }

    if (m_maxTurnRateDamaged < 0.0f) {
        m_maxTurnRateDamaged = m_maxTurnRate;
    }

    if (m_maxAccelDamaged < 0.0f) {
        m_maxAccelDamaged = m_maxAccel;
    }

    if (m_maxLiftDamaged < 0.0f) {
        m_maxLiftDamaged = m_maxLift;
    }

    if (m_appearance == LOCO_WINGS) {
        if (m_minSpeed <= 0.0f) {
            captainslog_debug("WINGS should always have positive minSpeeds (otherwise, they hover)");
            m_minSpeed = 0.01f;
        }

        if (m_minTurnSpeed <= 0.0f) {
            captainslog_debug("WINGS should always have positive minTurnSpeed");
            m_minTurnSpeed = 0.01f;
        }
    }

    if (m_appearance == LOCO_THRUST) {
        if (m_behaviorZ != Z_NO_Z_MOTIVE_FORCE || m_maxLift != 0.0f || m_maxLiftDamaged != 0.0f) {
            captainslog_error("THRUST locos may not use ZAxisBehavior or lift!\n");
        }

        if (m_maxSpeed <= 0.0f) {
            captainslog_debug("THRUST locos may not have zero m_maxSpeed; healing...\n");
            m_maxSpeed = 0.01f;
        }

        if (m_maxSpeedDamaged <= 0.0f) {
            captainslog_debug("THRUST locos may not have zero m_maxSpeedDamaged; healing...\n");
            m_maxSpeedDamaged = 0.01f;
        }

        if (m_minSpeed <= 0.0f) {
            captainslog_debug("THRUST locos may not have zero m_minSpeed; healing...\n");
            m_minSpeed = 0.01f;
        }
    }
}

LocomotorTemplate *LocomotorStore::New_Override(LocomotorTemplate *tmpl)
{
    if (tmpl == nullptr) {
        return nullptr;
    }

    LocomotorTemplate *newo = new LocomotorTemplate;

    newo = tmpl;
    tmpl->Set_Next(newo);
    newo->Set_Allocated();

    return newo;
}

LocomotorSet::LocomotorSet() : m_validLocomotorSurfaces(0), m_downhillOnly(false) {}

// check me
LocomotorSet::~LocomotorSet()
{
    // Clear();
}
