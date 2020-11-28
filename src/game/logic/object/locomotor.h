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
#pragma once
#include "asciistring.h"
#include "snapshot.h"
#include "namekeygenerator.h"
#include "overridable.h"
#include <vector>

struct FieldParse;
class INI;

enum LocomotorBehaviorZ
{
    Z_NO_Z_MOTIVE_FORCE,
    Z_SEA_LEVEL,
    Z_SURFACE_RELATIVE_HEIGHT,
    Z_ABSOLUTE_HEIGHT,
    Z_FIXED_SURFACE_RELATIVE_HEIGHT,
    Z_FIXED_ABSOLUTE_HEIGHT,
    Z_FIXED_RELATIVE_TO_GROUND_AND_BUILDINGS,
    Z_RELATIVE_TO_HIGHEST_LAYER
};

enum LocomotorAppearance
{
    LOCO_TWO_LEGS,
    LOCO_FOUR_WHEELS,
    LOCO_TREADS,
    LOCO_HOVER,
    LOCO_THRUST,
    LOCO_WINGS,
    LOCO_CLIMBER,
    LOCO_OTHER,
    LOCO_MOTORCYCLE
};

class LocomotorTemplate final : public Overridable
{
    IMPLEMENT_POOL(LocomotorTemplate)

public:
    LocomotorTemplate();
    virtual ~LocomotorTemplate() {}

    void Validate();

    static FieldParse *Get_Parse_Table() { return s_parseTable; }

public:
    Utf8String m_name;
    int m_surfaces; // not sure if int
    float m_maxSpeed;
    float m_maxSpeedDamaged;
    float m_minSpeed;
    float m_maxTurnRate;
    float m_maxTurnRateDamaged;
    float m_maxAccel;
    float m_maxAccelDamaged;
    float m_maxLift;
    float m_maxLiftDamaged;
    float m_braking;
    float m_minTurnSpeed;
    float m_preferredHeight;
    float m_preferredHeightDamping;
    float m_circlingRadius;
    float m_speedLimitZ;
    float m_extra2DFriction;
    float m_maxThrustAngle;
    LocomotorBehaviorZ m_behaviorZ;
    LocomotorAppearance m_appearance;
    int m_groupMovementPriority; // not sure if int
    float m_accelPitchLimit;
    float m_deaccelPitchLimit;
    float m_bounceKick;
    float m_pitchStiffness;
    float m_rollStiffness;
    float m_pitchDamping;
    float m_rollDamping;
    float m_pitchInDirectionOfZVelFactor;
    float m_thrustRoll;
    float m_thrustWobbleRate;
    float m_thrustMinWobble;
    float m_thrustMaxWobble;
    float m_forwardVelCoef;
    float m_lateralVelCoef;
    float m_forwardAccelCoef;
    float m_lateralAccelCoef;
    float m_uniformAxialDamping;
    float m_turnPivotOffset;
    int m_airborneTargetingHeight;
    float m_closeEnoughDist;
    bool m_closeEnoughDist3D;
    float m_slideIntoPlaceTime;
    bool m_locomotorWorksWhenDead;
    bool m_allowMotiveForceWhileAirborne;
    bool m_apply2DFrictionWhenAirborne;
    bool m_downhillOnly;
    bool m_stickToGround;
    bool m_canMoveBackwards;
    bool m_hasSuspension;
    float m_maximumWheelExtension;
    float m_maximumWheelCompression;
    float m_wheelTurnAngle;
    float m_wanderWidthFactor;
    float m_wanderLengthFactor;
    float m_wanderAboutPointRadius;
    float m_rudderCorrectionDegree;
    float m_rudderCorrectionRate;
    float m_elevatorCorrectionDegree;
    float m_elevatorCorrectionRate;

private:
    static FieldParse s_parseTable[];
};

class Locomotor : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Locomotor)

public:
    ~Locomotor();

    void CRC_Snapshot(Xfer* xfer) override {}
    void Xfer_Snapshot(Xfer* xfer) override;
    void Load_Post_Process() override {}
};

class LocomotorStore final : public SubsystemInterface
{
public:
    LocomotorStore() {}
    ~LocomotorStore();

    void Init() override {}
    void Reset() override;
    void Update() override {}

    LocomotorTemplate *Find_Locomotor_Template(NameKeyType key);

    Locomotor *New_Locomotor(LocomotorTemplate *tmpl);
    LocomotorTemplate *New_Override(LocomotorTemplate *tmpl);
	
	static void Parse_Locomotor_Template_Definition(INI *ini);

};

class LocomotorSet : public SnapShot
{
public:
    LocomotorSet();
	virtual ~LocomotorSet();

    void CRC_Snapshot(Xfer *xfer) override {}
    //void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override {}
	
    //void Clear();

	void Add_Locomotor(LocomotorTemplate const *tmpl);
    Locomotor *Find_Locomotor(LocomotorTemplate const *tmpl);

	int Get_Valid_Surfaces() const { return m_validLocomotorSurfaces; }
    bool Is_Downhill_Only() const { return m_downhillOnly; }
	
private:
    std::vector<Locomotor *> m_locomotorTemplates;
    int m_validLocomotorSurfaces;
    bool m_downhillOnly;
};