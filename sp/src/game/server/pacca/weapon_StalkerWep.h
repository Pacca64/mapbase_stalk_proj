//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "Sprite.h"		//sprites

#ifndef WEAPON_StalkerWep_H
#define WEAPON_StalkerWep_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_StalkerWep.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	StalkerWep_RANGE	75.0f
#define	StalkerWep_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponStalkerWep
//-----------------------------------------------------------------------------

class CWeaponStalkerWep : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponStalkerWep, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	DECLARE_DATADESC();

	CWeaponStalkerWep();

	float		GetRange( void )		{	return	StalkerWep_RANGE;	}
	float		GetFireRate( void )		{	return	StalkerWep_REFIRE;	}

	void		AddViewKick( void );
	void Precache(void);
	void Think(void);
	void OnPickedUp(CBaseCombatCharacter* pNewOwner);
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		PrimaryAttack(void);
	void		SecondaryAttack(void);

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

#ifdef MAPBASE
	// Don't use backup activities
	acttable_t		*GetBackupActivityList() { return NULL; }
	int				GetBackupActivityListCount() { return 0; }
#endif

	void   	DrawBeam(const Vector& startPos, const Vector& endPos, float width);

	void DrawAttackBeam(void);

	void UpdateOnRemove(void);

	void ItemPostFrame(void);

	int	m_nBulletType;	//probably will go unused

	//Taken from stalker code
	float				m_fNextDamageTime;

	float				m_bPlayingHitWall;
	float				m_bPlayingHitFlesh;
	CHandle<CSprite>	m_pLightGlow;
	float				m_fLightGlowTrans;
	float				m_fLightGlowLastUpdateTime;

	int				m_iLastKnownSkillLevel;	//Stores skill level from previous frame. Not saved; used only to change laser sprite.

	float			m_iLastLaserFireEnded;	//should be set to curtime when player releases attack1
	bool			m_bPrimaryFireHeldLastFrame;	//should be set to false when primary fire isn't active.
	float			m_fTimeSincePrimaryFireStarted;	//set when player fires a laser for the first time this cooldown, used for armor drain.
	float			m_fArmorDrainFraction;	//if this goes over 1, subtract 1 from armor and this. Allows us to pretend armor is a float.

	float			m_fArmorDrainRate;	//Starts at base convar value, then grows gradually by growth convar value.

protected:
	void CreateLaserSprite();

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
};

#endif // WEAPON_StalkerWep_H
