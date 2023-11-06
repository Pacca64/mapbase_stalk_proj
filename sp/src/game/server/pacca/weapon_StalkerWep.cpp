//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		StalkerWep - proxy weapon for stalker abilities. Based on weapon_crowbar
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_StalkerWep.h"
#include "beam_shared.h"	/* We will use some of the functions declared this later... */
#include "gamestats.h"		//saves stats for primary and secondary fires
#include "rumble_shared.h"	//need for melee secondary fire
#include "util_shared.h"	//for access to trace line
#include "bspflags.h"	//for access to trace line flags

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// This determines the sprite used by the laser beam
#define PHYSCANNON_BEAM_SPRITE "sprites/orangelight1.vmt"

// Stalker Constants copied over.
#define	MIN_STALKER_FIRE_RANGE		64
#define	MAX_STALKER_FIRE_RANGE		3600 // 3600 feet.
#define	STALKER_LASER_ATTACHMENT	1
#define	STALKER_TRIGGER_DIST		200	// Enemy dist. that wakes up the stalker
#define	STALKER_SENTENCE_VOLUME		(float)0.35
#define STALKER_LASER_DURATION		99999
#define STALKER_LASER_RECHARGE		1
#define STALKER_PLAYER_AGGRESSION	1

//copied from basebludgeonweapon
#define BLUDGEON_HULL_DIM		16	
static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

//special convars for us
ConVar    sk_plr_dmg_stalkerwep		( "sk_plr_dmg_stalkerwep","0");
ConVar    sk_npc_dmg_stalkerwep		( "sk_npc_dmg_stalkerwep","0");

//-----------------------------------------------------------------------------
// CWeaponStalkerWep
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponStalkerWep, DT_WeaponStalkerWep)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_StalkerWep, CWeaponStalkerWep );
PRECACHE_WEAPON_REGISTER( weapon_StalkerWep );
#endif

acttable_t CWeaponStalkerWep::m_acttable[] = 
{
	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_RUN,				ACT_RUN_MELEE,			false },
	{ ACT_WALK,				ACT_WALK_MELEE,			false },

	{ ACT_ARM,				ACT_ARM_MELEE,			false },
	{ ACT_DISARM,			ACT_DISARM_MELEE,		false },
#endif

#ifdef MAPBASE
	// HL2:DM activities (for third-person animations in SP)
	{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_MELEE,                    false },
	{ ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_MELEE,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_MELEE,            false },
	{ ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_MELEE,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_MELEE,            false },
	{ ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_MELEE,                    false },
#if EXPANDED_HL2DM_ACTIVITIES
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK2,	ACT_HL2MP_GESTURE_RANGE_ATTACK2_MELEE,		false },
	{ ACT_HL2MP_WALK,					ACT_HL2MP_WALK_MELEE,						false },
#endif
#endif
};

IMPLEMENT_ACTTABLE(CWeaponStalkerWep);

BEGIN_DATADESC(CWeaponStalkerWep)
	DEFINE_FIELD(m_nBulletType, FIELD_INTEGER),
END_DATADESC()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponStalkerWep::CWeaponStalkerWep( void )
{
	m_nBulletType = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponStalkerWep::GetDamageForActivity( Activity hitActivity )
{
	if ( ( GetOwner() != NULL ) && ( GetOwner()->IsPlayer() ) )
		return sk_plr_dmg_stalkerwep.GetFloat();

	return sk_npc_dmg_stalkerwep.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponStalkerWep::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( 1.0f, 2.0f );
	punchAng.y = random->RandomFloat( -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the StalkerWep!)
//-----------------------------------------------------------------------------
ConVar sk_StalkerWep_lead_time( "sk_StalkerWep_lead_time", "0.9" );

int CWeaponStalkerWep::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the StalkerWep!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_StalkerWep_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponStalkerWep::PrimaryAttack()
{
	// Only the player fires this way so we can cast
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	Vector startPos = pPlayer->Weapon_ShootPosition();
	Vector AimDir = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	trace_t tr;
	UTIL_TraceLine(startPos, startPos + AimDir * MAX_STALKER_FIRE_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = GetOwner()->Weapon_ShootPosition();
	DrawBeam(vecShootOrigin, tr.endpos, 15.5);
}

//------------------------------------------------------------------------------
// Purpose : Secondary Melee Attack. Copied from CBaseHLBludgeonWeapon::Swing, changed to act like primary but be secondary.
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponStalkerWep::SecondaryAttack()
{
	BOOL bIsSecondary = true;

	trace_t traceHit;

	// Try a ray
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	if (!bIsSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		//hack to let us reskin the crowbar; our primary fire needs no animations anyway.
		nHitActivity = ACT_VM_MISSCENTER;//nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

#ifdef MAPBASE
		// Sound has been moved here since we're using the other melee sounds now
		WeaponSound(SINGLE);
#endif

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{
#ifdef MAPBASE
		// Other melee sounds
		if (traceHit.m_pEnt && traceHit.m_pEnt->IsWorld())
			WeaponSound(MELEE_HIT_WORLD);
		else if (traceHit.m_pEnt && !traceHit.m_pEnt->PassesDamageFilter(triggerInfo))
			WeaponSound(MELEE_MISS);
		else
			WeaponSound(MELEE_HIT);
#endif

		Hit(traceHit, nHitActivity, bIsSecondary ? true : false);
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

#ifndef MAPBASE
	//Play swing sound
	WeaponSound(SINGLE);
#endif

#ifdef MAPBASE
	pOwner->SetAnimation(PLAYER_ATTACK1);
#endif
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponStalkerWep::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if ( pEnemy )
	{
		Vector vecDelta;
		VectorSubtract( pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta );
		VectorNormalize( vecDelta );
		
		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize( vecDelta2D );
		if ( DotProduct2D( vecDelta2D, vecDirection.AsVector2D() ) > 0.8f )
		{
			vecDirection = vecDelta;
		}
	}

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(), vecEnd, 
		Vector(-16,-16,-16), Vector(36,36,36), sk_npc_dmg_stalkerwep.GetFloat(), DMG_CLUB, 0.75 );
	
	// did I hit someone?
	if ( pHurt )
	{
		// play sound
		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponStalkerWep::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//Routines from this goofy tutorial: https://developer.valvesoftware.com/wiki/Laserweapon
//Modify away!

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &startPos - where the beam should begin
//          &endPos - where the beam should end
//          width - what the diameter of the beam should be (units?)
//-----------------------------------------------------------------------------
void CWeaponStalkerWep::DrawBeam(const Vector& startPos, const Vector& endPos, float width)
{
	//Tracer down the middle
	UTIL_Tracer(startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer");

	//Draw the main beam shaft
	CBeam* pBeam = CBeam::BeamCreate(PHYSCANNON_BEAM_SPRITE, 15.5);

	// It starts at startPos
	pBeam->SetStartPos(startPos);

	// This sets up some things that the beam uses to figure out where
	// it should start and end
	pBeam->PointEntInit(endPos, this);

	// This makes it so that the laser appears to come from the muzzle of the pistol
	pBeam->SetEndAttachment(LookupAttachment("Muzzle"));
	pBeam->SetWidth(width);
	//	pBeam->SetEndWidth( 0.05f );

		// Higher brightness means less transparent
	pBeam->SetBrightness(255);
	pBeam->SetColor(255, 185 + random->RandomInt(-16, 16), 40);
	pBeam->RelinkBeam();

	// The beam should only exist for a very short time
	pBeam->LiveForTime(0.1f);
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - used to figure out where to do the effect
//          nDamageType - ???
//-----------------------------------------------------------------------------
void CWeaponStalkerWep::DoImpactEffect(trace_t& tr, int nDamageType)
{
	//Draw our beam
	DrawBeam(tr.startpos, tr.endpos, 15.5);
	if ((tr.surface.flags & SURF_SKY) == false)
	{
		CPVSFilter filter(tr.endpos);
		te->GaussExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
		m_nBulletType = GetAmmoDef()->Index("GaussEnergy");
		UTIL_ImpactTrace(&tr, m_nBulletType);
	}
}